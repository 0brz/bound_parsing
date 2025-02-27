#ifndef _PARSE_BUILD_H_
#define _PARSE_BUILD_H_

#include "../lang/lang.h"
#include "../lexems/lexem.h"
#include "../tools/lexer/lex.h"
#include "../tools/offset_table.h"
#include "../types/types.h"
#include "../expr/expr.h"
#include "../funcs/fn.h"
#include "lex_build.h"
#include "../parse/parser.h"
// #include "lex_build.h"
#include <vector>

using namespace std;
using namespace parcel::type;
using namespace parcel::lexems;
using namespace parcel::lang;
using namespace parcel::expr;
using namespace parcel::funcs;
using namespace parcel::parser;
using namespace parcel::build;

using ps_elem = parcel::parser::element;

namespace parcel
{
    namespace build
    {
        class instr
        {
        private:
            std::map<string, token_hook *> map_hooks;
            std::vector<prog_go *> reg_entries{};

            bool check_lex_size(link_lex *lex, int need)
            {
                return lex->entries.size() >= need;
            };

            ps_elem *deep_build(link_lex *cur_lex, std::vector<ps_elem *> &builded)
            {
                if (cur_lex == NULL)
                    return NULL;

                auto ttype = cur_lex->val->type;

                if (ttype == BL_LIST)
                {
                    if (!check_lex_size(cur_lex, 1))
                    {
                        printf("deep_build: [ERR] <list> check_lex_size\n");
                        return NULL;
                    }

                    auto pick_next = cur_lex->entries.at(0);
                    ps_elem *build_next = deep_build(pick_next, builded);
                    if (build_next == NULL)
                    {
                        printf("deep_build: [ERR] <list> build_next returned 'null'\n");
                        return NULL;
                    }

                    builded.push_back(build_next);
                    ps_elem *list = new ps_elem(lex_type::BL_LIST, new parser::list(build_next));
                    builded.push_back(list);

                    printf("deep_build: [ok] <list>\n");
                    return list;
                }
                else if (ttype == HOOK_DEF)
                {
                    hook_def *v = static_cast<hook_def *>(cur_lex->val->value);
                    if (v != NULL)
                    {
                        // reg hook with deep sturcture.
                        if (map_hooks.find(v->name) == end(map_hooks))
                        {
                            // build hook.
                            if (!check_lex_size(cur_lex, 1))
                            {
                                printf("deep_build: [ERR] <hook_def> check_lex_size\n");
                                return NULL;
                            }

                            auto pick_base = cur_lex->entries.at(0);
                            ps_elem *build_base = deep_build(pick_base, builded);
                            if (build_base == NULL)
                            {
                                printf("deep_build: [ERR] <hook_def> build_base is null\n");
                                return NULL;
                            }

                            builded.push_back(build_base);

                            map_hooks[v->name] = new token_hook(v->name, build_base);
                            printf("deep_build: [ok] <hook_def> '%s' added to map.\n", v->name.c_str());
                        }
                        else
                        {
                            printf("deep_build: [ERR] <hook_def> '%s' already in map\n", v->name.c_str());
                        }
                    }
                }
                else if (ttype == HOOK_REF)
                {
                    hook_ref *v = static_cast<hook_ref *>(cur_lex->val->value);
                    if (v != NULL)
                    {
                        // use hook with linking.
                        auto find_hook = map_hooks.find(v->name);
                        if (find_hook != end(map_hooks))
                        {

                            token_hook *hk = (*find_hook).second;
                            ps_elem *he = new ps_elem(lex_type::HOOK_REF, hk);
                            // printf("deep_build: [ok] <hook_ref>\n");
                            return he;
                        }
                    }
                }
                else if (ttype == BL_WORD ||
                         ttype == BL_NUMBER)
                {
                    ps_elem *cur;
                    switch (ttype)
                    {
                    case BL_WORD:
                        cur = new ps_elem(ttype, new parser::word());
                        break;

                    case BL_NUMBER:
                        cur = new ps_elem(ttype, new parser::num());
                        break;

                    default:
                        break;
                    }

                    builded.push_back(cur);
                    printf("deep_build: [ok] <word/num>\n");
                    return cur;
                }
                else
                {
                    return NULL;
                }
            };

            bool add_map_hook(token_hook *hook)
            {
                if (map_hooks.find(hook->name) != end(map_hooks))
                {
                    return false;
                }

                map_hooks[hook->name] = hook;
                return true;
            };

        public:
            bool build(offset_table<link_lex *> *table)
            {
                // instr *inst = new instr();
                std::vector<ps_elem *> all_builds;

                // go by roots
                std::vector<link_lex *> roots = table->get_roots();
                for (link_lex *lex : roots)
                {
                    // pick next
                    if (!check_lex_size(lex, 1))
                    {
                        printf("instr_builder: !check_lex_size %s\n", lex->val->name());
                        // error
                        return NULL;
                    }

                    link_lex *pick_child = lex->entries.at(0);
                    printf("child=%s\n", pick_child->val->name());
                    ps_elem *build_child = deep_build(pick_child, all_builds);

                    if (lex->val->type == lex_type::GO)
                    {
                        prog_go *go = new prog_go(build_child);
                        reg_entries.push_back(go);
                        printf("instr_builder: [OK] <go>->%s\n", type::nameof(build_child->type));
                    }
                    else if (lex->val->type == lex_type::LINK_DEF)
                    {
                        printf("instr_builder: [OK] <link_def>\n");
                    }
                    else if (lex->val->type == lex_type::HOOK_DEF)
                    {
                        hook_def *def = static_cast<hook_def *>(lex->val->value);
                        if (def != NULL)
                        {
                            token_hook *tok_hook = new token_hook(def->name, build_child);
                            if (!add_map_hook(tok_hook))
                            {
                                // err
                            }

                            printf("instr_builder: [OK] <hook_def>\n");
                        }
                    }
                    else
                    {
                        printf("instr_builder: [ERR] unrecognized prog entry lex.\n");
                    }
                }

                return true;
            };

            void propagate(string &lex)
            {
                for (prog_go *e : reg_entries)
                {
                    e->act(lex, NULL, NULL);
                }
            };

            token_hook *find_hook(string name)
            {
                auto f = map_hooks.find(name);
                if (f != end(map_hooks))
                {
                    return (*f).second;
                }
                else
                    return NULL;
            };
        };
    }
}

#endif
