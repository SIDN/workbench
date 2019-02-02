/*
 * ldns-signzone signs a zone file
 * 
 * (c) NLnet Labs, 2005 - 2008
 * See the file LICENSE for the license
 * 
 * Modifications for the bad-dnssec tree by Jelte Jansen, SIDN 2013
 */

#include "config.h"
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>

#include <time.h>

#include <ldns/ldns.h>
#include <ldns/keys.h>

#ifdef HAVE_SSL
#include <openssl/conf.h>
#include <openssl/engine.h>
#endif /* HAVE_SSL */

#define MAX_FILENAME_LEN 250
int verbosity = 1;

#ifdef HAVE_SSL
#include <openssl/err.h>


/*
 * This is added to the original ldns-signzone.c
 */
ldns_rr_list *
my_ldns_sign_public(ldns_rr_list *rrset, ldns_key_list *keys)
{
	ldns_rr_list *signatures;
	ldns_rr_list *rrset_clone;
	ldns_rr *current_sig;
	ldns_rdf *b64rdf;
	ldns_key *current_key;
	size_t key_count;
	uint16_t i;
	ldns_buffer *sign_buf;
	ldns_rdf *new_owner;

	/* added to ldns_sign_public */
	char *cur_owner_str = ldns_rdf2str(ldns_rr_owner(ldns_rr_list_rr(rrset, 0)));;
	/* strings to look for the names of errors */
	static const char *bogussig = "bogussig.";
	static const char *unknownalgorithm = "unknownalgorithm.";
	static const char *sigexpired = "sigexpired.";
	static const char *signotincepted = "signotincepted.";
	time_t now;
	/* end addition */
	
	if (!rrset || ldns_rr_list_rr_count(rrset) < 1 || !keys) {
		return NULL;
	}

	new_owner = NULL;

	signatures = ldns_rr_list_new();

	/* prepare a signature and add all the know data
	 * prepare the rrset. Sign this together.  */
	rrset_clone = ldns_rr_list_clone(rrset);
	if (!rrset_clone) {
		return NULL;
	}

	/* make it canonical */
	for(i = 0; i < ldns_rr_list_rr_count(rrset_clone); i++) {
		ldns_rr_set_ttl(ldns_rr_list_rr(rrset_clone, i), 
			ldns_rr_ttl(ldns_rr_list_rr(rrset, 0)));
		ldns_rr2canonical(ldns_rr_list_rr(rrset_clone, i));
	}
	/* sort */
	ldns_rr_list_sort(rrset_clone);

	for (key_count = 0;
		key_count < ldns_key_list_key_count(keys);
		key_count++) {
		if (!ldns_key_use(ldns_key_list_key(keys, key_count))) {
			continue;
		}
		sign_buf = ldns_buffer_new(LDNS_MAX_PACKETLEN);
		if (!sign_buf) {
			ldns_rr_list_free(rrset_clone);
			ldns_rr_list_free(signatures);
			ldns_rdf_free(new_owner);
			return NULL;
		}
		b64rdf = NULL;

		current_key = ldns_key_list_key(keys, key_count);
		/* sign all RRs with keys that have ZSKbit, !SEPbit.
		   sign DNSKEY RRs with keys that have ZSKbit&SEPbit */
		if (ldns_key_flags(current_key) & LDNS_KEY_ZONE_KEY) {
			current_sig = ldns_create_empty_rrsig(rrset_clone,
			                                      current_key);

			/* Added to ldns_sign_public for the special signer */
			now = time(NULL);
			if (strncmp(signotincepted, cur_owner_str, 15) == 0) {
				(void)ldns_rr_rrsig_set_inception(current_sig,
					ldns_native2rdf_int32(LDNS_RDF_TYPE_TIME,
					                      now+315360000)); /* was 31536000 */
				(void)ldns_rr_rrsig_set_expiration(current_sig,
					ldns_native2rdf_int32(LDNS_RDF_TYPE_TIME,
					                      now+346896000)); /* was 63072000 */
			} else if (strncmp(sigexpired, cur_owner_str, 11) == 0) {
				(void)ldns_rr_rrsig_set_inception(current_sig,
					ldns_native2rdf_int32(LDNS_RDF_TYPE_TIME,
					                      now-63072000));
				(void)ldns_rr_rrsig_set_expiration(current_sig,
					ldns_native2rdf_int32(LDNS_RDF_TYPE_TIME,
					                      now-31536000));
			} else if (strncmp(unknownalgorithm, cur_owner_str, 17) == 0) {
				(void)ldns_rr_rrsig_set_algorithm(current_sig,
					ldns_native2rdf_int8(LDNS_RDF_TYPE_ALG, 200));
			}
			/* end addition to ldns_sign_public */

			/* right now, we have: a key, a semi-sig and an rrset. For
			 * which we can create the sig and base64 encode that and
			 * add that to the signature */

			if (ldns_rrsig2buffer_wire(sign_buf, current_sig)
			    != LDNS_STATUS_OK) {
				ldns_buffer_free(sign_buf);
				/* ERROR */
				ldns_rr_list_deep_free(rrset_clone);
				ldns_rr_free(current_sig);
				ldns_rr_list_deep_free(signatures);
				return NULL;
			}
			
			/* Added to ldns_sign_public for the special signer */
			if (strncmp(bogussig, cur_owner_str, 9) == 0) {
				sign_buf->_data[2] = 0xFF - sign_buf->_data[2];
			}
			/* end addition to ldns_sign_public */

			/* add the rrset in sign_buf */
			if (ldns_rr_list2buffer_wire(sign_buf, rrset_clone)
			    != LDNS_STATUS_OK) {
				ldns_buffer_free(sign_buf);
				ldns_rr_list_deep_free(rrset_clone);
				ldns_rr_free(current_sig);
				ldns_rr_list_deep_free(signatures);
				return NULL;
			}

			b64rdf = ldns_sign_public_buffer(sign_buf, current_key);

			if (!b64rdf) {
				/* signing went wrong */
				ldns_rr_list_deep_free(rrset_clone);
				ldns_rr_free(current_sig);
				ldns_rr_list_deep_free(signatures);
				return NULL;
			}

			ldns_rr_rrsig_set_sig(current_sig, b64rdf);

			/* push the signature to the signatures list */
			ldns_rr_list_push_rr(signatures, current_sig);
		}
		ldns_buffer_free(sign_buf); /* restart for the next key */
	}
	ldns_rr_list_deep_free(rrset_clone);

	return signatures;
ldns_rr_list *
ldns_sign_public(ldns_rr_list *rrset, ldns_key_list *keys)
{
	ldns_rr_list *signatures;
	ldns_rr_list *rrset_clone;
	ldns_rr *current_sig;
	ldns_rdf *b64rdf;
	ldns_key *current_key;
	size_t key_count;
	uint16_t i;
	ldns_buffer *sign_buf;
	ldns_rdf *new_owner;

	if (!rrset || ldns_rr_list_rr_count(rrset) < 1 || !keys) {
		return NULL;
	}

	new_owner = NULL;

	signatures = ldns_rr_list_new();

	/* prepare a signature and add all the know data
	 * prepare the rrset. Sign this together.  */
	rrset_clone = ldns_rr_list_clone(rrset);
	if (!rrset_clone) {
		return NULL;
	}

	/* make it canonical */
	for(i = 0; i < ldns_rr_list_rr_count(rrset_clone); i++) {
		ldns_rr_set_ttl(ldns_rr_list_rr(rrset_clone, i), 
			ldns_rr_ttl(ldns_rr_list_rr(rrset, 0)));
		ldns_rr2canonical(ldns_rr_list_rr(rrset_clone, i));
	}
	/* sort */
	ldns_rr_list_sort(rrset_clone);

	for (key_count = 0;
		key_count < ldns_key_list_key_count(keys);
		key_count++) {
		if (!ldns_key_use(ldns_key_list_key(keys, key_count))) {
			continue;
		}
		sign_buf = ldns_buffer_new(LDNS_MAX_PACKETLEN);
		if (!sign_buf) {
			ldns_rr_list_free(rrset_clone);
			ldns_rr_list_free(signatures);
			ldns_rdf_free(new_owner);
			return NULL;
		}
		b64rdf = NULL;

		current_key = ldns_key_list_key(keys, key_count);
		/* sign all RRs with keys that have ZSKbit, !SEPbit.
		   sign DNSKEY RRs with keys that have ZSKbit&SEPbit */
		if (ldns_key_flags(current_key) & LDNS_KEY_ZONE_KEY) {
			current_sig = ldns_create_empty_rrsig(rrset_clone,
			                                      current_key);

			/* right now, we have: a key, a semi-sig and an rrset. For
			 * which we can create the sig and base64 encode that and
			 * add that to the signature */

			if (ldns_rrsig2buffer_wire(sign_buf, current_sig)
			    != LDNS_STATUS_OK) {
				ldns_buffer_free(sign_buf);
				/* ERROR */
				ldns_rr_list_deep_free(rrset_clone);
				ldns_rr_free(current_sig);
				ldns_rr_list_deep_free(signatures);
				return NULL;
			}

			/* add the rrset in sign_buf */
			if (ldns_rr_list2buffer_wire(sign_buf, rrset_clone)
			    != LDNS_STATUS_OK) {
				ldns_buffer_free(sign_buf);
				ldns_rr_list_deep_free(rrset_clone);
				ldns_rr_free(current_sig);
				ldns_rr_list_deep_free(signatures);
				return NULL;
			}

			b64rdf = ldns_sign_public_buffer(sign_buf, current_key);

			if (!b64rdf) {
				/* signing went wrong */
				ldns_rr_list_deep_free(rrset_clone);
				ldns_rr_free(current_sig);
				ldns_rr_list_deep_free(signatures);
				return NULL;
			}

			ldns_rr_rrsig_set_sig(current_sig, b64rdf);

			/* push the signature to the signatures list */
			ldns_rr_list_push_rr(signatures, current_sig);
		}
		ldns_buffer_free(sign_buf); /* restart for the next key */
	}
	ldns_rr_list_deep_free(rrset_clone);

	return signatures;
}
}



// Below functions are modified so the above one is used

static void
ldns_key_list_filter_for_dnskey(ldns_key_list *key_list)
{
	int saw_ksk = 0;
	size_t i;
	for(i=0; i<ldns_key_list_key_count(key_list); i++)
		if((ldns_key_flags(ldns_key_list_key(key_list, i))&LDNS_KEY_SEP_KEY)) {
			saw_ksk = 1;
			break;
		}
	if(!saw_ksk)
		return;
	for(i=0; i<ldns_key_list_key_count(key_list); i++)
		if(!(ldns_key_flags(ldns_key_list_key(key_list, i))&LDNS_KEY_SEP_KEY))
			ldns_key_set_use(ldns_key_list_key(key_list, i), 0);
}

/** If there are no ZSKs use KSK as ZSK */
static void
ldns_key_list_filter_for_non_dnskey(ldns_key_list *key_list)
{
	int saw_zsk = 0;
	size_t i;
	for(i=0; i<ldns_key_list_key_count(key_list); i++)
		if(!(ldns_key_flags(ldns_key_list_key(key_list, i))&LDNS_KEY_SEP_KEY)) {
			saw_zsk = 1;
			break;
		}
	if(!saw_zsk)
		return;
	/* else filter all KSKs */
	for(i=0; i<ldns_key_list_key_count(key_list); i++)
		if((ldns_key_flags(ldns_key_list_key(key_list, i))&LDNS_KEY_SEP_KEY))
			ldns_key_set_use(ldns_key_list_key(key_list, i), 0);
}

ldns_status
my_ldns_dnssec_zone_create_rrsigs_flg( ldns_dnssec_zone *zone
				  , ldns_rr_list *new_rrs
				  , ldns_key_list *key_list
				  , int (*func)(ldns_rr *, void*)
				  , void *arg
				  , int flags
				  )
{
	ldns_status result = LDNS_STATUS_OK;

	ldns_rbnode_t *cur_node;
	ldns_rr_list *rr_list;

	ldns_dnssec_name *cur_name;
	ldns_dnssec_rrsets *cur_rrset;
	ldns_dnssec_rrs *cur_rr;

	ldns_rr_list *siglist;

	size_t i;

	int on_delegation_point = 0; /* handle partially occluded names */

	ldns_rr_list *pubkey_list = ldns_rr_list_new();
	for (i = 0; i<ldns_key_list_key_count(key_list); i++) {
		ldns_rr_list_push_rr( pubkey_list
				    , ldns_key2rr(ldns_key_list_key(
							key_list, i))
				    );
	}
	/* TODO: callback to see is list should be signed */
	/* TODO: remove 'old' signatures from signature list */
	cur_node = ldns_rbtree_first(zone->names);
	while (cur_node != LDNS_RBTREE_NULL) {
		cur_name = (ldns_dnssec_name *) cur_node->data;

		if (!cur_name->is_glue) {
			on_delegation_point = ldns_dnssec_rrsets_contains_type(
					cur_name->rrsets, LDNS_RR_TYPE_NS)
				&& !ldns_dnssec_rrsets_contains_type(
					cur_name->rrsets, LDNS_RR_TYPE_SOA);
			cur_rrset = cur_name->rrsets;
			while (cur_rrset) {
				/* reset keys to use */
				ldns_key_list_set_use(key_list, true);

				/* walk through old sigs, remove the old,
				   and mark which keys (not) to use) */
				cur_rrset->signatures =
					ldns_dnssec_remove_signatures(cur_rrset->signatures,
											key_list,
											func,
											arg);
				if(!(flags&LDNS_SIGN_DNSKEY_WITH_ZSK) &&
					cur_rrset->type == LDNS_RR_TYPE_DNSKEY) {
					ldns_key_list_filter_for_dnskey(key_list);
				}

				if(cur_rrset->type != LDNS_RR_TYPE_DNSKEY) {
					ldns_key_list_filter_for_non_dnskey(key_list);
				}

				/* TODO: just set count to zero? */
				rr_list = ldns_rr_list_new();

				cur_rr = cur_rrset->rrs;
				while (cur_rr) {
					ldns_rr_list_push_rr(rr_list, cur_rr->rr);
					cur_rr = cur_rr->next;
				}

				/* only sign non-delegation RRsets */
				/* (glue should have been marked earlier, 
				 *  except on the delegation points itself) */
				if (!on_delegation_point ||
						ldns_rr_list_type(rr_list) 
							== LDNS_RR_TYPE_DS ||
						ldns_rr_list_type(rr_list) 
							== LDNS_RR_TYPE_NSEC ||
						ldns_rr_list_type(rr_list) 
							== LDNS_RR_TYPE_NSEC3) {
					siglist = my_ldns_sign_public(rr_list, key_list);
					for (i = 0; i < ldns_rr_list_rr_count(siglist); i++) {
						if (cur_rrset->signatures) {
							result = ldns_dnssec_rrs_add_rr(cur_rrset->signatures,
											   ldns_rr_list_rr(siglist,
														    i));
						} else {
							cur_rrset->signatures = ldns_dnssec_rrs_new();
							cur_rrset->signatures->rr =
								ldns_rr_list_rr(siglist, i);
						}
						if (new_rrs) {
							ldns_rr_list_push_rr(new_rrs,
												 ldns_rr_list_rr(siglist,
															  i));
						}
					}
					ldns_rr_list_free(siglist);
				}

				ldns_rr_list_free(rr_list);

				cur_rrset = cur_rrset->next;
			}

			/* sign the nsec */
			ldns_key_list_set_use(key_list, true);
			cur_name->nsec_signatures =
				ldns_dnssec_remove_signatures(cur_name->nsec_signatures,
										key_list,
										func,
										arg);
			ldns_key_list_filter_for_non_dnskey(key_list);

			rr_list = ldns_rr_list_new();
			ldns_rr_list_push_rr(rr_list, cur_name->nsec);
			siglist = my_ldns_sign_public(rr_list, key_list);

			for (i = 0; i < ldns_rr_list_rr_count(siglist); i++) {
				if (cur_name->nsec_signatures) {
					result = ldns_dnssec_rrs_add_rr(cur_name->nsec_signatures,
									   ldns_rr_list_rr(siglist, i));
				} else {
					cur_name->nsec_signatures = ldns_dnssec_rrs_new();
					cur_name->nsec_signatures->rr =
						ldns_rr_list_rr(siglist, i);
				}
				if (new_rrs) {
					ldns_rr_list_push_rr(new_rrs,
								 ldns_rr_list_rr(siglist, i));
				}
			}

			ldns_rr_list_free(siglist);
			ldns_rr_list_free(rr_list);
		}
		cur_node = ldns_rbtree_next(cur_node);
	}

	ldns_rr_list_deep_free(pubkey_list);
	return result;
}

ldns_status
my_ldns_dnssec_zone_sign_flg(ldns_dnssec_zone *zone,
				  ldns_rr_list *new_rrs,
				  ldns_key_list *key_list,
				  int (*func)(ldns_rr *, void *),
				  void *arg,
				  int flags)
{
	ldns_status result = LDNS_STATUS_OK;

	if (!zone || !new_rrs || !key_list) {
		return LDNS_STATUS_ERR;
	}

	/* zone is already sorted */
	result = ldns_dnssec_zone_mark_glue(zone);
	if (result != LDNS_STATUS_OK) {
		return result;
	}

	/* check whether we need to add nsecs */
	if (zone->names && !((ldns_dnssec_name *)zone->names->root->data)->nsec) {
		result = ldns_dnssec_zone_create_nsecs(zone, new_rrs);
		if (result != LDNS_STATUS_OK) {
			return result;
		}
	}

	result = my_ldns_dnssec_zone_create_rrsigs_flg(zone,
					new_rrs,
					key_list,
					func,
					arg,
					flags);

	return result;
}

/*
 * End of custom additions
 */



static void
usage(FILE *fp, const char *prog) {
	fprintf(fp, "%s [OPTIONS] zonefile key [key [key]]\n", prog);
	fprintf(fp, "  signs the zone with the given key(s)\n");
	fprintf(fp, "  -b\t\tuse layout in signed zone and print comments DNSSEC records\n");
	fprintf(fp, "  -d\t\tused keys are not added to the zone\n");
	fprintf(fp, "  -e <date>\texpiration date\n");
	fprintf(fp, "  -f <file>\toutput zone to file (default <name>.signed)\n");
	fprintf(fp, "  -i <date>\tinception date\n");
	fprintf(fp, "  -o <domain>\torigin for the zone\n");
	fprintf(fp, "  -v\t\tprint version and exit\n");
	fprintf(fp, "  -A\t\tsign DNSKEY with all keys instead of minimal\n");
	fprintf(fp, "  -E <name>\tuse <name> as the crypto engine for signing\n");
	fprintf(fp, "           \tThis can have a lot of extra options, see the manual page for more info\n");
	fprintf(fp, "  -k <id>,<int>\tuse key id with algorithm int from engine\n");
	fprintf(fp, "  -K <id>,<int>\tuse key id with algorithm int from engine as KSK\n");
	fprintf(fp, "\t\tif no key is given (but an external one is used through the engine support, it might be necessary to provide the right algorithm number.\n");
	fprintf(fp, "  -n\t\tuse NSEC3 instead of NSEC.\n");
	fprintf(fp, "\t\tIf you use NSEC3, you can specify the following extra options:\n");
	fprintf(fp, "\t\t-a [algorithm] hashing algorithm\n");
	fprintf(fp, "\t\t-t [number] number of hash iterations\n");
	fprintf(fp, "\t\t-s [string] salt\n");
	fprintf(fp, "\t\t-p set the opt-out flag on all nsec3 rrs\n");
	fprintf(fp, "\n");
	fprintf(fp, "  keys must be specified by their base name (usually K<name>+<alg>+<id>),\n");
	fprintf(fp, "  i.e. WITHOUT the .private extension.\n");
	fprintf(fp, "  If the public part of the key is not present in the zone, the DNSKEY RR\n");
	fprintf(fp, "  will be read from the file called <base name>.key. If that does not exist,\n");
	fprintf(fp, "  a default DNSKEY will be generated from the private key and added to the zone.\n");
	fprintf(fp, "  A date can be a timestamp (seconds since the epoch), or of\n  the form <YYYYMMdd[hhmmss]>\n");
}

static void check_tm(struct tm tm)
{
	if (tm.tm_year < 70) {
		fprintf(stderr, "You cannot specify dates before 1970\n");
		exit(EXIT_FAILURE);
	}
	if (tm.tm_mon < 0 || tm.tm_mon > 11) {
		fprintf(stderr, "The month must be in the range 1 to 12\n");
		exit(EXIT_FAILURE);
	}
	if (tm.tm_mday < 1 || tm.tm_mday > 31) {
		fprintf(stderr, "The day must be in the range 1 to 31\n");
		exit(EXIT_FAILURE);
	}
	
	if (tm.tm_hour < 0 || tm.tm_hour > 23) {
		fprintf(stderr, "The hour must be in the range 0-23\n");
		exit(EXIT_FAILURE);
	}

	if (tm.tm_min < 0 || tm.tm_min > 59) {
		fprintf(stderr, "The minute must be in the range 0-59\n");
		exit(EXIT_FAILURE);
	}

	if (tm.tm_sec < 0 || tm.tm_sec > 59) {
		fprintf(stderr, "The second must be in the range 0-59\n");
		exit(EXIT_FAILURE);
	}

}

/*
 * if the ttls are different, make them equal
 * if one of the ttls equals LDNS_DEFAULT_TTL, that one is changed
 * otherwise, rr2 will get the ttl of rr1
 * 
 * prints a warning if a non-default TTL is changed
 */
static void
equalize_ttls(ldns_rr *rr1, ldns_rr *rr2, uint32_t default_ttl)
{
	uint32_t ttl1, ttl2;
	
	ttl1 = ldns_rr_ttl(rr1);
	ttl2 = ldns_rr_ttl(rr2);
	
	if (ttl1 != ttl2) {
		if (ttl1 == default_ttl) {
			ldns_rr_set_ttl(rr1, ttl2);
		} else if (ttl2 == default_ttl) {
			ldns_rr_set_ttl(rr2, ttl1);
		} else {
			ldns_rr_set_ttl(rr2, ttl1);
			fprintf(stderr, 
			        "warning: changing non-default TTL %u to %u\n",
			        (unsigned int) ttl2, (unsigned int)  ttl1);
		}
	}
}

static void
equalize_ttls_rr_list(ldns_rr_list *rr_list, ldns_rr *rr, uint32_t default_ttl)
{
	size_t i;
	ldns_rr *cur_rr;
	
	for (i = 0; i < ldns_rr_list_rr_count(rr_list); i++) {
		cur_rr = ldns_rr_list_rr(rr_list, i);
		if (ldns_rr_compare_no_rdata(cur_rr, rr) == 0) {
			equalize_ttls(cur_rr, rr, default_ttl);
		}
	}
}

static ldns_rr *
find_key_in_zone(ldns_rr *pubkey_gen, ldns_zone *zone) {
	size_t key_i;
	ldns_rr *pubkey;
	
	for (key_i = 0;
		key_i < ldns_rr_list_rr_count(ldns_zone_rrs(zone));
		key_i++) {
		pubkey = ldns_rr_list_rr(ldns_zone_rrs(zone), key_i);
		if (ldns_rr_get_type(pubkey) == LDNS_RR_TYPE_DNSKEY &&
			(ldns_calc_keytag(pubkey)
				==
				ldns_calc_keytag(pubkey_gen) ||
					 /* KSK has gen-keytag + 1 */
					 ldns_calc_keytag(pubkey)
					 ==
					 ldns_calc_keytag(pubkey_gen) + 1) 
			   ) {
				if (verbosity >= 2) {
					fprintf(stderr, "Found it in the zone!\n");
				}
				return pubkey;
		}
	}
	return NULL;
}

static ldns_rr *
find_key_in_file(const char *keyfile_name_base, ldns_key* ATTR_UNUSED(key),
	uint32_t zone_ttl)
{
	char *keyfile_name;
	FILE *keyfile;
	int line_nr;
	uint32_t default_ttl = zone_ttl;

	ldns_rr *pubkey = NULL;
	keyfile_name = LDNS_XMALLOC(char,
	                            strlen(keyfile_name_base) + 5);
	snprintf(keyfile_name,
		 strlen(keyfile_name_base) + 5,
		 "%s.key",
		 keyfile_name_base);
	if (verbosity >= 2) {
		fprintf(stderr, "Trying to read %s\n", keyfile_name);
	}
	keyfile = fopen(keyfile_name, "r");
	line_nr = 0;
	if (keyfile) {
		if (ldns_rr_new_frm_fp_l(&pubkey,
					 keyfile,
					 &default_ttl,
					 NULL,
					 NULL,
					 &line_nr) ==
		    LDNS_STATUS_OK) {
			if (verbosity >= 2) {
				printf("Key found in file: %s\n", keyfile_name);
			}
		}
		fclose(keyfile);
	}
	LDNS_FREE(keyfile_name);
	return pubkey;
}

/* this function tries to find the specified keys either in the zone that
 * has been read, or in a <basename>.key file. If the key is not found,
 * a public key is generated, and it is assumed the key is a ZSK
 * 
 * if add_keys is true; the DNSKEYs are added to the zone prior to signing
 * if it is false, they are not added.
 * Even if keys are not added, the function is still needed, to check
 * whether keys of which we only have key data are KSKs or ZSKS
 */
static void
find_or_create_pubkey(const char *keyfile_name_base, ldns_key *key, ldns_zone *orig_zone, bool add_keys, uint32_t default_ttl) {
	ldns_rr *pubkey_gen, *pubkey;
	int key_in_zone;
	
	if (default_ttl == LDNS_DEFAULT_TTL) {
		default_ttl = ldns_rr_ttl(ldns_zone_soa(orig_zone));
	}

	if (!ldns_key_pubkey_owner(key)) {
		ldns_key_set_pubkey_owner(key, ldns_rdf_clone(ldns_rr_owner(ldns_zone_soa(orig_zone))));
	}

	/* find the public key in the zone, or in a
	 * seperate file
	 * we 'generate' one anyway, 
	 * then match that to any present in the zone,
	 * if it matches, we drop our own. If not,
	 * we try to see if there is a .key file present.
	 * If not, we use our own generated one, with
	 * some default values 
	 *
	 * Even if -d (do-not-add-keys) is specified, 
	 * we still need to do this, because we need
	 * to have any key flags that are set this way
	 */
	pubkey_gen = ldns_key2rr(key);
	ldns_rr_set_ttl(pubkey_gen, default_ttl);

	if (verbosity >= 2) {
		fprintf(stderr,
			   "Looking for key with keytag %u or %u\n",
			   (unsigned int) ldns_calc_keytag(pubkey_gen),
			   (unsigned int) ldns_calc_keytag(pubkey_gen)+1
			   );
	}

	pubkey = find_key_in_zone(pubkey_gen, orig_zone);
	key_in_zone = 1;
	if (!pubkey) {
		key_in_zone = 0;
		/* it was not in the zone, try to read a .key file */
		pubkey = find_key_in_file(keyfile_name_base, key, default_ttl);
		if (!pubkey && !(ldns_key_flags(key) & LDNS_KEY_SEP_KEY)) {
			/* maybe it is a ksk? */
			ldns_key_set_keytag(key, ldns_key_keytag(key) + 1);
			pubkey = find_key_in_file(keyfile_name_base, key, default_ttl);
			if (!pubkey) {
				/* ok, no file, set back to ZSK */
				ldns_key_set_keytag(key, ldns_key_keytag(key) - 1);
			}
		}
		if(pubkey && ldns_dname_compare(ldns_rr_owner(pubkey), ldns_rr_owner(ldns_zone_soa(orig_zone))) != 0) {
			fprintf(stderr, "Error %s.key has wrong name: %s, should be: %s\n",
				keyfile_name_base, ldns_rdf2str(ldns_rr_owner(pubkey)), ldns_rdf2str(ldns_rr_owner(ldns_zone_soa(orig_zone))));
			exit(EXIT_FAILURE); /* leak rdf2str, but we exit */
		}
	}
	
	if (!pubkey) {
		/* okay, no public key found,
		   just use our generated one */
		pubkey = pubkey_gen;
		if (verbosity >= 2) {
			fprintf(stderr, "Not in zone, no .key file, generating ZSK DNSKEY from private key data\n");
		}
	} else {
		ldns_rr_free(pubkey_gen);
	}
	ldns_key_set_flags(key, ldns_rdf2native_int16(ldns_rr_rdf(pubkey, 0)));
	ldns_key_set_keytag(key, ldns_calc_keytag(pubkey));
	
	if (add_keys && !key_in_zone) {
		equalize_ttls_rr_list(ldns_zone_rrs(orig_zone), pubkey, default_ttl);
		ldns_zone_push_rr(orig_zone, pubkey);
	}
}

void
strip_dnssec_records(ldns_zone *zone)
{
	ldns_rr_list *new_list;
	ldns_rr *cur_rr;
	
	new_list = ldns_rr_list_new();
	
	while ((cur_rr = ldns_rr_list_pop_rr(ldns_zone_rrs(zone)))) {
		if (ldns_rr_get_type(cur_rr) == LDNS_RR_TYPE_RRSIG ||
		    ldns_rr_get_type(cur_rr) == LDNS_RR_TYPE_NSEC ||
		    ldns_rr_get_type(cur_rr) == LDNS_RR_TYPE_NSEC3
		   ) {
			
			ldns_rr_free(cur_rr);
		} else {
			ldns_rr_list_push_rr(new_list, cur_rr);
		}
	}
	ldns_rr_list_free(ldns_zone_rrs(zone));
	ldns_zone_set_rrs(zone, new_list);
}

int
main(int argc, char *argv[])
{
	const char *zonefile_name;
	FILE *zonefile = NULL;
	int line_nr = 0;
	int c;
	int argi;
	ENGINE *engine = NULL;

	ldns_zone *orig_zone;
	ldns_rr_list *orig_rrs = NULL;
	ldns_rr *orig_soa = NULL;
	ldns_dnssec_zone *signed_zone;

	char *keyfile_name_base;
	char *keyfile_name = NULL;
	FILE *keyfile = NULL;
	ldns_key *key = NULL;
	ldns_key_list *keys;
	ldns_status s;
	size_t i;
	ldns_rr_list *added_rrs;

	char *outputfile_name = NULL;
	FILE *outputfile;
	
	/* tmp vars for engine keys */
	char *eng_key_l;
	size_t eng_key_id_len;
	char *eng_key_id;
	int eng_key_algo;
	
	bool use_nsec3 = false;
	int signflags = 0;

	/* Add the given keys to the zone if they are not yet present */
	bool add_keys = true;
	uint8_t nsec3_algorithm = 1;
	uint8_t nsec3_flags = 0;
	size_t nsec3_iterations_cmd = 1;
	uint16_t nsec3_iterations = 1;
	uint8_t nsec3_salt_length = 0;
	uint8_t *nsec3_salt = NULL;
	
	/* we need to know the origin before reading ksk's,
	 * so keep an array of filenames until we know it
	 */
	struct tm tm;
	uint32_t inception;
	uint32_t expiration;
	ldns_rdf *origin = NULL;
	uint32_t ttl = LDNS_DEFAULT_TTL;
	ldns_rr_class class = LDNS_RR_CLASS_IN;	
	
	char *prog = strdup(argv[0]);
	ldns_status result;

	ldns_output_format_storage fmt_st;
	ldns_output_format* fmt = ldns_output_format_init(&fmt_st);
	
	inception = 0;
	expiration = 0;
	
	keys = ldns_key_list_new();

	OPENSSL_config(NULL);

	while ((c = getopt(argc, argv, "a:bde:f:i:k:lno:ps:t:vAE:K:")) != -1) {
		switch (c) {
		case 'a':
			nsec3_algorithm = (uint8_t) atoi(optarg);
			if (nsec3_algorithm != 1) {
				fprintf(stderr, "Bad NSEC3 algorithm, only RSASHA1 allowed\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'b':
			ldns_output_format_set(fmt, LDNS_COMMENT_FLAGS
						  | LDNS_COMMENT_LAYOUT      
						  | LDNS_COMMENT_NSEC3_CHAIN
						  | LDNS_COMMENT_BUBBLEBABBLE);
			break;
		case 'd':
			add_keys = false;
			break;
		case 'e':
			/* try to parse YYYYMMDD first,
			 * if that doesn't work, it
			 * should be a timestamp (seconds since epoch)
			 */
			memset(&tm, 0, sizeof(tm));

			if (strlen(optarg) == 8 &&
			    sscanf(optarg, "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday)
			    ) {
			   	tm.tm_year -= 1900;
			   	tm.tm_mon--;
			   	check_tm(tm);
				expiration = 
					(uint32_t) ldns_mktime_from_utc(&tm);
			} else if (strlen(optarg) == 14 &&
					 sscanf(optarg, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec)
					 ) {
			   	tm.tm_year -= 1900;
			   	tm.tm_mon--;
			   	check_tm(tm);
				expiration = 
					(uint32_t) ldns_mktime_from_utc(&tm);
			} else {
				expiration = (uint32_t) atol(optarg);
			}
			break;
		case 'f':
			outputfile_name = LDNS_XMALLOC(char, MAX_FILENAME_LEN);
			strncpy(outputfile_name, optarg, MAX_FILENAME_LEN);
			break;
		case 'i':
			memset(&tm, 0, sizeof(tm));

			if (strlen(optarg) == 8 &&
			    sscanf(optarg, "%4d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday)
			    ) {
			   	tm.tm_year -= 1900;
			   	tm.tm_mon--;
			   	check_tm(tm);
				inception = 
					(uint32_t) ldns_mktime_from_utc(&tm);
			} else if (strlen(optarg) == 14 &&
					 sscanf(optarg, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec)
					 ) {
			   	tm.tm_year -= 1900;
			   	tm.tm_mon--;
			   	check_tm(tm);
				inception = 
					(uint32_t) ldns_mktime_from_utc(&tm);
			} else {
				inception = (uint32_t) atol(optarg);
			}
			break;
		case 'n':
			use_nsec3 = true;
			break;
		case 'o':
			if (ldns_str2rdf_dname(&origin, optarg) != LDNS_STATUS_OK) {
				fprintf(stderr, "Bad origin, not a correct domain name\n");
				usage(stderr, prog);
				exit(EXIT_FAILURE);
			}
			break;
		case 'p':
			nsec3_flags = nsec3_flags | LDNS_NSEC3_VARS_OPTOUT_MASK;
			break;
		case 'v':
			printf("zone signer version %s (ldns version %s)\n", LDNS_VERSION, ldns_version());
			exit(EXIT_SUCCESS);
			break;
		case 'A':
			signflags |= LDNS_SIGN_DNSKEY_WITH_ZSK;
			break;
		case 'E':
			ENGINE_load_builtin_engines();
			ENGINE_load_dynamic();
			ENGINE_load_cryptodev();
			engine = ENGINE_by_id(optarg);
			if (!engine) {
				printf("No such engine: %s\n", optarg);
				engine = ENGINE_get_first();
				printf("Available engines:\n");
				while (engine) {
					printf("%s\n", ENGINE_get_id(engine));
					engine = ENGINE_get_next(engine);
				}
				exit(EXIT_FAILURE);
			} else {
				if (!ENGINE_init(engine)) {
					printf("The engine couldn't initialize\n");
					exit(EXIT_FAILURE);
				}
				ENGINE_set_default_RSA(engine);
				ENGINE_set_default_DSA(engine);
				ENGINE_set_default(engine, 0);
			}
			break;
		case 'k':
			eng_key_l = strchr(optarg, ',');
			if (eng_key_l && strlen(eng_key_l) > 1) {
				if (eng_key_l > optarg) {
					eng_key_id_len = (size_t) (eng_key_l - optarg);
					eng_key_id = malloc(eng_key_id_len + 1);
					memcpy(eng_key_id, optarg, eng_key_id_len);
					eng_key_id[eng_key_id_len] = '\0';
				} else {
					/* no id given, use default from engine */
					eng_key_id = NULL;
				}

				eng_key_algo = atoi(eng_key_l + 1);

				printf("Engine key id: %s, algo %d\n", eng_key_id, eng_key_algo);

				s = ldns_key_new_frm_engine(&key, engine, eng_key_id, eng_key_algo);
				if (s == LDNS_STATUS_OK) {
					/* must be dnssec key */
					switch (ldns_key_algorithm(key)) {
					case LDNS_SIGN_RSAMD5:
					case LDNS_SIGN_RSASHA1:
					case LDNS_SIGN_RSASHA1_NSEC3:
					case LDNS_SIGN_RSASHA256:
					case LDNS_SIGN_RSASHA512:
					case LDNS_SIGN_DSA:
					case LDNS_SIGN_DSA_NSEC3:
					case LDNS_SIGN_ECC_GOST:
#ifdef USE_ECDSA
					case LDNS_SIGN_ECDSAP256SHA256:
					case LDNS_SIGN_ECDSAP384SHA384:
#endif
						ldns_key_list_push_key(keys, key);
						/*printf("Added key at %p:\n", key);*/
						/*ldns_key_print(stdout, key);*/
						break;
					default:
						fprintf(stderr, "Warning, key not suitable for signing, ignoring key with algorithm %u\n", ldns_key_algorithm(key));
						break;
					}
					if (expiration != 0) {
						ldns_key_set_expiration(key,
								expiration);
					}
					if (inception != 0) {
						ldns_key_set_inception(key,
								inception);
					}
				} else {
					printf("Error reading key '%s' from engine: %s\n", eng_key_id, ldns_get_errorstr_by_id(s));
					#ifdef HAVE_SSL
							if (ERR_peek_error()) {
								ERR_load_crypto_strings();
								ERR_print_errors_fp(stderr);
								ERR_free_strings();
							}
					#endif
					exit(EXIT_FAILURE);
				}

				if (eng_key_id) {
					free(eng_key_id);
				}
			} else {
				printf("Error: bad engine key specification (should be: -k <id>,<algorithm>)).\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'K':
			printf("Not implemented yet\n");
			exit(EXIT_FAILURE);
			break;
		case 's':
			if (strlen(optarg) % 2 != 0) {
				fprintf(stderr, "Salt value is not valid hex data, not a multiple of 2 characters\n");
				exit(EXIT_FAILURE);
			}
			nsec3_salt_length = (uint8_t) strlen(optarg) / 2;
			nsec3_salt = LDNS_XMALLOC(uint8_t, nsec3_salt_length);
			for (c = 0; c < (int) strlen(optarg); c += 2) {
				if (isxdigit((int) optarg[c]) && isxdigit((int) optarg[c+1])) {
					nsec3_salt[c/2] = (uint8_t) ldns_hexdigit_to_int(optarg[c]) * 16 +
						ldns_hexdigit_to_int(optarg[c+1]);
				} else {
					fprintf(stderr, "Salt value is not valid hex data.\n");
					exit(EXIT_FAILURE);
				}
			}

			break;
		case 't':
			nsec3_iterations_cmd = (size_t) atol(optarg);
			if (nsec3_iterations_cmd > LDNS_NSEC3_MAX_ITERATIONS) {
				fprintf(stderr, "Iterations count can not exceed %u, quitting\n", LDNS_NSEC3_MAX_ITERATIONS);
				exit(EXIT_FAILURE);
			}
			nsec3_iterations = (uint16_t) nsec3_iterations_cmd;
			break;
		default:
			usage(stderr, prog);
			exit(EXIT_SUCCESS);
		}
	}
	
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		printf("Error: not enough arguments\n");
		usage(stdout, prog);
		exit(EXIT_FAILURE);
	} else {
		zonefile_name = argv[0];
	}

	/* read zonefile first to find origin if not specified */
	
	if (strncmp(zonefile_name, "-", 2) == 0) {
		s = ldns_zone_new_frm_fp_l(&orig_zone,
					   stdin,
					   origin,
					   ttl,
					   class,
					   &line_nr);
			if (s != LDNS_STATUS_OK) {
				fprintf(stderr, "Zone not read, error: %s at stdin line %d\n", 
					   ldns_get_errorstr_by_id(s),
					   line_nr);
				exit(EXIT_FAILURE);
			} else {
				orig_soa = ldns_zone_soa(orig_zone);
				if (!orig_soa) {
					fprintf(stderr,
						   "Error reading zonefile: missing SOA record\n");
					exit(EXIT_FAILURE);
				}
				orig_rrs = ldns_zone_rrs(orig_zone);
				if (!orig_rrs) {
					fprintf(stderr,
						   "Error reading zonefile: no resource records\n");
					exit(EXIT_FAILURE);
				}
			}
	} else {
		zonefile = fopen(zonefile_name, "r");
		
		if (!zonefile) {
			fprintf(stderr,
				   "Error: unable to read %s (%s)\n",
				   zonefile_name,
				   strerror(errno));
			exit(EXIT_FAILURE);
		} else {
			s = ldns_zone_new_frm_fp_l(&orig_zone,
			                           zonefile,
			                           origin,
			                           ttl,
			                           class,
			                           &line_nr);
			if (s != LDNS_STATUS_OK) {
				fprintf(stderr, "Zone not read, error: %s at %s line %d\n", 
					   ldns_get_errorstr_by_id(s), 
					   zonefile_name, line_nr);
				exit(EXIT_FAILURE);
			} else {
				orig_soa = ldns_zone_soa(orig_zone);
				if (!orig_soa) {
					fprintf(stderr,
						   "Error reading zonefile: missing SOA record\n");
					exit(EXIT_FAILURE);
				}
				orig_rrs = ldns_zone_rrs(orig_zone);
				if (!orig_rrs) {
					fprintf(stderr,
						   "Error reading zonefile: no resource records\n");
					exit(EXIT_FAILURE);
				}
			}
			fclose(zonefile);
		}
	}

	/* read the ZSKs */
	argi = 1;
	while (argi < argc) {
		keyfile_name_base = argv[argi];
		keyfile_name = LDNS_XMALLOC(char, strlen(keyfile_name_base) + 9);
		snprintf(keyfile_name,
			    strlen(keyfile_name_base) + 9,
			    "%s.private",
			    keyfile_name_base);
		keyfile = fopen(keyfile_name, "r");
		line_nr = 0;
		if (!keyfile) {
			fprintf(stderr,
				   "Error: unable to read %s: %s\n",
				   keyfile_name,
				   strerror(errno));
		} else {
			s = ldns_key_new_frm_fp_l(&key, keyfile, &line_nr);
			fclose(keyfile);
			if (s == LDNS_STATUS_OK) {
				/* set times in key? they will end up
				   in the rrsigs
				*/
				if (expiration != 0) {
					ldns_key_set_expiration(key, expiration);
				}
				if (inception != 0) {
					ldns_key_set_inception(key, inception);
				}

				LDNS_FREE(keyfile_name);
				
				ldns_key_list_push_key(keys, key);
			} else {
				fprintf(stderr, "Error reading key from %s at line %d: %s\n", argv[argi], line_nr, ldns_get_errorstr_by_id(s));
			}
		}
		/* and, if not unset by -p, find or create the corresponding DNSKEY record */
		if (key) {
			find_or_create_pubkey(keyfile_name_base, key,
			                      orig_zone, add_keys, ttl);
		}
		argi++;
	}
	
	if (ldns_key_list_key_count(keys) < 1) {
		fprintf(stderr, "Error: no keys to sign with. Aborting.\n\n");
		usage(stderr, prog);
		exit(EXIT_FAILURE);
	}

	signed_zone = ldns_dnssec_zone_new();
	if (ldns_dnssec_zone_add_rr(signed_zone, ldns_zone_soa(orig_zone)) !=
	    LDNS_STATUS_OK) {
		fprintf(stderr,
		  "Error adding SOA to dnssec zone, skipping record\n");
	}
	
	for (i = 0;
	     i < ldns_rr_list_rr_count(ldns_zone_rrs(orig_zone));
	     i++) {
		if (ldns_dnssec_zone_add_rr(signed_zone, 
		         ldns_rr_list_rr(ldns_zone_rrs(orig_zone), 
		         i)) !=
		    LDNS_STATUS_OK) {
			fprintf(stderr,
			        "Error adding RR to dnssec zone");
			fprintf(stderr, ", skipping record:\n");
			ldns_rr_print(stderr, 
			  ldns_rr_list_rr(ldns_zone_rrs(orig_zone), i));
		}
	}

	/* list to store newly created rrs, so we can free them later */
	added_rrs = ldns_rr_list_new();

	if (use_nsec3) {
		result = ldns_dnssec_zone_sign_nsec3_flg_mkmap(signed_zone,
			added_rrs,
			keys,
			ldns_dnssec_default_replace_signatures,
			NULL,
			nsec3_algorithm,
			nsec3_flags,
			nsec3_iterations,
			nsec3_salt_length,
			nsec3_salt,
			signflags,
			&fmt_st.hashmap);
	} else {
		result = my_ldns_dnssec_zone_sign_flg(signed_zone,
				added_rrs,
				keys,
				ldns_dnssec_default_replace_signatures,
				NULL,
				signflags);
	}
	if (result != LDNS_STATUS_OK) {
		fprintf(stderr, "Error signing zone: %s\n",
			   ldns_get_errorstr_by_id(result));
	}

	if (!outputfile_name) {
		outputfile_name = LDNS_XMALLOC(char, MAX_FILENAME_LEN);
		snprintf(outputfile_name, MAX_FILENAME_LEN, "%s.signed", zonefile_name);
	}

	if (signed_zone) {
		if (strncmp(outputfile_name, "-", 2) == 0) {
			ldns_dnssec_zone_print(stdout, signed_zone);
		} else {
			outputfile = fopen(outputfile_name, "w");
			if (!outputfile) {
				fprintf(stderr, "Unable to open %s for writing: %s\n",
					   outputfile_name, strerror(errno));
			} else {
				ldns_dnssec_zone_print_fmt(
						outputfile, fmt, signed_zone);
				fclose(outputfile);
			}
		}
	} else {
		fprintf(stderr, "Error signing zone.\n");

#ifdef HAVE_SSL
		if (ERR_peek_error()) {
			ERR_load_crypto_strings();
			ERR_print_errors_fp(stderr);
			ERR_free_strings();
		}
#endif
		exit(EXIT_FAILURE);
	}
	
	ldns_key_list_free(keys);
	/* since the ldns_rr records are pointed to in both the ldns_zone
	 * and the ldns_dnssec_zone, we can either deep_free the
	 * dnssec_zone and 'shallow' free the original zone and added
	 * records, or the other way around
	 */
	ldns_dnssec_zone_free(signed_zone);
	ldns_zone_deep_free(orig_zone);
	ldns_rr_list_deep_free(added_rrs);
	
	LDNS_FREE(outputfile_name);
	
	CRYPTO_cleanup_all_ex_data();

	free(prog);
	exit(EXIT_SUCCESS);
}
#else
int
main(int argc, char **argv)
{
	fprintf(stderr, "ldns-signzone needs OpenSSL support, which has not been compiled in\n");
	return 1;
}
#endif /* HAVE_SSL */
