#include "lex_build.h"
#include <stack>

using namespace parcel::build;

// fix
lex *_new_lex(lex_type type, lvalue *val)
{
    lex *bl = new lex(type, val);
    return bl;
};

lex *_new_hook_def(string &name)
{
    hook_def *v = new hook_def(name);
    lex *b = new lex(lex_type::HOOK_DEF, v);
    return b;
}

bool _is_literal(lexer &lx)
{
    // 123.123.123 (ip...)
    // 123.123 (float)
    // 123 (int)
    // "word" -> litr_word
    // 'a' -> litr_symbol
    // "a" -> litr_word

    size_t old = lx.cursor_get();

    char single_pref = '_';
    string cur;
    if (lx.next_float(cur) != lx.npos)
    { // 123.123
        // lx.cursor_move(-cur.size());
        lx.cursor_set(old);
        return true;
    }
    else if (lx.next_int(cur) != lx.npos)
    { // 123
        // lx.cursor_move(-cur.size());
        lx.cursor_set(old);
        return true;
    }
    else if (lx.next_like_rounded(cur, "\"", "\"", "") != lx.npos)
    { // "app"
        // lx.cursor_move(-cur.size());
        lx.cursor_set(old);
        return true;
    }
    else if (lx.next_like_rounded(cur, "'", "'", "") != lx.npos)
    { // 'g'
        // lx.cursor_move(-cur.size());
        if (cur.size() == 3)
        {
            // lx.cursor_move(-cur.size());
            lx.cursor_set(old);
            return true;
        }
    }

    lx.cursor_set(old);

    return false;
};

bool _is_tspace(char t)
{
    string trash = "\r\t \n";
    return trash.find(t) != string::npos;
};

bool _is_special_delim(char t)
{
    string trash = ":&";
    return trash.find(t) != string::npos;
};

string _clear_expr_string(string &src)
{
    stringstream ss;
    for (char t : src)
    {
        if (_is_tspace(t) || t == '\n')
            continue;
        else
            ss << t;
    }

    return ss.str();
};

bool _check_s(string &s, const char *contains)
{
    if (s.find_first_of(contains) != string::npos)
    {
        return true;
    }

    return false;
};

/*
============================
*/

fn_arglist *try_build_fn_arglist(lexer &lx, bool &out_build_status)
{
    //
    auto old = lx.cursor_get();
    char pref = ' ';
    fn_arglist *head = NULL;
    fn_arglist *args = NULL;
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
            // else
            //  lx.cursor_move(-1);
        }

        string cur;
        if (lx.next_float(cur) != lx.npos)
        {
            string _val = string(cur.c_str());
            if (args == NULL)
            {
                args = new fn_arglist(lex_type::LITR_FLOAT, _val);
                head = args;
            }
            else
            {
                args->next_arg = new fn_arglist(lex_type::LITR_FLOAT, _val);
                args = args->next_arg;
            }
        }
        else if (lx.next_int(cur) != lx.npos)
        {
            string _val = string(cur.c_str());
            if (args == NULL)
            {
                args = new fn_arglist(lex_type::LITR_INT, _val);
                head = args;
            }
            else
            {
                args->next_arg = new fn_arglist(lex_type::LITR_INT, _val);
                args = args->next_arg;
            }
        }
        else if (lx.next_like_rounded(cur, "\"", "\"", "") != lx.npos)
        {
            string _val = string(cur.c_str());
            if (args == NULL)
            {
                args = new fn_arglist(lex_type::LITR_STR, _val);
                head = args;
            }
            else
            {
                args->next_arg = new fn_arglist(lex_type::LITR_STR, _val);
                args = args->next_arg;
            }
        }
        else if (lx.next_like_rounded(cur, "'", "'", "") != lx.npos)
        {
            char _valb = cur[1]; // we can take, because size of char view=3, like 'a', 'b'
            string _val = cur.substr(1, 1);
            if (args == NULL)
            {
                args = new fn_arglist(lex_type::LITR_CHAR, _val);
                head = args;
            }
            else
            {
                args->next_arg = new fn_arglist(lex_type::LITR_CHAR, _val);
                args = args->next_arg;
            }
        }
        else
        {
            fn_arglist *pt = head;
            while (pt != NULL)
            {
                delete pt;
                pt = pt->next_arg;
            };

            out_build_status = false;
            return NULL;
        }
    }

    if (head == NULL)
    {
        lx.cursor_set(old);
        out_build_status = false;
        return NULL;
    }

    out_build_status = true;
    return args;
};

fn_ref *try_build_fn_ref(lexer &lx)
{
    string fn_id;
    auto old = lx.cursor_get();

    // get name
    if (lx.next_id(fn_id) != lx.npos)
    {
        // printf("[try_build_fn_call] id=%s\n", fn_id.c_str());

        lx.skip(" \t");
        bool args_build = false;
        fn_arglist *args = try_build_fn_arglist(lx, args_build);
        if (!args_build)
        {
            lx.cursor_set(old);
            return NULL;
        }

        // printf("[try_build_fn_call] exr args\n");

        fn_ref *_val = new fn_ref(fn_id, args);
        return _val;
    }

    lx.cursor_set(old);
    return NULL;
}

bool try_build_fn_expr(stack<string> &postfix, btree<fn_ref *> *tree)
{
    if (postfix.empty())
    {
        // DEBUG_MSG("[try_build_fn_tree] postfix is empty, try mm");
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
            tree->left = new fn_expr();

            postfix.push(_left);
            if (!try_build_fn_expr(postfix, tree->left))
                return false;
        }
        else
        {
            fn_ref *fn_ref = NULL;
            lexer fn_lx(_left);
            if ((fn_ref = try_build_fn_ref(fn_lx)) == NULL)
                return false;

            tree->left = new fn_expr();
            tree->left->value = fn_ref;
        }

        // right
        auto _right = postfix.top();
        postfix.pop();

        if (_right == "|" || _right == "&")
        {
            tree->right = new fn_expr();
            postfix.push(_right);
            if (!try_build_fn_expr(postfix, tree->right))
                return false;
        }
        else
        {
            fn_ref *fn_ref = NULL;
            lexer fn_lx(_right);
            if ((fn_ref = try_build_fn_ref(fn_lx)) == NULL)
                return false;

            tree->right = new fn_expr();
            tree->right->value = fn_ref;
        }
    }

    return true;
};

/*
============================
*/

bool _table_link_last(offset_table<link_lex *> *gt, link_lex *linked_lex)
{
    link_lex *v;
    if (!gt->last(v))
    {
        printf("[_table_link_last] ret null\n");
        return false;
    }

    v->entries.push_back(linked_lex);
}

/*
============================
*/

lex *parcel::build::inplace_build_tag(lexer &lx)
{
    string tagname;
    auto c = lx.cursor_get();

    if (lx.next_word(tagname) != lx.npos)
    {
        lx.skip(" \t");
        if (lx.at(lx.cursor_get()) == LANG_TAG_PREFIX)
        {
            lx.cursor_move(1);

            lex_type type = typeof(tagname);
            lex *_bl = _new_lex(type, NULL); // tagword doesnt have a value
            return _bl;
        }
    }

    lx.cursor_set(c);
    return NULL;
};

lex *parcel::build::inplace_build_hook_def(lexer &lx)
{
    auto old = lx.cursor_get();
    if (lx.at(old) == LANG_HOOK_PREFIX)
    {
        lx.cursor_move(1);
        string v;
        if (lx.next_id(v) != lx.npos)
        {
            lx.skip(" \r\t");
            if (lx.at(lx.cursor_get()) == LANG_TAG_PREFIX)
            {
                lex *bl = _new_hook_def(v);
                return bl;
            }
        }
    }

    lx.cursor_set(old);
    return NULL;
};

lex *parcel::build::inplace_build_hook_ref(lexer &lx)
{
    auto old = lx.cursor_get();
    if (lx.at(old) == LANG_HOOK_PREFIX)
    {
        lx.cursor_move(1);
        string v;
        if (lx.next_id(v) != lx.npos)
        {
            lex *bl = _new_lex(lex_type::HOOK_REF, new hook_ref(v));
            return bl;
        }
    }

    lx.cursor_set(old);
    return NULL;
};

lex *parcel::build::inplace_build_link_def(lexer &lx)
{
    auto old = lx.cursor_get();
    if (lx.at(old) == LANG_LINK_PREFIX)
    {
        lx.cursor_move(1);
        string v;
        if (lx.next_id(v) != lx.npos)
        {
            lx.skip(" \r\t");
            if (lx.at(lx.cursor_get()) == LANG_TAG_PREFIX)
            {
                lex *bl = _new_lex(lex_type::LINK_DEF, new link_def(v));
                return bl;
            }
        }
    }

    lx.cursor_set(old);
    return NULL;
};

lex *parcel::build::inplace_build_literal(lexer &lx)
{
    if (!_is_literal(lx))
        return NULL;

    size_t old = lx.cursor_get();

    string cur;
    if (lx.next_float(cur) != lx.npos)
    {
        float _val = stof(cur.c_str());
        auto _bl = _new_lex(lex_type::LITR_FLOAT, new value_litr_float(_val));
        return _bl;
        //_link_last_block(gt, _bl);
        // printf("~%zi [gt(link.last)] (float) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
    }
    else if (lx.next_int(cur) != lx.npos)
    {
        int _val = stoi(cur.c_str());
        auto _bl = _new_lex(lex_type::LITR_INT, new value_litr_int(_val));
        return _bl;
        //_link_last_block(gt, _bl);
        // printf("~%zi [gt(link.last)] (int) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
    }
    else if (lx.next_like_rounded(cur, "\"", "\"", "") != lx.npos)
    {
        string _val = string(cur.c_str());
        auto _bl = _new_lex(lex_type::LITR_STR, new value_litr_string(_val));
        return _bl;
        //_link_last_block(gt, _bl);
        // printf("~%zi [gt(link.last)] (str) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
    }
    else if (lx.next_like_rounded(cur, "'", "'", "") != lx.npos)
    {
        char _val = cur[1]; // we can take, because size of char view=3, like 'a', 'b'
        auto _bl = _new_lex(lex_type::LITR_CHAR, new value_litr_char(_val));
        return _bl;
        //_link_last_block(gt, _bl);
        // printf("~%zi [gt(link.last)] (char) %s -> %s\n", line_offset, _last_name.c_str(), cur.c_str());
    }
    else
    {
        lx.cursor_set(old);
        return NULL;
    }
};

lex *parcel::build::inplace_build_basetype(lexer &lx)
{
    size_t old = lx.cursor_get();
    string id;

    if (lx.next_id(id) != lx.npos)
    {
        lex_type type = typeof(id);
        if (type == lex_type::BL_WORD ||
            type == lex_type::BL_NUMBER)
        {
            return new lex(type, NULL);
        }
    }

    lx.cursor_set(old);
    return NULL;
};

lex *parcel::build::inplace_build_fn_ref(lexer &lx)
{
    fn_ref *v = try_build_fn_ref(lx);
    if (v != NULL)
    {
        lex *l = new lex(lex_type::FN_REF, v);
        return l;
    }
    else
        return NULL;
};

lex *parcel::build::inplace_build_fn_expr(lexer &lx)
{
    string expr_s;
    auto old = lx.cursor_get();

    lx.next_until("\n\r", expr_s);
    lx.cursor_set(old);
    auto next_cursor = old + expr_s.size();

    if (_check_s(expr_s, LEX_SYMBOLS_NO_EXPR))
        return NULL;

    expr_s.insert(0, "(");
    expr_s.append(")");

    string normalized_expr_buff = _clear_expr_string(expr_s);
    lexer lx2(normalized_expr_buff);

    // printf("FN_ARG=%s\n", normalized_expr_buff.c_str());

    stack<string> expr_postfix;
    if (!expr::to_postfix(lx2, expr_postfix))
    {
        printf("!to_postfix=%i\n", expr_postfix.size());
        return NULL;
    }

    if (expr_postfix.size() <= 1)
    {
        // printf("EXPR+POST=%i\n", expr_postfix.size());
        lx.cursor_set(old);
        return NULL;
    }

    fn_expr *tr = new fn_expr();
    if (!try_build_fn_expr(expr_postfix, tr))
    {
        delete tr;
        // DEBUG_MSG("[try_build_fn_expr] Cant build fn expr tree");
        return NULL;
    }

    lx.cursor_set(next_cursor);

    lex *l = new lex(lex_type::FN_REF_EXPR, tr);
    return l;
};

offset_table<link_lex *> *parcel::build::build_lex_table(string &src)
{
    offset_table<link_lex *> *gt = new offset_table<link_lex *>();

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
        lex *bl = NULL;
        if ((bl = inplace_build_hook_def(lx)) != NULL)
        {

            gt->add({new link_lex(bl)}, line_offset);
            printf("~%zi [gt(nolink))].hook_def\n", line_offset);
            continue;
        }
        else if ((bl = inplace_build_hook_ref(lx)) != NULL)
        {
            link_lex *linked = new link_lex(bl);
            _table_link_last(gt, linked);

            gt->add(linked, line_offset);
            printf("~%zi [gt(nolink))].hook_ref\n", line_offset);
            continue;
        }
        else if ((bl = inplace_build_link_def(lx)) != NULL)
        {
            gt->add(new link_lex(bl), line_offset);
            printf("~%zi [gt(nolink))].link\n", line_offset);
            continue;
        }

        // LITERALS, VALUES
        lex *lit;
        if ((lit = inplace_build_literal(lx)) != NULL)
        {
            link_lex *linked = new link_lex(lit);
            _table_link_last(gt, linked);

            printf("~%zi [gt(link-last))].literal\n", line_offset);
            continue;
        }

        // FNS, EXPR
        lex *fn;
        if ((fn = inplace_build_fn_expr(lx)) != NULL)
        {
            link_lex *linked = new link_lex(fn);
            _table_link_last(gt, linked);

            gt->add(linked, line_offset);
            printf("~%zi [gt(link-last))].fn_expr\n", line_offset);
            continue;
        }
        else if ((fn = inplace_build_fn_ref(lx)) != NULL)
        {
            link_lex *linked = new link_lex(fn);
            _table_link_last(gt, linked);

            gt->add(linked, line_offset);
            printf("~%zi [gt(link-last))].fn_ref\n", line_offset);
            continue;
        }

        // TAGS
        lex *tag;
        if ((tag = inplace_build_tag(lx)) != NULL)
        {
            // root tags
            if (line_offset == 0)
            {
                link_lex *linked = new link_lex(tag);
                _table_link_last(gt, linked);

                gt->add(linked, line_offset);
            }
            else
            {
                link_lex *linked = new link_lex(tag);
                _table_link_last(gt, linked);

                gt->add(linked, line_offset);
            }

            printf("~%zi [gt(link-last))].tag\n", line_offset);
            continue;
        }

        // BASETYPE
        lex *basetype;
        if ((basetype = inplace_build_basetype(lx)) != NULL)
        {
            // root tags
            if (line_offset == 0)
            {
                link_lex *linked = new link_lex(basetype);
                _table_link_last(gt, linked);

                gt->add(linked, line_offset);
            }
            else
            {
                link_lex *linked = new link_lex(basetype);
                _table_link_last(gt, linked);

                gt->add(linked, line_offset);
            }

            printf("~%zi [gt(link-last))].basetype\n", line_offset);
            continue;
        }

        if (lx.at(lx.cursor_get()) == '\n')
        {
            lx.cursor_move(1);
            line_offset = lx.skip(" \t");
        }
        else if (lx.can_read())
        {
            // if_word_ex
            char t = lx.at(lx.cursor_get());
            if ((!_is_tspace(t) && !_is_special_delim(t)))
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