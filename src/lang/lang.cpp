#include "lang.h"

bool parcel::lang::is_basic_tag(string &str)
{
    lex_type tp = typeof(str);
    switch (tp)
    {
    case BL_NUMBER:
    case BL_WORD:
    case BL_TAGVAL:
    case BL_TAGVAL_TAG:
    case BL_TAGVAL_VAL:
    case GO:
        return true;

    default:
        return false;
    }
};

bool parcel::lang::is_basetype(string &str)
{
    lex_type tp = typeof(str);
    switch (tp)
    {
    case BL_WORD:
    case BL_NUMBER:
        return true;

    default:
        return false;
    }
};
