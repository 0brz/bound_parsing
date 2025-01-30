#include <string>
#include "parcel.h"
#include "builder.h"
#include <stack>

using namespace std;
using namespace lex;

#define LANG_EXPR_BR_OPEN '('
#define LANG_EXPR_BR_CLOSE ')'
#define LANG_LOGIC_AND '&'
#define LANG_LOGIC_OR '|'

#pragma region expressions

#include "expr/expr_tree.h"

// bit_expr_list

#pragma endregion

// -------------------------
/*
graph_block* create_function(string& name, string& args) {
    graph_block* b = new graph_block();
    value_function* v = new value_function(name, args);
    b->type = RULE_TYPE::FUNCTION;
    b->value = v;
    return b;
};
*/

graph_block *create_hook(string &name)
{
    graph_block *b = new graph_block();
    value_datahook *v = new value_datahook(name);
    b->type = RULE_TYPE::DATA_HOOK;
    b->value = v;
    return b;
};

graph_block *create_literal(RULE_TYPE type, graph_value *val)
{
    graph_block *b = new graph_block();
    b->type = type;
    b->value = val;
    return b;
};

graph_block *create_block(RULE_TYPE type, graph_value *val)
{
    graph_block *bl = new graph_block();
    bl->type = type;
    bl->value = val;
    return bl;
}

// -----------------------

/*
bool builder::is_function(lexer& lx) {
    printf("______________\n");
    lx.get_info(cout);
    size_t move_back_size = 0;
            string id;
            size_t ofs = 0;
            if ((ofs = lx.next_id(id)) != lx.npos) {
                //printf("___is_fun: id=%s\n", id.c_str());
                move_back_size+=ofs;
                lx.get_info(cout);

                if (lx.check_sequence("()", ":")) {
                    //printf("___is_fun: sec=\n");
                    lx.cursor_move(-move_back_size);

                    //lx.get_info(cout);
                    //printf("______________\n");
                    return true;
                }
            }

    lx.cursor_move(-move_back_size);
    return false;
} ;
*/

bool is_function_def(lexer &lx)
{
    string id;
    size_t id_ofs;

    size_t old_ofs = lx.cursor_get();

    if ((id_ofs = lx.next_id(id)) != lx.npos)
    {
        if (lx.check_sequence("()", ":-"))
        {
            string args;
            size_t ofs_args;
            if ((ofs_args = lx.next_like_rounded(args, "(", ")", ":->^?{}[]")) != lx.npos)
            {
                // :->
                lx.skip(" \t");
                if (lx.step_next(":"))
                {
                    lx.skip(" \n\t");
                    if (lx.step_next("->"))
                    {
                        lx.cursor_set(old_ofs);
                        return true;
                    }
                }
            }
        }
    }

    lx.cursor_set(old_ofs);
    return false;
};

bool is_function_ref(lexer &lx)
{
    string id;
    size_t id_ofs;

    size_t old_ofs = lx.cursor_get();

    if ((id_ofs = lx.next_id(id)) != lx.npos)
    {
        if (lx.check_sequence("()", ":-"))
        {
            string args;
            size_t ofs_args;
            if ((ofs_args = lx.next_like_rounded(args, "(", ")", ":->^?{}[]")) != lx.npos)
            {
                // :->
                lx.skip(" \t");
                if (!lx.step_next(":"))
                {
                    lx.cursor_set(old_ofs);
                    return true;
                }
            }
        }
    }

    lx.cursor_set(old_ofs);
    return false;
};

bool builder::is_hook(lexer &lx)
{
    size_t move_back_size = 0;
    char single_pref = '_';
    string id;
    if (lx.next_id(id) != lx.npos)
    {
        move_back_size += id.size();
        if (lx.next_symbol(single_pref))
        {
            move_back_size++;
            if (single_pref == LANG_TAG_PREFIX)
            {
                lx.cursor_move(-move_back_size);
                return true;
            }
        }
    }

    lx.cursor_move(-move_back_size);
    return false;
};

bool is_hook_ref(lexer &lx)
{
    size_t move_back_size = 0;
    char single_pref = '_';
    string id;
    if (lx.next_id(id) != lx.npos)
    {
        move_back_size += id.size();
        lx.cursor_move(-move_back_size);
        return true;
    }

    lx.cursor_move(-move_back_size);
    return false;
};

bool builder::is_literal(lexer &lx)
{
    // 123.123.123 (ip...)
    // 123.123 (float)
    // 123 (int)
    // "word" -> litr_word
    // 'a' -> litr_symbol
    // "a" -> litr_word

    // printf("______________\n");
    // lx.get_info(cout);

    char single_pref = '_';
    string cur;
    if (lx.next_float(cur) != lx.npos)
    { // 123.123
        lx.cursor_move(-cur.size());
        return true;
    }
    else if (lx.next_int(cur) != lx.npos)
    { // 123
        lx.cursor_move(-cur.size());
        return true;
    }
    else if (lx.next_like_rounded(cur, "\"", "\"", "") != lx.npos)
    { // "app"
        lx.cursor_move(-cur.size());
        return true;
    }
    else if (lx.next_like_rounded(cur, "'", "'", "") != lx.npos)
    { // 'g'
        lx.cursor_move(-cur.size());
        if (cur.size() == 3)
        {
            return true;
        }
    }

    // lx.get_info(cout);
    // printf("______________\n");
    return false;
};

// ----------------------

/*
graph_block* builder::build_function(lexer& lx, bool is_ref) {
    // get prefix
    // get name
    // get args
    lx.get_info(cout);
    string _name;
    if (lx.next_id(_name) == lx.npos) {
        printf("[ERR] build.function: name\n");
        lx.get_info(cout);
        return NULL;
    }

    string _args;
    if (lx.next_like_rounded(_args, "(", ")", lx.symbols_no_id_nospace) ==
        lx.npos) {
        printf("[ERR] build.function: args\n");
        lx.get_info(cout);
        return NULL;
    }

    graph_block* bl = create_function(_name, _args);
    if (is_ref) {
        bl->type = RULE_TYPE::FUNCTION_REF;
    }

    printf("[build] fn: %s(%s)\n", _name.c_str(), _args.c_str());
    return bl;
};
*/

graph_block *build_function_def(lexer &lx)
{
    // name, args, ret value
    // read until the next tag
    return NULL;
}

graph_block *builder::build_hook(lexer &lx, bool is_ref)
{
    // get name
    // get args
    // lx.get_info(cout);
    string _name;
    if (lx.next_id(_name) == lx.npos)
    {
        printf("[ERR] build.hook: name\n");
        // lx.get_info(cout);
        return NULL;
    }

    graph_block *b = create_hook(_name);
    if (is_ref)
    {
        b->type == RULE_TYPE::DATA_HOOK_REF;
    }
    return b;
}

bool step_char_if_eq(lexer &lx, char check_symbol)
{
    lx.skip(" ");
    if (lx.can_read())
    {
        char t;
        lx.next_symbol(t);
        if (t == check_symbol)
        {
            return true;
        }
        else
        {
            lx.cursor_move(-1);
        }
    }

    return false;
};

// ----------------------
bool _link_last_block(graph_table<graph_block *> *gt, graph_block *bl)
{
    graph_block *last;
    if (!gt->last(last))
    {
        printf("[_link_last_block] ret null\n");
        return false;
    }

    last->entries.push_back(bl);
}

// ---------------------
/*
    is_vardef()
    is_func_def
    is_func_call

    build_vardef
    build_func_def
    build_func_call
*/

bool is_vardef(lexer &lx)
{
    string w;
    auto old_ofs(lx.cursor_get());
    if (lx.next_word(w) != lx.npos)
    {
        if (w == LANG_VAR)
        {
            lx.cursor_set(old_ofs);
            return true;
        }
    }

    lx.cursor_set(old_ofs);
    return false;
};

graph_block *build_vardef(lexer &lx)
{
    // var: (<base_type>) <varname>
    string cur;
    auto old_ofs(lx.cursor_get());
    if (lx.next_word(cur))
    {
        if (cur == LANG_VAR)
        {
            lx.skip(" \r\t");
            if (lx.step_next(":"))
            {
                // get base type
                lx.skip(" \t");
                if (lx.next_like_rounded(cur, "(", ")", lx.symbols_no_id_nospace) == lx.npos)
                {
                    return false;
                }

                // get name
                string var_id;
                if (lx.next_id(var_id) == lx.npos)
                {
                    return false;
                }

                graph_value *val = new value_vardef(var_id, cur);
                graph_block *bl = create_block(RULE_TYPE::VAR_DEF, val);
                return bl;
            }
        }
    }

    lx.cursor_set(old_ofs);
    return false;
}

// ---------
bool is_func_def(lexer &lx)
{
    // fn: (<base_block_type>) <fn_name> (<args>)
    // arg: <base_type> <arg_name>
    return false;
}

bool is_func_call(lexer &lx)
{
    // <func_name>(<args>)
    // value_fn_arglist
    string fn_id;
    auto old_ofs(lx.cursor_get());

    // get name
    if (lx.next_id(fn_id) != lx.npos)
    {
        if (lx.check_sequence("()", ":^[]{}"))
        {
            lx.cursor_set(old_ofs);
            return true;
        }
    }

    lx.cursor_set(old_ofs);
    return false;
}

value_fn_arglist *build_fn_args(lexer &lx, bool &out_build_status)
{
    //
    char pref = ' ';
    value_fn_arglist *head = NULL;
    value_fn_arglist *args = NULL;
    while (lx.can_read())
    {
        // lx.get_info(cout);
        if (lx.next_symbol(pref))
        {
            if (pref == ')')
                break;
            if (pref == '(')
                lx.cursor_move(1);
            if (pref == ',')
                lx.cursor_move(1);
            if (pref == ' ')
                lx.cursor_move(1);
            else
                lx.cursor_move(-1);
        }

        string cur;
        if (lx.next_float(cur) != lx.npos)
        {
            string _val = string(cur.c_str());
            if (args == NULL)
            {
                args = new value_fn_arglist(RULE_TYPE::LITR_FLOAT, _val);
                head = args;
            }
            else
            {
                args->next_arg = new value_fn_arglist(RULE_TYPE::LITR_FLOAT, _val);
                args = args->next_arg;
            }
        }
        else if (lx.next_int(cur) != lx.npos)
        {
            string _val = string(cur.c_str());
            if (args == NULL)
            {
                args = new value_fn_arglist(RULE_TYPE::LITR_INT, _val);
                head = args;
            }
            else
            {
                args->next_arg = new value_fn_arglist(RULE_TYPE::LITR_INT, _val);
                args = args->next_arg;
            }
        }
        else if (lx.next_like_rounded(cur, "\"", "\"", "") != lx.npos)
        {
            string _val = string(cur.c_str());
            if (args == NULL)
            {
                args = new value_fn_arglist(RULE_TYPE::LITR_STR, _val);
                head = args;
            }
            else
            {
                args->next_arg = new value_fn_arglist(RULE_TYPE::LITR_STR, _val);
                args = args->next_arg;
            }
        }
        else if (lx.next_like_rounded(cur, "'", "'", "") != lx.npos)
        {
            char _valb = cur[1]; // we can take, because size of char view=3, like 'a', 'b'
            string _val = cur.substr(1, 1);
            if (args == NULL)
            {
                args = new value_fn_arglist(RULE_TYPE::LITR_CHAR, _val);
                head = args;
            }
            else
            {
                args->next_arg = new value_fn_arglist(RULE_TYPE::LITR_CHAR, _val);
                args = args->next_arg;
            }
        }
        else
        {

            // clean
            printf("~() [build_fn_args]\n");
            value_fn_arglist *pt = head;
            while (pt != NULL)
            {
                delete pt;
                pt = pt->next_arg;
            };

            out_build_status = false;
            return NULL;
        }
    }

    out_build_status = true;
    return args;
}

graph_block *build_func_call(lexer &lx)
 {
    string fn_id;

    // get name
    if (lx.next_id(fn_id) != lx.npos)
    {
        printf("[build_func_call] id=%s\n", fn_id.c_str());

        lx.skip(" \t");
        bool args_build = false;
        value_fn_arglist *args = build_fn_args(lx, args_build);
        if (!args_build)
        {
            return NULL;
        }

        value_fn_ref *_val = new value_fn_ref(fn_id, args);
        graph_block *bl = create_block(RULE_TYPE::FN_REF, _val);

        return bl;
    }
    else
        return NULL;
};

graph_block *build_func_call(lexer &lx, string& fn_name)
 {
    string fn_id;

    // get name
    if (lx.next_id(fn_id) != lx.npos)
    {
        printf("[build_func_call] id=%s\n", fn_id.c_str());
        fn_name = fn_id;
        lx.skip(" \t");
        bool args_build = false;
        value_fn_arglist *args = build_fn_args(lx, args_build);
        if (!args_build)
        {
            return NULL;
        }

        value_fn_ref *_val = new value_fn_ref(fn_id, args);
        graph_block *bl = create_block(RULE_TYPE::FN_REF, _val);

        return bl;
    }
    else
        return NULL;
};

// -------------- EXPR

// acess_fn_args
// acess_fn_call
// access_fn_expr

// get_logic_entry

/*

    build_fn_expr_stack(){
    
        logic_entry  = get_logic_entry(src);
        left = src.substr()
        right = src.substr()
        
        // check left
        if (is_fn_expr(left))
            build_fn_expr(left, call_stack)
        else (is_fn_call(left))
            call_stack += build_fn_call(left)
        
        // check right


    }

*/

size_t get_expr_logic_entry(lexer& lx) {
    stack<char> brack_seq;
    short deep = 0;
    char last_op = 0;
    int op_entry = 0;
    bool left_right_set = false;
    // '(gt(500) & less(300))'

    string _left;
    string _right;

    char cur_sm = '~';
    //printf("[get_expr_logic_entry]\n");

    while (lx.can_read())
    {
        lx.skip(" \t");
        //lx.get_info(cout);
        if (!lx.next_symbol(cur_sm))
        {
            // err
            return 0;
            break;
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
                // err
                return 0;
                break;
            }
        }
        else if (cur_sm == LANG_LOGIC_AND || cur_sm == LANG_LOGIC_OR) {
            if (deep <= 1) {
                last_op = cur_sm;
                op_entry = lx.cursor_get()-1;
            }
        }
        else {
            //lx.get_info(cout);
            // parsing like 'less(500)', 'btw(500, 1000)'
            //lx.cursor_move(-1);
        }
    }

    return op_entry;
}

void get_expr_deep(lexer& lx, stack<string>& call_stack) {
    size_t entry = get_expr_logic_entry(lx);
    string left;
    string right;

    if (entry > 0) {
        lx.str_left(entry, 1, left);
        lx.str_right(entry, 1, right);
        //printf("____LEFT='%s' RIGHT='%s'\n", left.c_str(), right.c_str());
    }

    if (entry > 0) {
        // parse left
        lexer lx_left(left);
        get_expr_deep(lx_left, call_stack);

    //((gt(500) & less(300)) | diff(300))
        // parse right
        lexer lx_right(right);
        get_expr_deep(lx_right, call_stack);
    }

    if (entry == 0) {
        lx.get_info(cout);
        string v;
        lx.str(v);
        call_stack.push(v);
    }
    else {
        char op = lx.at(entry);
        //printf("_op=%c\n", op);
        string s(1, op);
        call_stack.push(s);
    }
}


string _simple_remove_spaces(string& src) {
    stringstream ss;
    for (char t : src) {
        if (t == ' ') continue;
        else ss << t;
    }

    return ss.str();
}


void try_build_fn_tree(stack<string>& postfix, fn_btree_refs* tree) {
    auto entry = postfix.top();
    postfix.pop();

    if (entry == "|" || entry == "&") {
        if (entry == "|") tree->and_or0 = 0;
        else tree->and_or0 = 1;
        
        // left
        auto _left = postfix.top();
        postfix.pop();

        if (_left == "|" || _left == "&") {
            tree->left = new fn_btree_refs(); 

            //tree->left = new expr_tree();
            postfix.push(_left);
            try_build_fn_tree(postfix, tree->left);
        }
        else {
            // this is a value
            tree->left = new fn_btree_refs();
            //tree->left->value = build_func_call()
            printf("build(left)=%s\n", _left.c_str());
            // old
            //tree->left = new expr_tree();
            //tree->left->val = _left;
        }

        // right
        auto _right = postfix.top();
        postfix.pop();

        if (_right == "|" || _right == "&") {
            tree->right = new fn_btree_refs();
            postfix.push(_right);
            try_build_fn_tree(postfix, tree->right);
        }
        else {
            tree->right = new fn_btree_refs();
            printf("build(right)=%s\n", _right.c_str());
            //tree->right->val = _right;
        }
    }

}

bool try_build_fn_expr(lexer &lx)
{
    // prepare string
    string expr_s;
    auto c = lx.cursor_get();
    lx.next_until("\n\r", expr_s);
    lx.cursor_set(c);

    expr_s.insert(0, "(");
    expr_s.append(")");

    string ps = _simple_remove_spaces(expr_s);

    vector<graph_block *> fns;

    lexer lx2(ps);

    stack<string> cs;
    get_expr_deep(lx2, cs);

    if (cs.size() <= 1) {
        lx.cursor_set(c);
        return false;
    }

    fn_btree_refs* tr = new fn_btree_refs();
    try_build_fn_tree(cs, tr);

    value_fn_expr_refs* expr_val = new value_fn_expr_refs(tr);
    graph_block* fn_expr = create_block(RULE_TYPE::FN_REF_EXPR, expr_val);

    return true;
}

graph_table<graph_block *> *builder::build_lex_graph(string &src)
{
    graph_table<graph_block *> *gt = new graph_table<graph_block *>();

    lexer lx(src);
    string cur;
    size_t line_offset = 0;
    size_t literals_offset = -1;
    char single_prefix = '_';

    while (lx.can_read())
    {

        if (step_char_if_eq(lx, LANG_PREFIX))
        {
            // printf("[build].is_def\n");
            if (is_vardef(lx))
            {
                // no linking
                auto bl = build_vardef(lx);
                gt->add(bl, line_offset);
                printf("~%zi [gt(nolink))].vardef\n", line_offset);
            }
            else if (is_func_call(lx))
            {
                auto bl = build_func_call(lx);
                if (bl == NULL)
                {
                    // exp
                    printf("[ERR] build: [build_func_call]\n");
                    break;
                }

                printf("~%zi [gt(link.last)].fn.call\n", line_offset);
                _link_last_block(gt, bl);
            }
            else if (builder::is_hook(lx))
            {
                // hook def, no linking
                auto _bl = build_hook(lx, false);
                gt->add(_bl, line_offset);
                printf("~%zi [gt(nolink)].hook\n", line_offset);
                // lx.get_info(cout);
            }
            else if (is_hook_ref(lx))
            {
                // hook ref, link
                auto _bl = build_hook(lx, true);
                gt->add(_bl, line_offset);
                _link_last_block(gt, _bl);
                printf("~%zi [gt(link.last)] hook(ref)\n", line_offset);
            }
        }

        if (is_literal(lx))
        {

            graph_block *_last;
            string _last_name;
            if (gt->last(_last))
                _last_name = lex::nameof(_last->type);
            else
                _last_name = "<undf>";

            // printf("[build] is_lit\n");

            if (lx.next_float(cur) != lx.npos)
            {
                int _val = stof(cur.c_str());
                auto _bl = create_literal(RULE_TYPE::LITR_FLOAT, new value_litr_float(_val));
                _link_last_block(gt, _bl);
                // gt->add(_bl, literals_offset);
                printf("~%zi [gt(link.last)] (float) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }
            else if (lx.next_int(cur) != lx.npos)
            {
                int _val = stoi(cur.c_str());
                auto _bl = create_literal(RULE_TYPE::LITR_INT, new value_litr_int(_val));
                _link_last_block(gt, _bl);
                // gt->add(_bl, literals_offset);
                printf("~%zi [gt(link.last)] (int) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }
            else if (lx.next_like_rounded(cur, "\"", "\"", "") != lx.npos)
            {
                string _val = string(cur.c_str());
                auto _bl = create_literal(RULE_TYPE::LITR_STR, new value_litr_string(_val));
                _link_last_block(gt, _bl);
                // gt->add(_bl, literals_offset);
                printf("~%zi [gt(link.last)] (str) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }
            else if (lx.next_like_rounded(cur, "'", "'", "") != lx.npos)
            {
                // printf("~%zi [build] lit.char=%s\n", line_offset, cur.c_str());
                //  create_char
                char _val = cur[1]; // we can take, because size of char view=3, like 'a', 'b'
                auto _bl = create_literal(RULE_TYPE::LITR_CHAR, new value_litr_char(_val));
                //_last->entries.push_back(_bl);
                _link_last_block(gt, _bl);
                // gt->add(_bl, literals_offset);
                //  _link_last_block
                printf("~%zi [gt(link.last)] (char) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }
        }

        // lx.get_info(cout);
        if (lx.next_word(cur) != lx.npos)
        {
            // tag
            RULE_TYPE _type = lex::typeof(cur);
            graph_block *_last;
            string _last_name;
            if (gt->last(_last))
                _last_name = lex::nameof(_last->type);
            else
                _last_name = "<undf>";

            if (lex::is_valuetag(_type))
            {
                // printf("~%zi [gt(basetag)] %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
                //  is_expr()
                //  is_literal
                //  else -> default value
                if (step_char_if_eq(lx, LANG_TAG_PREFIX))
                {
                    // printf("~%zi [gt(basetag, tag)] %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());

                    if (is_literal(lx))
                    {
                        printf("~%zi [gt(basetag, tag, litr)] %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
                    }
                    else if (try_build_fn_expr(lx))
                    {
                        printf("~%zi [gt(basetag, tag, fn_expr)] %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
                        break;
                    }
                    else if (is_func_call(lx)) {
                        auto fn_single = build_func_call(lx);
                        if (fn_single == NULL) {
                            // ERR
                            continue;
                        }

                        printf("~%zi [gt(basetag, tag, fn_single)] %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
                        
                        break;
                    }

                    // is_fn_expr
                    // is_litr
                }

                continue;
            }
            else if (step_char_if_eq(lx, LANG_TAG_PREFIX))
            {

                if (!lex::is_tagword(cur))
                {
                    printf("~%zi [ERR] build: (type error) tag='%s'\n", line_offset, cur.c_str());
                    break;
                }

                RULE_TYPE _type = lex::typeof(cur);
                if (_type == RULE_TYPE::GO)
                {
                    // this is a entrypoint, no parent relates
                    graph_block *_bl = create_block(RULE_TYPE::GO, NULL);
                    gt->add(_bl, line_offset);
                    printf("~%zi [gt] [tag].go\n", line_offset);
                }
                else
                {
                    // gt.link_prev(bl)

                    // has_value

                    graph_block *_bl = create_block(_type, NULL);
                    gt->add(_bl, line_offset);
                    printf("~%zi [gt(link.last)] %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
                }

                continue;
            }

            printf("__CUR=%s\n", cur.c_str());
        }

        if (step_char_if_eq(lx, '\n'))
        {
            line_offset = lx.skip(" \t");
        }
        else
        {
            lx.cursor_move(1);
        }
    }

    return gt;
};
