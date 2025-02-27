#ifndef _FUNCS_
#define _FUNCS_

#include "../lang/lang.h"
#include "../lexems/lexem.h"
#include "../tools/lexer/lex.h"
#include "../tools/btree.h"
#include "../types/types.h"
#include "../expr/expr.h"
using namespace std;
using namespace parcel::type;
using namespace parcel::lexems;
using namespace parcel::lang;

namespace parcel
{
    /*
    ref
    ref_expr
    arglist
*/

    namespace funcs
    {
        struct fn_arglist
        {

            lex_type litr_type;
            string value;
            fn_arglist *next_arg;

            fn_arglist(lex_type type, string &value)
            {
                this->litr_type = type;
                this->value = value;
                this->next_arg = NULL;
            };

            ~fn_arglist()
            {
                // DEBUG_DCTOR;
                fn_arglist *head = next_arg;
                while (head != NULL)
                {
                    // DEBUG_MSG("~[fn_arglist] arg");
                    delete head;
                    head = head->next_arg;
                }
            };
        };

        struct fn_ref : public lvalue
        {
            string name;
            fn_arglist *args;

            fn_ref(string &name)
            {
                this->name = name;
                args = NULL;
            }

            fn_ref(string &name, fn_arglist *args)
            {
                this->name = name;
                args = args;
            }

            ~fn_ref() {
            };
        };

        struct fn_expr : btree<fn_ref *>, lvalue
        {
        };
    }
}

#endif