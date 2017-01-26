#ifndef __WOT_SPARQL__
#define __WOT_SPARQL__

#define DEFAULT_SPARQL_ENDPOINT_HOST "lov.okfn.org"
#define DEFAULT_SPARQL_ENDPOINT_PATH "/dataset/lov/sparql"

namespace wot {

typedef struct {
    char *name;
    char *prefix;
} Prefix;

class PrefixResolver
{
private:
    Prefix prefixes[10];
    unsigned char numPrefixes;
public:
    PrefixResolver();
    ~PrefixResolver();

    /**
    * Adds a prefix definition to this resolver.
    * name: The shorthand for the prefix.
    * prefix: The IRI prefix the shorthand stands for.
    * Returns true if the definition was successfully inserted.
    * False if the maximum count was reached.
    */
    bool addPrefix(const char *name, const char *prefix);

    unsigned resolve(const char *shorthand, char *out, unsigned outLen);
};

/**
* Retrieves equivalent classes and/or resources for a given class or resource.
* Queries the SPARQL-endpoint specified and returns the IRIs in the transitive closure of rdfs:seeAlso and owl:sameAs.
*
* iri: The IRI of the class/resource for which equivalent classes/resources should be determined.
* out: The equivalent IRIs will be written at the first addresses pointed to by this array.
* entryCount: Maximum number of entries in out.
* entryLen: Maximum size of each entry pointed to by out.
*
* Returns the number of results written to out on success. Returns negative on error.
*/
unsigned short queryEquivalentClasses(const char *iri, char **out,
                                unsigned char entryCount, unsigned entryLen,
                                const char *endpointHost = DEFAULT_SPARQL_ENDPOINT_HOST,
                                const char *endpointPath = DEFAULT_SPARQL_ENDPOINT_PATH);

bool classesEquivalent(const char *iri1, const char *iri2);

}

#endif
