
1.1

- Modified how 'broken' zones are made: the 'brokenness' is now based on the
  zone apex, not the rrtype owner; this means that in a delegation from a
  correct zone to a 'broken' zone (such as bogussig.<zone>), the delegation
  itself is now correct and valid.
- Fixed 'unknownalgorithm' issue; it is no longer bogus, but correctly
  signed with the unregistered algorithm 200 (named 'twocents' for this
  occasion)
- ldns-sign-special and ldns-3597 have been removed from this repository,
  and added to the companion repository 'ldns-dns-workbench', which is
  a fork of ldns containing all necessary changes for the workbench.
  See https://github.com/SIDN/ldns-dns-workbench

1.0

- Initial Release