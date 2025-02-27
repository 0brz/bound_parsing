#ifndef _LANG_
#define _LANG_

#include "../types/types.h"

#define LANG_EXPR_BR_OPEN '('
#define LANG_EXPR_BR_CLOSE ')'
#define LANG_LOGIC_AND '&'
#define LANG_LOGIC_OR '|'

#define LEX_SYMBOLS_NO_EXPR ":@!*^%$<>{}:\"'~;"
#define LEX_SYMBOLS_SPACES " \r\t\n"

#define LANG_PREFIX '&'
#define LANG_TAG_PREFIX ':'
#define LANG_HOOK_PREFIX '$'
#define LANG_LINK_PREFIX '@'

using namespace parcel::type;

/*
    lang based logic.
*/

namespace parcel
{
    namespace lang
    {
        bool is_basic_tag(string &str);
        bool is_basetype(string &str);
    }
}

#endif