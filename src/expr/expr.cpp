#include "expr.h"

// skip values like 'gt(500, 'a', "word")'
bool skip_fn_proto(lexer &lx)
{
    string w;
    if (lx.next_id(w) != lx.npos)
    {
        if (lx.next_like(w, "(", ")", "") != lx.npos)
        {
            return true;
        }
    }

    return false;
};

size_t parcel::expr::get_logic_entry(lexer &lx)
{
    stack<char> brack_seq;
    short deep = 0;
    char last_op = 0;
    int op_entry = 0;
    bool left_right_set = false;
    // '(gt(500) & less(300))'

    string _left;
    string _right;

    char cur_sm = '~';
    // lx.get_info(cout);

    while (lx.can_read())
    {
        lx.skip(" \t");
        // lx.get_info(cout);
        if (!lx.next_symbol(cur_sm))
        {
            // printf("_____!lx.next_symbol(cur_sm)\n");
            return 0;
        }

        if (cur_sm == LANG_EXPR_BR_OPEN)
        {
            brack_seq.push(cur_sm);
            deep++;
        }
        else if (cur_sm == LANG_EXPR_BR_CLOSE)
        {
            if (brack_seq.size() > 0 && brack_seq.top() == LANG_EXPR_BR_OPEN)
            {
                brack_seq.pop();
                deep--;
            }
            else
            {
                // printf("_____brack_seq.size() > 0 && brack_seq.top()\n");
                return 0;
            }
        }
        else if (cur_sm == LANG_LOGIC_AND || cur_sm == LANG_LOGIC_OR)
        {
            if (deep <= 1)
            {
                last_op = cur_sm;
                op_entry = lx.cursor_get() - 1;
            }
        }
        else
        {
            lx.cursor_move(-1);
            if (skip_fn_proto(lx))
            {
            }
        }
    }

    return op_entry;
};

bool parcel::expr::to_postfix(lexer &lx, stack<string> &call_stack)
{
    size_t entry = get_logic_entry(lx);
    string left;
    string right;

    if (entry > 0)
    {
        // DEBUG_MSG("[deep_expr_postfix] entry got ok");
        // printf("ENTRY=%i\n", entry);
        lx.str_left(entry, 1, left);
        if (left == "")
        {
            // printf("____LEFT_FAIL\n");
            return false;
        }
        lx.str_right(entry, 1, right);
        if (right == "")
        {
            // printf("____right_FAIL\n");
            return false;
        }
        // printf("____LEFT='%s' RIGHT='%s'\n", left.c_str(), right.c_str());
    }

    if (entry > 0)
    {
        // parse left
        lexer lx_left(left);
        to_postfix(lx_left, call_stack);

        //((gt(500) & less(300)) | diff(300))
        // parse right
        lexer lx_right(right);
        to_postfix(lx_right, call_stack);
    }

    if (entry == 0)
    {
        string v;
        lx.str(v);
        call_stack.push(v);
        return true;
    }
    else
    {
        char op = lx.at(entry);
        // printf("_op=%c\n", op);
        string s(1, op);
        call_stack.push(s);
        return true;
    }
}
