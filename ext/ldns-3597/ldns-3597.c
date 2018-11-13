//
// ldns-3597
//
// Converts (supported) RR type presentation formats
// to RFC3597 notation
//
// Written by Jelte Jansen for SIDN
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <getopt.h>

#include <ldns/ldns.h>

#define MAX_LEN 4096

void
usage() {
    printf("usage: ldns-3597 [types]\n");
    printf("Types are in the form of RR type mnemonics (e.g.\n");
    printf("AAAA, MX, etc)\n");
    printf("Multiple types can be given.\n");
    printf("Reads RRs (flattened) from stdin, and converts the\n");
    printf("given types of those to rfc3597 unknown-type format\n");
    printf("For instance, ldns-3597 A6 converts all A6 records\n");
    printf("it reads to unknown-type format.\n");
    printf("Known types that use RR type mnemonics in their\n");
    printf("presentation format (e.g. RRSIG, NSEC, NSEC3) are\n");
    printf("also converted if necessary.\n");
    printf("\n");
    printf("Example: ldns-read-zone -f - myzone.zone | ldns-3597 MX AAAA PTR\n");
}

// Print an rr in its normal presentation format
void
print_normal(FILE* out, ldns_rr* rr) {
    char* str = ldns_rr2str(rr);
    fprintf(out, "%s", str);
    free(str);
}

// Print an rr in rfc-3597 unknown type format
void
print_rr_as_3597(FILE* out, ldns_rr* rr) {
    uint8_t rdf_wire[MAX_LEN];
    memset(rdf_wire, 0, MAX_LEN);
    uint8_t* cur_wire;
    size_t wire_pos = 0;
    size_t cur_rdf = 0;
    size_t result_size = 0;
    size_t cur_pos = 0;

    for (cur_rdf = 0; cur_rdf < ldns_rr_rd_count(rr); cur_rdf++) {
        ldns_rdf2wire(&cur_wire, ldns_rr_rdf(rr, cur_rdf),
                      &result_size);
        memcpy(&rdf_wire[wire_pos], cur_wire, result_size);
        wire_pos += result_size;
        free(cur_wire);
    }

    char* owner_str = ldns_rdf2str(ldns_rr_owner(rr));
    char* class_str = ldns_rr_class2str(ldns_rr_get_class(rr));
    fprintf(out, 
            "%s    %u %s TYPE%u \\# %u ",
            owner_str,
            ldns_rr_ttl(rr),
            class_str,
            ldns_rr_get_type(rr),
            (unsigned int)wire_pos);
    free(owner_str);
    free(class_str);

    for (cur_pos = 0; cur_pos < wire_pos; cur_pos++) {
        fprintf(out, "%02x ", rdf_wire[cur_pos]);
    }
    fprintf(out, "\n");
}

// Print an rrsig RR in rfc-3597 unknown type format, if the
// covered type is in the given list
void
print_rrsig_as_3597(FILE* out, ldns_rr_type* convert_types,
                    size_t convert_types_len, ldns_rr* rr) {
    if (ldns_rr_get_type(rr) != LDNS_RR_TYPE_RRSIG) {
        printf("Error (bug): non-RRSIG RR passed to "
               "print_rrsig_as_3597\n");
    }

    if (is_convert_type(convert_types, convert_types_len,
            ldns_rdf2rr_type(ldns_rr_rrsig_typecovered(rr)))) {
        // ok just print this in 3597 completely (we could print
        // the RRSIG normally then replace the covered-type field
        // with TYPEXXX, but this is easier)
        fprintf(out, "; RRSIG:\n");
        print_rr_as_3597(out, rr);
    } else {
        print_normal(out, rr);
    }
}

// Print an rrsig RR in rfc-3597 unknown type format, if the
// covered type is in the given list
void
print_nsec_as_3597(FILE* out, ldns_rr_type* convert_types,
                   size_t convert_types_len, ldns_rr* rr) {
    size_t i;
    ldns_rdf* bitmap = NULL;

    if (ldns_rr_get_type(rr) == LDNS_RR_TYPE_NSEC) {
        bitmap = ldns_nsec_get_bitmap(rr);
    } else if (ldns_rr_get_type(rr) == LDNS_RR_TYPE_NSEC3) {
        bitmap = ldns_nsec3_bitmap(rr);
    } else {
        printf("Error (bug): non-NSEC(3) RR passed to "
               "print_nsec_as_3597\n");
    }

    for (i = 0; i < convert_types_len; ++i) {
        if (ldns_nsec_bitmap_covers_type(ldns_nsec_get_bitmap(rr),
                                         convert_types[i])) {
            fprintf(out, "; NSEC:\n");
            print_rr_as_3597(out, rr);
            return;
        }
    }
    print_normal(out, rr);
}

// Adds the given type to the list to convert
void
add_convert_type(ldns_rr_type* types, size_t* len, ldns_rr_type type) {
    if (*len < MAX_LEN) {
        types[(*len)++] = type;
    } else {
        printf("Error: too many conversion types requested\n");
    }
}

// Returns 1 if the given type is in the given list-to-convert
int
is_convert_type(ldns_rr_type* types, size_t len, ldns_rr_type type) {
    size_t i;
    for (i=0; i < len; ++i) {
        if (types[i] == type) {
            return 1;
        }
    }
    return 0;
}

int
main(int argc, char** argv) {
    int c;
    size_t cur_len = 0;
    size_t cur_arg = 0;
    char *str;
    unsigned int cur_line_nr = 0;
    ldns_rr *rr = NULL;
    ldns_status result;
    char *input_filename = NULL;
    FILE *input_file = NULL;
    char *output_filename = NULL;
    FILE *output_file = NULL;

    if (argc == 1) {
        usage();
        return 1;
    } else {
        while ((c = getopt (argc, argv, "hi:o:")) != -1)
            switch (c) {
                case 'h':
                    usage();
                    return 0;
                case 'i':
                    input_filename = optarg;
                    break;
                case 'o':
                    output_filename = optarg;
                    break;
                default:
                    usage();
                    return 1;
        }
    }

    // list of types to 3597-ify
    ldns_rdf *type_rdf = NULL;
    ldns_rr_type convert_types[MAX_LEN];
    size_t convert_types_len = 0;

    for (cur_arg = optind; cur_arg < argc; ++cur_arg) {
        // accept only mnemonic format for now
        if (ldns_str2rdf_type(&type_rdf,
                              argv[cur_arg]) == LDNS_STATUS_OK &&
            ldns_rdf2rr_type(type_rdf) != 0) {
            add_convert_type(convert_types, &convert_types_len,
                             ldns_rdf2rr_type(type_rdf));
            ldns_rdf_deep_free(type_rdf);
            type_rdf = NULL;
        } else {
            printf("Error: unknown RR type: %s\n", argv[cur_arg]);
            return 2;
        }
    }

    str = malloc(MAX_LEN);
    memset(str, 0, MAX_LEN);
    size_t len = MAX_LEN;

    if (input_filename != NULL) {
        input_file = fopen(input_filename, "r");
        if (input_file == NULL) {
            printf("Error: unable to open %s for reading: %s\n",
                   input_filename, strerror(errno));
        }
    } else {
        input_file = stdin;
    }

    if (output_filename != NULL) {
        output_file = fopen(output_filename, "w");
        if (output_file == NULL) {
            printf("Error: unable to open %s for writing: %s\n",
                   output_filename, strerror(errno));
        }
    } else {
        output_file = stdout;
    }

    while (getline(&str, &len, input_file) != -1) {
        cur_line_nr++;
        // skip some very basic things
        // (note that it will still unnecessarily error on a lot of
        // input)
        if (len == 0 || str[0] == ';') {
            continue;
        }

        result = ldns_rr_new_frm_str(&rr, str, 0, NULL, NULL);
        if (result != LDNS_STATUS_OK) {
            printf("Error parsing RR: %s\n",
                ldns_get_errorstr_by_id(result));
            printf("At line %u\n", cur_line_nr);
            printf("Line content: %s\n", str);
            return result;
        }
        // print in 3597 format
        if (is_convert_type(convert_types, convert_types_len,
                            ldns_rr_get_type(rr))) {
            print_rr_as_3597(output_file, rr);
        } else {
            // special case for NSEC and RRSIG; the types can appear
            // in RDATA (are there any others?)
            if (ldns_rr_get_type(rr) == LDNS_RR_TYPE_RRSIG) {
                print_rrsig_as_3597(output_file,
                                    convert_types, convert_types_len,
                                    rr);
            } else if (ldns_rr_get_type(rr) == LDNS_RR_TYPE_NSEC ||
                       ldns_rr_get_type(rr) == LDNS_RR_TYPE_NSEC3) {
                print_nsec_as_3597(output_file,
                                   convert_types, convert_types_len,
                                   rr);
            } else {
                print_normal(output_file,rr);
            }
        }
        ldns_rr_free(rr);
    }

    free(str);
    return 0;
}
