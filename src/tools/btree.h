#ifndef _BTREE_
#define _BTREE_

template <typename Value>
struct btree
{
    bool and_or0;
    btree *left;
    btree *right;
    Value value;

    btree(bool and_or0, btree *left, btree *right) : and_or0(and_or0), left(left), right(right), value(NULL) {};
    btree(Value &val) : value(val), left(NULL), right(NULL), and_or0(0) {};
    btree() : left(NULL), right(NULL), value(NULL) {};
    ~btree()
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