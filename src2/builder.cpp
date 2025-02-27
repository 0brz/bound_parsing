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

#define LEX_SYMBOLS_NO_EXPR ":@!*^%$<>{}:\"'~;"
#define LEX_SYMBOLS_SPACES " \r\t\n"

#pragma region expressions

#include "expr/expr_tree.h"

// bit_expr_list

#pragma endregion

bool _is_space(char t)
{
    string trash = "\r\t \n";
    return trash.find(t) != string::npos;
};

bool _is_special_delim(char t)
{
    string trash = ":&";
    return trash.find(t) != string::npos;
};

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

    return false;
};

// ----------------------

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
    string _name;
    if (lx.next_id(_name) == lx.npos)
    {
        printf("[ERR] build.hook: name\n");
        return NULL;
    }

    graph_block *b = create_hook(_name);
    if (is_ref)
    {
        b->type == RULE_TYPE::DATA_HOOK_REF;
    }
    return b;
}

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

bool is_fn_call(lexer &lx)
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

value_fn_ref *try_build_fn_ref(lexer &lx)
{
    string fn_id;
    auto old = lx.cursor_get();

    // get name
    if (lx.next_id(fn_id) != lx.npos)
    {
        // printf("[try_build_fn_call] id=%s\n", fn_id.c_str());

        lx.skip(" \t");
        bool args_build = false;
        value_fn_arglist *args = build_fn_args(lx, args_build);
        if (!args_build)
        {
            lx.cursor_set(old);
            return NULL;
        }

        value_fn_ref *_val = new value_fn_ref(fn_id, args);
        return _val;
    }

    lx.cursor_set(old);
    return NULL;
};

/*
graph_block *try_build_fn_call(lexer &lx, string &fn_name)
{
    string fn_id;
    auto old = lx.cursor_get();

    // get name
    if (lx.next_id(fn_id) != lx.npos)
    {
        printf("[try_build_fn_call] id=%s\n", fn_id.c_str());
        fn_name = fn_id;
        lx.skip(" \t");
        bool args_build = false;
        value_fn_arglist *args = build_fn_args(lx, args_build);
        if (!args_build)
        {
            lx.cursor_set(old);
            return NULL;
        }

        value_fn_ref *_val = new value_fn_ref(fn_id, args);
        graph_block *bl = create_block(RULE_TYPE::FN_REF, _val);

        return bl;
    }

    lx.cursor_set(old);
    return NULL;
};
*/

// -------------- EXPR

// acess_fn_args
// acess_fn_call
// access_fn_expr

// get_logic_entry

size_t get_expr_logic_entry(lexer &lx)
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
    // printf("[get_expr_logic_entry]\n");

    while (lx.can_read())
    {
        lx.skip(" \t");
        // lx.get_info(cout);
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
            // lx.get_info(cout);
            //  parsing like 'less(500)', 'btw(500, 1000)'
            // lx.cursor_move(-1);
        }
    }

    return op_entry;
}

bool deep_expr_postfix(lexer &lx, stack<string> &call_stack)
{
    size_t entry = get_expr_logic_entry(lx);
    string left;
    string right;

    if (entry > 0)
    {
        // DEBUG_MSG("[deep_expr_postfix] entry got ok");
        lx.str_left(entry, 1, left);
        if (left == "")
            return false;
        lx.str_right(entry, 1, right);
        if (right == "")
            return false;
        // printf("____LEFT='%s' RIGHT='%s'\n", left.c_str(), right.c_str());
    }

    if (entry > 0)
    {
        // parse left
        lexer lx_left(left);
        deep_expr_postfix(lx_left, call_stack);

        //((gt(500) & less(300)) | diff(300))
        // parse right
        lexer lx_right(right);
        deep_expr_postfix(lx_right, call_stack);
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

string _clear_expr_string(string &src)
{
    stringstream ss;
    for (char t : src)
    {
        if (_is_space(t) || t == '\n')
            continue;
        else
            ss << t;
    }

    return ss.str();
};

bool check_s(string &s, const char *contains)
{
    if (s.find_first_of(contains) != string::npos)
    {
        return true;
    }

    return false;
}

bool try_build_fn_tree(stack<string> &postfix, fn_btree_refs *tree)
{
    if (postfix.empty())
    {
        DEBUG_MSG("[try_build_fn_tree] postfix is empty, try mm");
        return false;
    }

    auto entry = postfix.top();
    postfix.pop();

    if (entry == "|" || entry == "&")
    {
        if (entry == "|")
            tree->and_or0 = 0;
        else
            tree->and_or0 = 1;

        // left
        auto _left = postfix.top();
        postfix.pop();

        if (_left == "|" || _left == "&")
        {
            tree->left = new fn_btree_refs();

            postfix.push(_left);
            if (!try_build_fn_tree(postfix, tree->left))
                return false;
        }
        else
        {
            value_fn_ref *fn_ref = NULL;
            lexer fn_lx(_left);
            if ((fn_ref = try_build_fn_ref(fn_lx)) == NULL)
                return false;

            tree->left = new fn_btree_refs();
            tree->left->value = fn_ref;
        }

        // right
        auto _right = postfix.top();
        postfix.pop();

        if (_right == "|" || _right == "&")
        {
            tree->right = new fn_btree_refs();
            postfix.push(_right);
            if (!try_build_fn_tree(postfix, tree->right))
                return false;
        }
        else
        {
            value_fn_ref *fn_ref = NULL;
            lexer fn_lx(_right);
            if ((fn_ref = try_build_fn_ref(fn_lx)) == NULL)
                return false;

            tree->right = new fn_btree_refs();
            tree->right->value = fn_ref;
        }
    }

    return true;
}

value_fn_expr_refs *try_build_fn_expr(lexer &lx)
{
    // prepare string
    string expr_s;
    auto old = lx.cursor_get();
    lx.next_until("\n\r", expr_s);
    lx.cursor_set(old);
    auto next_cursor = old += expr_s.size();

    if (check_s(expr_s, LEX_SYMBOLS_NO_EXPR))
        return NULL;

    expr_s.insert(0, "(");
    expr_s.append(")");

    string normalized_expr_buff = _clear_expr_string(expr_s);
    lexer lx2(normalized_expr_buff);

    stack<string> expr_postfix;
    if (!deep_expr_postfix(lx2, expr_postfix))
    {
        return NULL;
    }

    // DEBUG_MSG("[try_build_fn_expr] deep_expr_postfix ok");

    if (expr_postfix.size() <= 1)
    {
        lx.cursor_set(old);
        return NULL;
    }

    fn_btree_refs *tr = new fn_btree_refs();
    if (!try_build_fn_tree(expr_postfix, tr))
    {
        DEBUG_MSG("[try_build_fn_expr] Cant build fn expr tree");
        return NULL;
    }

    lx.cursor_set(next_cursor);

    value_fn_expr_refs *expr_val = new value_fn_expr_refs(tr);

    return expr_val;
}

graph_block *try_build_tagword(lexer &lx)
{
    string tagname;
    auto c = lx.cursor_get();

    if (lx.next_word(tagname) != lx.npos)
    {
        lx.skip(" \t");
        if (lx.at(lx.cursor_get()) == LANG_TAG_PREFIX)
        {
            lx.cursor_move(1);

            RULE_TYPE type = lex::typeof(tagname);
            graph_block *_bl = create_block(type, NULL); // tagword doesnt have a value
            return _bl;
        }
    }

    lx.cursor_set(c);
    return NULL;
};

graph_block *try_build_basetype(lexer &lx)
{
    string type;
    auto old = lx.cursor_get();
    if (lx.next_id(type) != lx.npos)
    {
        auto s = lx.skip(" \r\t\n");
        if (isalpha(lx.at(lx.cursor_get())) || !lx.has_next_not_of(lx.cursor_get(), LEX_SYMBOLS_SPACES))
        {
            auto tp = lex::typeof(type);
            if (lex::is_basevalue(tp))
            {
                graph_block *bl = create_block(tp, NULL);
                return bl;
            }
        }
    }

    lx.cursor_set(old);
    return NULL;
}

graph_block *try_build_hook(lexer &lx)
{
    auto old = lx.cursor_get();
    if (lx.at(old) == LANG_PREFIX)
    {
        lx.cursor_move(1);
        string v;
        if (lx.next_id(v) != lx.npos)
        {
            lx.skip(" \r\t");
            if (lx.at(lx.cursor_get()) == LANG_TAG_PREFIX)
            {
                graph_block *bl = create_block(RULE_TYPE::DATA_HOOK, NULL);
                return bl;
            }
        }
    }

    lx.cursor_set(old);
    return NULL;
};

graph_block *try_build_vardef(lexer &lx)
{
    auto old = lx.cursor_get();
    if (lx.at(old) == LANG_VARDEF)
    {
        lx.cursor_move(1);
        string v;
        if (lx.next_id(v) != lx.npos)
        {
            lx.skip(" \r\t");
            if (lx.at(lx.cursor_get()) == LANG_TAG_PREFIX)
            {
                graph_block *bl = create_block(RULE_TYPE::VAR_DEF, NULL);
                return bl;
            }
        }
    }

    lx.cursor_set(old);
    return NULL;
};

graph_block *try_build_vardef_ref(lexer &lx)
{
    auto old = lx.cursor_get();
    if (lx.at(old) == LANG_VARDEF)
    {
        lx.cursor_move(1);
        string v;
        if (lx.next_id(v) != lx.npos)
        {
            value_vardef_ref *ref = new value_vardef_ref(v);
            graph_block *bl = create_block(RULE_TYPE::VAR_DEF_REF, ref);
            return bl;
        }
    }

    lx.cursor_set(old);
    return NULL;
};

graph_block *try_build_hook_ref(lexer &lx)
{
    auto old = lx.cursor_get();
    if (lx.at(old) == LANG_PREFIX)
    {
        lx.cursor_move(1);
        string v;
        if (lx.next_id(v) != lx.npos)
        {
            value_vardef_ref *ref = new value_vardef_ref(v);
            graph_block *bl = create_block(RULE_TYPE::DATA_HOOK_REF, ref);
            return bl;
        }
    }

    lx.cursor_set(old);
    return NULL;
};

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
        // DEFINES
        // vardef
        // hook
        graph_block *bl = NULL;
        if ((bl = try_build_hook(lx)) != NULL)
        {
            gt->add(bl, line_offset);
            printf("~%zi [gt(nolink))].hook\n", line_offset);
            continue;
        }
        else if ((bl = try_build_vardef(lx)) != NULL)
        {
            gt->add(bl, line_offset);
            printf("~%zi [gt(nolink))].vardef\n", line_offset);
            continue;
        }

        // LITERALS, VALUES
        if (is_literal(lx))
        {
            graph_block *_last;
            string _last_name;
            if (gt->last(_last))
                _last_name = lex::nameof(_last->type);
            else
                _last_name = "<undf>";

            if (lx.next_float(cur) != lx.npos)
            {
                int _val = stof(cur.c_str());
                auto _bl = create_literal(RULE_TYPE::LITR_FLOAT, new value_litr_float(_val));
                _link_last_block(gt, _bl);
                printf("~%zi [gt(link.last)] (float) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }
            else if (lx.next_int(cur) != lx.npos)
            {
                int _val = stoi(cur.c_str());
                auto _bl = create_literal(RULE_TYPE::LITR_INT, new value_litr_int(_val));
                _link_last_block(gt, _bl);
                printf("~%zi [gt(link.last)] (int) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }
            else if (lx.next_like_rounded(cur, "\"", "\"", "") != lx.npos)
            {
                string _val = string(cur.c_str());
                auto _bl = create_literal(RULE_TYPE::LITR_STR, new value_litr_string(_val));
                _link_last_block(gt, _bl);
                printf("~%zi [gt(link.last)] (str) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }
            else if (lx.next_like_rounded(cur, "'", "'", "") != lx.npos)
            {
                char _val = cur[1]; // we can take, because size of char view=3, like 'a', 'b'
                auto _bl = create_literal(RULE_TYPE::LITR_CHAR, new value_litr_char(_val));
                _link_last_block(gt, _bl);
                printf("~%zi [gt(link.last)] (char) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
            }

            continue;
        }

        // VALUED TAGS
        graph_block *basev = NULL;
        if ((basev = try_build_basetype(lx)) != NULL)
        {
            _link_last_block(gt, basev);
            printf("~%zi [gt(link.last)] (basetype) \n", line_offset);
            continue;
        }
        else if ((basev = try_build_vardef_ref(lx)) != NULL)
        {
            _link_last_block(gt, basev);
            printf("~%zi [gt(link.last)] (vardef_ref) \n", line_offset);
            continue;
        }
        else if ((basev = try_build_hook_ref(lx)) != NULL)
        {
            _link_last_block(gt, basev);
            printf("~%zi [gt(link.last)] (hook_ref) \n", line_offset);
            continue;
        }

        //  FN, EXPRS
        value_fn_ref *fn_ref = NULL;
        value_fn_expr_refs *fn_expr = NULL;
        if ((fn_expr = try_build_fn_expr(lx)) != NULL)
        {
            printf("~%zi [gt(link.last)] (fn_call_expr) \n", line_offset);
            graph_block *fn_expr_block = create_block(RULE_TYPE::FN_REF_EXPR, fn_expr);
            _link_last_block(gt, fn_expr_block);
            continue;
        }
        else if ((fn_ref = try_build_fn_ref(lx)) != NULL)
        {
            printf("~%zi [gt(link.last)] (fn_call)\n", line_offset);
            graph_block *bl = create_block(RULE_TYPE::FN_REF, fn_ref);
            _link_last_block(gt, bl);
            continue;
        }

        //  TAGWORDS
        graph_block *tag = NULL;
        if ((tag = try_build_tagword(lx)) != NULL)
        {
            graph_block *_last;
            string _last_name;
            if (gt->last(_last))
                _last_name = lex::nameof(_last->type);
            else
                _last_name = "<undf>";

            if (_last == NULL)
            {
                printf("LAST IS NULL\n");
            }

            // specials
            if (tag->type == RULE_TYPE::GO)
            {
                // this is a entrypoint, no parent relates
                gt->add(tag, line_offset);
                printf("~%zi [gt] [tag].go\n", line_offset);
            }
            else
            {
                // parent
                graph_block *par;
                if (gt->parent(line_offset - 2, par))
                {
                    par->entries.push_back(tag);
                    printf("~%zi [gt, link=parent] %s -> %s\n", line_offset, lex::nameof(par->type), lex::nameof(tag->type));
                }

                gt->add(tag, line_offset);
            }

            continue;
        }

        if (lx.at(lx.cursor_get()) == '\n')
        {
            lx.cursor_move(1);
            line_offset = lx.skip(" \t");
        }
        else if (lx.can_read())
        {
            char t = lx.at(lx.cursor_get());
            if (!_is_space(t) && !_is_special_delim(t))
            {
                stringstream at;
                lx.get_cursor_dest(at);
                printf("[build] lex_graph: unrecognized symbol sequence at (%c) '%s'\n", t, at.str().c_str());
                delete gt;
                return NULL;
            }
            else
                lx.cursor_move(1);
        }
    }

    printf("Build lex graph end.\n");

    return gt;
};
