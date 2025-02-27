#include "types.h"

using namespace std;
using namespace parcel::type;

static map<lex_type, const char *> names{
    // all reserved keyword blocks
    {GO, "go"},
    {BL_TAGVAL, "tagval"},
    {BL_TAGVAL_TAG, "tag"},
    {BL_TAGVAL_VAL, "val"},
    {BL_WORD, "word"},
    {BL_NUMBER, "num"},
    {BL_LIST, "list"},
    {BL_SEQ, "seq"},
    {BL_VEC, "vector"},
    {BL_SET, "set"},

    // literals
    {LITR_CHAR, "litr.char"},
    {LITR_STR, "litr.str"},
    {LITR_INT, "litr.int"},
    {LITR_FLOAT, "litr.float"},

    // hooks
    {HOOK_DEF, "hook.def"},
    {HOOK_REF, "hook.ref"},

    // links
    {LINK_DEF, "link.def"},
    {LINK_REF, "link.ref"},

    // fns
    {FN_ARG_LIST, "fn.arg_list"},
    {FN_REF, "fn.ref"},
    {FN_REF_EXPR, "fn.ref.expr"}};

const char *parcel::type::nameof(lex_type type)
{
    auto fn = names.find(type);
    if (fn == end(names))
    {
        return "<null>";
    }

    return (*fn).second;
};

lex_type parcel::type::typeof(string & type_name)
{
    for (auto it = begin(names); it != names.end(); it++)
    {
        if ((*it).second == type_name)
        {
            return (*it).first;
        }
    }

    return lex_type::_TYPE_ERR;
};