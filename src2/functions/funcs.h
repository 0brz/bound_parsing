#ifndef _FUNCS_
#define _FUNCS_

#include "../parcel.h"

enum fn_type
{
    gt,
    egt,
    less,
    eless,
};

struct fn_value_basic
{
};

struct fn_proto
{
    fn_value_basic *value;
    fn_type type;

    fn_proto(fn_type type, fn_value_basic *val) : type(type), value(val) {};
};

// ---------------------
//      NUMBERS

struct fn_gt : public fn_value_basic
{
    float val;
    fn_gt(float val) : val(val) {};
};

struct fn_less : public fn_value_basic
{
    float val;
    fn_less(float val) : val(val) {};
};

#endif
