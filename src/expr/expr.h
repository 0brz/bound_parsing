#ifndef _EXPR_
#define _EXPR_

#include "../tools/lexer/lex.h"
#include "../lang/lang.h"
#include <stack>

namespace parcel
{
    namespace expr
    {
        size_t get_logic_entry(lexer &lx);
        bool to_postfix(lexer &lx, stack<string> &call_stack);
    }
}

#endif