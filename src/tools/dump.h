#ifndef _DUMP_
#define _DUMP_

#include "../lexems/lexem.h"
#include "../builder/lex_build.h"
#include "../parse/parser.h"
#include "../parse/token.h"
#include "offset_table.h"
#include <iomanip>
#include <memory>
#include <queue>

using namespace parcel::lexems;
using namespace parcel::build;

namespace tools
{
    namespace dump
    {
        void lextable(offset_table<link_lex *> *table)
        {
            std::vector<pair<int, std::vector<link_lex *>>> v = table->as_list();
            printf("Dump lex table. (count=%i)\n", v.size());

            for (const pair<int, std::vector<link_lex *>> &e : v)
            {
                for (const link_lex *t : e.second)
                {
                    cout << e.first << ":" << setw(e.first) << "[" << t->val->name() << "]\n";
                }
            }
        };

        void hook(parcel::parser::token_hook *h)
        {
            token *tk = h->tk;
            queue<pair<token *, short>> q;
            q.push({tk, 0});

            printf("Dump hook '%s':\n", h->name.c_str());

            while (!q.empty())
            {
                auto cur = q.front();

                if (cur.first->val != NULL)
                {
                    cout << setw(cur.second) << "[" << nameof(cur.first->type) << "]" << " v='" << cur.first->val->str() << "'\n";
                }
                else
                {
                    printf("[%s] (%s) Not Builded!\n", h->name.c_str(), nameof(cur.first->type));
                    // break;
                }

                if (cur.first->type == type::list)
                {
                    val_list *ls = static_cast<val_list *>(cur.first->val);
                    if (ls != NULL)
                    {
                        // cout << setw(cur.second) << "[List] (";
                        for (token *n : ls->v)
                        {
                            // cout <<tk_nameof(cur.first->type) << ",";
                            q.push({n, cur.second + 3});
                        }
                    }
                }
                else if (cur.first->type == type::set)
                {
                    val_set *ls = static_cast<val_set *>(cur.first->val);
                    if (ls != NULL)
                    {
                        // cout << setw(cur.second) << "[Set] (";
                        for (token *n : ls->v)
                        {
                            // cout <<tk_nameof(cur.first->type) << ",";
                            q.push({n, cur.second + 3});
                        }
                        // cout << ")\n";
                    }
                }
                else if (cur.first->type == type::vec)
                {
                    val_vector *ls = static_cast<val_vector *>(cur.first->val);
                    if (ls != NULL)
                    {
                        // cout << setw(cur.second) << "[Vec] (";
                        for (token *n : ls->v)
                        {
                            // cout <<tk_nameof(cur.first->type) << ",";
                            q.push({n, cur.second + 3});
                        }
                        // cout << ")\n";
                    }
                }

                q.pop();
            }
        };

    }
}

#endif