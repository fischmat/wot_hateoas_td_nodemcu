#ifndef __WOT_SPARQL__
#define __WOT_SPARQL__

namespace wot {

/**
* Retrieves equivalent classes and/or resources for a given class or resource.
* Queries the SPARQL-endpoint at lov.okfn.org and returns the IRIs in the transitive closure of rdfs:seeAlso and owl:sameAs.
*
* iri: The IRI of the class/resource for which equivalent classes/resources should be determined.
* out: The equivalent IRIs will be written at the first addresses pointed to by this array.
* entryCount: Maximum number of entries in out.
* entryLen: Maximum size of each entry pointed to by out.
*
* Returns the number of results written to out on success. Returns negative on error.
*/
unsigned short queryEquivalentClasses(const char *iri, char **out, unsigned char entryCount, unsigned entryLen);

}

#endif
