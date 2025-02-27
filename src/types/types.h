
#ifndef _TYPES_
#define _TYPES_

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <optional>
#include <string>
#include <map>
using namespace std;

namespace parcel
{
    namespace type
    {
        enum lex_type
        {
            GO,
            // base blocks
            BL_TAGVAL,
            BL_TAGVAL_VAL,
            BL_TAGVAL_TAG,
            BL_WORD,
            BL_NUMBER,
            // literals...
            LITR_CHAR,
            LITR_STR,
            LITR_FLOAT,
            LITR_INT,
            // data hooks
            HOOK_DEF,
            HOOK_REF,
            // links
            LINK_DEF,
            LINK_REF,
            // funcs
            FN_REF,
            FN_ARG_LIST,
            FN_REF_EXPR,
            // lists
            BL_SET,
            BL_LIST,
            BL_SEQ,
            BL_VEC,

            _TYPE_ERR
        };

        extern const char *nameof(lex_type type);
        extern lex_type typeof(string & type_name);

    }
}

#endif