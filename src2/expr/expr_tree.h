#ifndef _EXPR_TREE_
#define _EXPR_TREE_

#include <iostream>
#include <string>
#include "../functions/funcs.h"

using namespace std;

template <typename FnPrototype>
struct fn_btree
{
    bool and_or0;
    fn_btree *left;
    fn_btree *right;
    FnPrototype *value;

    fn_btree(bool and_or0, fn_btree *left, fn_btree *right) : and_or0(and_or0), left(left), right(right), value(NULL) {};
    fn_btree(FnPrototype *single_fn) : value(single_fn), left(NULL), right(NULL), and_or0(0) {};
    fn_btree() : left(NULL), right(NULL), value(NULL) {};
    ~fn_btree()
    {
        if (value != NULL)
        {
            delete value;
        }
        if (left != NULL)
        {
            delete left;
        }
        if (right != NULL)
        {
            delete right;
        }
    }
};

#endif