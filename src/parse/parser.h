#ifndef _PARSE_PARSER_H_
#define _PARSE_PARSER_H_

#include <map>
#include <string>
#include <vector>
#include "token.h"
#include <queue>
#include "../types/types.h"
#include "../tools/ring.h"

using namespace parcel::tokens;
using namespace parcel::type;

namespace parcel
{
    namespace parser
    {
        class parse_cursor
        {
        public:
            size_t pos;
            size_t size;
            parse_cursor(size_t pos, size_t sz) : pos(pos), size(sz) {};
            bool at_end() { return pos == size - 1; };
        };

        enum act_result
        {
            MOD,
            ACT,
            FAIL,
            ERR_TOKEN_TYPE,
        };

        struct pr_val
        {
            // fn info
            virtual act_result act(string &lex, token *par, token *t2 = NULL) = 0;
            virtual void reset() = 0;
        };

        struct element
        {
            lex_type type;
            pr_val *val;

            element(lex_type type, pr_val *v) : type(type), val(v) {};
            void reset() {};

            act_result act(string &lex, token *par, token *t2 = NULL)
            {
                if (val != NULL)
                {
                    return val->act(lex, par, t2);
                }

                return FAIL;
            };
        };

        // ------------- TOOLS
        void list_add(token *list, token *tk)
        {
            val_list *v = static_cast<val_list *>(list->val);
            if (v != NULL)
            {
                v->v.push_back(tk);
            }
        };

        void set_add(token *list, token *tk)
        {
            val_set *v = static_cast<val_set *>(list->val);
            if (v != NULL)
            {
                v->v.push_back(tk);
            }
        };

        token *create_tk_for(element *el)
        {
            auto tk = new token();
            if (el->type == parcel::type::BL_LIST)
            {
                tk->type = tokens::list;
                tk->val = new val_list();
            }
            else if (el->type == parcel::type::BL_SET)
            {
                tk->type = tokens::set;
                tk->val = new val_set();
            }
            else if (el->type == parcel::type::BL_VEC)
            {
                tk->type = tokens::vec;
                // tk->val = new val_vector();
            }

            return tk;
        };

        // ------------- TOOLS

        struct word : pr_val
        {
            void reset() {};
            act_result act(string &lex, token *par, token *t2 = NULL)
            {
                // printf("pr_word\n");
                if (!isalpha(lex[0]))
                    return FAIL;

                par->type = tokens::type::word;
                par->val = new val_word(lex);
                return ACT;
            }
        };

        struct num : pr_val
        {
            void reset() {};
            act_result act(string &lex, token *par, token *t2 = NULL)
            {
                // printf("pr_num\n");
                if (lex[0] >= '0' && lex[0] <= '9')
                {
                    par->type = tokens::type::number;
                    par->val = new val_num(lex);
                    return ACT;
                }

                // int v = stoi(lex);
                return FAIL;
            }
        };

        struct literal_string : pr_val
        {
            string val;

            void reset() {};

            act_result act(string &lex, token *par, token *t2 = NULL)
            {
                if (!lex.empty() && lex == val)
                {
                    if (par != NULL)
                    {
                        par->type = tokens::type::literal_string;
                        par->val = new val_string(lex);
                        return ACT;
                    }

                    return ACT;
                }
                else
                    return FAIL;
            }

            literal_string(const char *t) : val(t) {};
        };

        struct literal_char : pr_val
        {
            char val;

            void reset() {};

            act_result act(string &lex, token *par, token *t2 = NULL)
            {
                if (!lex.empty() && lex.at(0) == val)
                {
                    if (par != NULL)
                    {
                        par->type = tokens::type::literal_char;
                        par->val = new val_char(val);
                        return ACT;
                    }

                    return ACT;
                }
                else
                    return FAIL;
            }

            literal_char(char t) : val(t) {};
        };

        struct list : pr_val
        {
            element *el;
            token *el_t;  // storing the deep stucture
            token *el_t2; // t2 param

            void reset() {
                // el->reset();
            };

            list(element *basetype)
            {
                el = basetype;
                el_t = create_tk_for(el);
                el_t2 = create_tk_for(el);
            }

            act_result act(string &lex, token *par, token *t2 = NULL)
            { // par=list

                auto res = el->act(lex, el_t, el_t2);

                if (res == ACT)
                {

                    if (el_t2->val != NULL)
                    {
                        printf("__[list]: ACT (r2) s=%s\n", lex.c_str());
                        list_add(par, el_t2);
                        el_t2 = create_tk_for(el);
                        // fix: recreate toke
                    }
                    else
                    {
                        printf("__[list]: ACT s=%s\n", lex.c_str());
                    }

                    if (par->val != NULL)
                    {
                        // printf("list_add_1\n");
                        list_add(par, el_t);
                        // printf("list_add\n");
                        el_t = create_tk_for(el);
                    }
                }
                else if (res == MOD)
                {
                    printf("__[list]: mod s=%s\n", lex.c_str());
                    // keep building...
                }
                else if (res == FAIL)
                {
                    printf("__[list]: fail s=%s\n", lex.c_str());
                    // clear token?
                }

                return res;
            };
        };

        struct set : pr_val
        {
            // for each pair el-token
            std::vector<element *> els_row;
            std::vector<token *> tk_row; // storing the deep stucture

            void reset() {};

            set(std::vector<element *> &els)
            {
                els_row = els;
                for (element *e : els_row)
                {
                    tk_row.push_back(create_tk_for(e));
                }
            }

            act_result act(string &lex, token *par, token *t2 = NULL)
            { // par=set
                // printf("pr_list\n");

                bool is_act = false;
                bool is_mod = false;

                for (int i = 0; i < els_row.size() && i < tk_row.size(); i++)
                {
                    auto res = els_row[i]->act(lex, tk_row[i]);

                    if (res == ACT)
                    {
                        set_add(par, tk_row[i]);
                        // recreate old token store
                        tk_row[i] = create_tk_for(els_row[i]);
                        is_act = true;
                    }
                    else if (res == MOD)
                    {
                        // keep building...
                        is_mod = true;
                    }
                    else if (res == FAIL)
                    {
                        // clear token?
                    }
                }

                if (is_act)
                    return ACT;
                if (is_mod)
                    return MOD;
            };
        };

        // ------------- UTILS.
        void deep_clear_tokens(token *root)
        {
            queue<token *> q;
            q.push(root);

            printf("____[deep_clear_lists]\n");

            while (!q.empty())
            {
                token *t = q.front();
                q.pop();

                if (t->type == tokens::list)
                {
                    val_list *ls = static_cast<val_list *>(t->val);
                    printf("_____LS=%i\n", ls->v.size());
                    for (auto tk : ls->v)
                    {
                        q.push(tk);
                    }
                    ls->v.clear();
                    printf("_____LS2=%i\n", ls->v.size());
                }
                else if (t->type == tokens::set)
                {
                    val_set *ls = static_cast<val_set *>(t->val);
                    for (auto tk : ls->v)
                    {
                        q.push(tk);
                    }
                    ls->v.clear();
                }
                else if (t->type == tokens::vec)
                {
                    val_vector *vec = static_cast<val_vector *>(t->val);
                    for (auto tk : vec->v)
                    {
                        q.push(tk);
                    }
                    // vec->v.clear();
                }
            }

            if (root->type == tokens::list)
            {
            }
        };

        void deep_reset_elems(element *root)
        {
            queue<element *> q;
            q.push(root);

            while (!q.empty())
            {
                element *t = q.front();
                q.pop();

                // pay attention to collecitons.
                // they need be clear all represented values
                // tokens: deep_clear lists

                if (t->type == type::BL_SET)
                {
                    set *set = static_cast<parser::set *>(t->val);
                    if (set != NULL)
                    {
                        for (auto de : set->els_row)
                        {
                            q.push(de);
                        }

                        for (auto tk : set->tk_row)
                        {
                            deep_clear_tokens(tk);
                        }
                    }
                }
                else if (t->type == type::BL_LIST)
                {
                    parser::list *ls = static_cast<parser::list *>(t->val);
                    if (ls != NULL)
                    {
                        q.push(ls->el);
                        deep_clear_tokens(ls->el_t);
                    }
                }
                /*
                else if (t->type == cond_type::type_ps_vec) {
                    pr_vector* vec = static_cast<pr_vector*>(t->val);
                    vec->reset();
                }
                    */

                t->val->reset();
            }
        };

        bool is_collection(type::lex_type type)
        {
            switch (type)
            {
            case type::lex_type::BL_SET:
            case type::lex_type::BL_LIST:
                return true;

            default:
                return false;
            }
        };

        bool collection_empty(token *tk)
        {
            if (tk->type == tokens::list)
            {
                val_list *ls = static_cast<val_list *>(tk->val);
                return ls->v.empty();
            }
            else if (tk->type == tokens::set)
            {
                val_set *ls = static_cast<val_set *>(tk->val);
                return ls->v.empty();
            }
            return true;
        };
        // ------------- UTILS.

        struct vector : pr_val
        {
            ring<pair<element *, token *> *> el_ring;
            shared_ptr<parse_cursor> lex_cursor;

            void reset()
            {
                // reset deep refs
                el_ring.to_begin();

                auto row = el_ring.as_row();
                for (auto &e : row)
                {
                    e->first->reset();
                    // deep_reset_elems(e->first);
                    deep_clear_tokens(e->second);
                }
            };

            void build_out(token *par)
            {
                std::vector<token *> builded_row;
                auto elems = el_ring.as_row();
                for (auto &e : elems)
                {
                    builded_row.push_back(e->second);
                    e->second = create_tk_for(e->first);
                }

                val_vector *vec = new val_vector(builded_row);
                par->type = tokens::vec;
                par->val = vec;
            }

            vector(std::vector<element *> &v, shared_ptr<parse_cursor> cursor)
            {
                lex_cursor = cursor;

                for (element *e : v)
                {
                    pair<element *, token *> *pr = new pair<element *, token *>{e, create_tk_for(e)};
                    el_ring.add_end(pr);
                }

                el_ring.to_begin();
                printf("vec.ring.builded=%i\n", el_ring.len());
            };

            bool is_ring_ending()
            {
                if (el_ring.pos() == el_ring.len() - 1)
                {
                    return true;
                }
                else
                    return false;
            };

            act_result act(string &lex, token *par, token *t2 = NULL)
            { // vec

                pair<element *, token *> *cur_pair = el_ring.get_current();
                act_result res = cur_pair->first->act(lex, cur_pair->second);

                printf("-------------[vec].act() s=%s ring=%s\n", lex.c_str(), nameof(cur_pair->first->type));

                bool prev_ending = is_ring_ending();

                if (res == ACT)
                {
                    if (is_ring_ending())
                    { // finish build vec.
                        bool is_coll = is_collection(cur_pair->first->type);
                        // printf("[vec].act() END.ACT s=%s is_coll=%i\n", lex.c_str(), is_coll);

                        //
                        if (is_ring_ending() && is_collection(cur_pair->first->type) && (lex_cursor.get()->at_end()))
                        {
                            printf("[vec].act() (ACT) -> ACT  list,ring_end,curs_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            // ring end
                            // curs end
                            // last coll
                            build_out(par);
                            el_ring.to_begin();

                            return ACT;
                        }

                        // end.list (cont rule)
                        if (is_collection(cur_pair->first->type))
                        {
                            printf("[vec].act() (ACT) -> MOD  list,ring_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            return MOD;
                        }
                        // end.t
                        else
                        {
                            printf("[vec].act() (ACT) -> ACT  T,ring_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());

                            build_out(par);
                            el_ring.to_begin();

                            return ACT;
                        }
                    }
                    else
                    {
                        // printf("[vec].act() MID.ACT s=%s\n", lex.c_str());

                        // mid.list
                        if (is_collection(cur_pair->first->type))
                        {
                            printf("[vec].act() (ACT) -> MOD mid.list s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            return MOD;
                        }
                        // mid.t
                        else
                        {
                            printf("[vec].act() (ACT) -> MOD  mid.T s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            el_ring.to_next();
                            return MOD;
                        }
                    }
                }
                else if (res == MOD)
                {
                    // end.mod
                    // mid.mod
                    // curs.end
                    // printf("LAST_ENDING curs=%i\n", lex_cursor.get()->pos);
                    if (is_ring_ending() && is_collection(cur_pair->first->type) && (lex_cursor.get()->at_end()))
                    {

                        printf("[vec].act() (MOD) -> ACT curs_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                        // ring end
                        // curs end
                        // last coll
                        build_out(par);
                        el_ring.to_begin();

                        return ACT;
                    }

                    printf("[vec].act() (MOD) -> MOD s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                    return MOD;
                }
                else if (res == FAIL)
                {

                    // list take next rule.
                    if (is_collection(cur_pair->first->type) &&
                        !collection_empty(cur_pair->second))
                    {
                        // try pick next.
                        // end/mid/beg
                        el_ring.to_next();
                        auto next_pair = el_ring.get_current();
                        // use temp token to not change current state.
                        // FIX: we need deep CLEARED head fr this vector
                        token *temp_token = create_tk_for(next_pair->first);
                        act_result next_res = next_pair->first->act(lex, temp_token);

                        // ring_end, curs_end, next_res, next_type
                        /*
                            coll, t, coll
                            coll, t
                        */
                        if (next_res == ACT)
                        {
                            // n.act.end
                            // n.end, t
                            // n.end, list (curs_end)
                            if (is_ring_ending() && is_collection(next_pair->first->type))
                            {
                                if (lex_cursor.get()->at_end())
                                {
                                    printf("+++++++++ s=%s [FAIL, ACT, coll, ring-end, curs-end] -> ACT %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                    next_pair->second = temp_token;
                                    build_out(par);
                                    el_ring.to_begin();
                                    return ACT;
                                }
                                else
                                {
                                    printf("+++++++++ s=%s [FAIL, ACT, coll, ring-end] -> MOD %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                    next_pair->second = temp_token;
                                    // its not curs_end, keep building last list.
                                    return MOD;
                                }
                            }
                            else if (is_ring_ending() && !is_collection(next_pair->first->type))
                            {
                                // on last elem. T, Build
                                printf("+++++++++ s=%s [FAIL, ACT, !coll, ring-end] -> ACT %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                next_pair->second = temp_token;
                                build_out(par);
                                el_ring.to_begin();
                                return ACT;
                            }
                            else if (prev_ending)
                            {
                                printf("+++++++++ s=%s [FAIL, ACT, coll, prev-end] -> ACT %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                build_out(par);
                                next_pair->second = temp_token;
                                if (!is_collection(next_pair->first->type))
                                    el_ring.to_next();

                                return ACT;
                            }
                            else
                            {
                                // activate first elem, last list
                                printf("+++++++++ s=%s [FAIL, ACT, any] -> MOD %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                next_pair->second = temp_token;
                                // step on next in the mid
                                return MOD;
                            }
                        }
                        else if (next_res == MOD)
                        {
                            // begin build first|next block
                            // go to first from last list.
                            // TEMMP ONLY THERE!
                            if (prev_ending)
                            {
                                build_out(par);
                                next_pair->second = temp_token;
                                return ACT;
                            }
                            else
                            {
                                next_pair->second = temp_token;
                                // curs_end,...
                                return MOD;
                            }
                        }
                        else if (next_res == FAIL)
                        {
                            this->reset();
                            return FAIL;
                        }
                    }

                    printf("[vec].act() (FAIL) -> FAIL <reset> s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                    this->reset();
                    // other elem
                    return FAIL;
                }

                printf("[vec].act() (UNDEF CASE) -> FAIL  s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                return FAIL;
            };
        };

        struct seq : pr_val
        {
            ring<pair<element *, token *> *> el_ring;
            shared_ptr<parse_cursor> lex_cursor;

            void reset()
            {
                // reset deep refs
                el_ring.to_begin();

                auto row = el_ring.as_row();
                for (auto &e : row)
                {
                    e->first->reset();
                    // deep_reset_elems(e->first);
                    deep_clear_tokens(e->second);
                }
            };

            void build_out(token *par)
            {
                std::vector<token *> builded_row;
                auto elems = el_ring.as_row();
                for (auto &e : elems)
                {
                    builded_row.push_back(e->second);
                    e->second = create_tk_for(e->first);
                }

                val_vector *vec = new val_vector(builded_row);
                par->type = tokens::vec;
                par->val = vec;
            }

            seq(std::vector<element *> &v, shared_ptr<parse_cursor> cursor)
            {
                lex_cursor = cursor;

                for (element *e : v)
                {
                    pair<element *, token *> *pr = new pair<element *, token *>{e, create_tk_for(e)};
                    el_ring.add_end(pr);
                }

                el_ring.to_begin();
                printf("vec.ring.builded=%i\n", el_ring.len());
            };

            bool is_ring_ending()
            {
                if (el_ring.pos() == el_ring.len() - 1)
                {
                    return true;
                }
                else
                    return false;
            }

            act_result act(string &lex, token *par, token *t2 = NULL)
            { // vec

                pair<element *, token *> *cur_pair = el_ring.get_current();
                act_result res = cur_pair->first->act(lex, cur_pair->second);

                printf("-------------[seq].act() s=%s ring=%s\n", lex.c_str(), nameof(cur_pair->first->type));

                bool prev_ending = is_ring_ending();

                if (res == ACT)
                {
                    if (is_ring_ending())
                    { // finish build vec.

                        if (is_ring_ending() && is_collection(cur_pair->first->type) && (lex_cursor.get()->at_end()))
                        {
                            printf("[seq].act() (ACT) -> ACT  list,ring_end,curs_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            // ring end
                            // curs end
                            // last coll
                            build_out(par);
                            el_ring.to_begin();

                            return ACT;
                        }

                        // end.list (cont rule)
                        if (is_collection(cur_pair->first->type))
                        {
                            printf("[seq].act() (ACT) -> MOD  list,ring_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            return MOD;
                        }
                        // end.t
                        else
                        {
                            printf("[seq].act() (ACT) -> ACT  T,ring_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());

                            build_out(par);
                            el_ring.to_begin();

                            return ACT;
                        }
                    }
                    else
                    {
                        // printf("[vec].act() MID.ACT s=%s\n", lex.c_str());

                        // mid.list
                        if (is_collection(cur_pair->first->type))
                        {
                            printf("[seq].act() (ACT) -> MOD mid.list s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            return MOD;
                        }
                        // mid.t
                        else
                        {
                            printf("[seq].act() (ACT) -> MOD  mid.T s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                            el_ring.to_next();
                            return MOD;
                        }
                    }
                }
                else if (res == MOD)
                {
                    // end.mod
                    // mid.mod
                    // curs.end
                    if (is_ring_ending() && is_collection(cur_pair->first->type) && (lex_cursor.get()->at_end()))
                    {

                        printf("[seq].act() (MOD) -> ACT curs_end s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                        // ring end
                        // curs end
                        // last coll
                        build_out(par);
                        el_ring.to_begin();

                        return ACT;
                    }

                    printf("[seq].act() (MOD) -> MOD s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                    return MOD;
                }
                else if (res == FAIL)
                {

                    // list take next rule.
                    if (is_collection(cur_pair->first->type) &&
                        !collection_empty(cur_pair->second))
                    {
                        // try pick next.
                        // end/mid/beg
                        el_ring.to_next();
                        auto next_pair = el_ring.get_current();
                        // use temp token to not change current state.
                        // FIX: we need deep CLEARED head fr this vector
                        token *temp_token = create_tk_for(next_pair->first);
                        act_result next_res = next_pair->first->act(lex, temp_token);

                        // ring_end, curs_end, next_res, next_type
                        /*
                            coll, t, coll
                            coll, t
                        */
                        if (next_res == ACT)
                        {
                            // n.act.end
                            // n.end, t
                            // n.end, list (curs_end)
                            if (is_ring_ending() && is_collection(next_pair->first->type))
                            {
                                if (lex_cursor.get()->at_end())
                                {
                                    printf("+++++++++ s=%s [FAIL, ACT, coll, ring-end, curs-end] -> ACT %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                    next_pair->second = temp_token;
                                    build_out(par);
                                    el_ring.to_begin();
                                    return ACT;
                                }
                                else
                                {
                                    printf("+++++++++ s=%s [FAIL, ACT, coll, ring-end] -> MOD %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                    next_pair->second = temp_token;
                                    // its not curs_end, keep building last list.
                                    return MOD;
                                }
                            }
                            else if (is_ring_ending() && !is_collection(next_pair->first->type))
                            {
                                // on last elem. T, Build
                                printf("+++++++++ s=%s [FAIL, ACT, !coll, ring-end] -> ACT %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                next_pair->second = temp_token;
                                build_out(par);
                                el_ring.to_begin();
                                return ACT;
                            }
                            else if (prev_ending)
                            {
                                printf("+++++++++ s=%s [FAIL, ACT, coll, prev-end] -> ACT %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                build_out(par);
                                next_pair->second = temp_token;
                                if (!is_collection(next_pair->first->type))
                                    el_ring.to_next();

                                return ACT;
                            }
                            else
                            {
                                // activate first elem, last list
                                printf("+++++++++ s=%s [FAIL, ACT, any] -> MOD %s\n", lex.c_str(), nameof(cur_pair->first->type));
                                next_pair->second = temp_token;
                                // step on next in the mid
                                return MOD;
                            }
                        }
                        else if (next_res == MOD)
                        {
                            // begin build first|next block
                            // go to first from last list.
                            // TEMMP ONLY THERE!
                            if (prev_ending)
                            {
                                build_out(par);
                                next_pair->second = temp_token;
                                return ACT;
                            }
                            else
                            {
                                next_pair->second = temp_token;
                                // curs_end,...
                                return MOD;
                            }
                        }
                        else if (next_res == FAIL)
                        {
                            // this->reset();
                            return FAIL;
                        }
                    }

                    printf("[seq].act() (FAIL) -> FAIL <reset> s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                    // this->reset();
                    //  other elem
                    return FAIL;
                }

                printf("[seq].act() (UNDEF CASE) -> FAIL  s='%s' ring=%i\n", lex.c_str(), el_ring.pos());
                return FAIL;
            };
        };

        struct prog_go : pr_val
        {
            element *base; // by value

            void reset() {};

            prog_go(element *bs)
            {
                base = bs;
            }

            act_result act(string &lex, token *par, token *r2)
            {
                return base->act(lex, par, r2);
            }
        };

        struct token_hook : pr_val
        {
            string name;
            element *base; // by value
            token *tk;     // storing deep structure

            void reset() {};

            token_hook(string nam, element *bs)
            {
                name = nam;
                base = bs;
                tk = create_tk_for(base);
            }

            act_result act(string &lex, token *par, token *r2)
            {
                printf("[token_hook] par\n");
                act_result res = base->act(lex, tk);
                if (res == act_result::FAIL)
                {
                    // deep clear ?
                    // feature: hook strict mode. clear after fail
                }

                return res;
            };
        };

        // ------------- ANALYZER.

    }
}

#endif