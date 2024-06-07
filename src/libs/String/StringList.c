#include "StringList.h"

/*  struct bstrList * bsplits (const_bstring str, bstring splitStr)
 *
 *  Create an array of sequential substrings from str divided by any of the
 *  characters in splitStr.  An empty splitStr causes a single entry bstrList
 *  containing a copy of str to be returned.
 */
struct StringList* Splits(ConstString str, ConstString splitStr) {
    struct genBstrList g;

    if (str == NULL || str->slen < 0 || str->data == NULL ||
        splitStr == NULL || splitStr->slen < 0 || splitStr->data == NULL)
        return NULL;

    g.bl = (struct StringList*)StrAlloc(sizeof(struct StringList));
    if (g.bl == NULL) return NULL;
    g.bl->mlen = 4;
    g.bl->entry = (String*)StrAlloc(g.bl->mlen * sizeof(String));
    if (NULL == g.bl->entry) {
        StrFree(g.bl);
        return NULL;
    }
    g.b = (String)str;
    g.bl->qty = 0;

    if (SplitsCb(str, splitStr, 0, bscb, &g) < 0) {
        StringListDestroy(g.bl);
        return NULL;
    }
    return g.bl;
}