#ifndef _LEX_BUILD_H_
#define _LEX_BUILD_H_

#include "../lang/lang.h"
#include "../lexems/lexem.h"
#include "../tools/lexer/lex.h"
#include "../tools/offset_table.h"
#include "../types/types.h"
#include "../expr/expr.h"
#include "../funcs/fn.h"

using namespace std;
using namespace parcel::type;
using namespace parcel::lexems;
using namespace parcel::lang;
using namespace parcel::expr;
using namespace parcel::funcs;

namespace parcel
{
    namespace build
    {
        struct link_lex
        {
            lex *val;
            std::vector<link_lex *> entries; // maybe fix to small_vec
            link_lex(lex *v) : val(v), entries{} {};
            link_lex() : val(NULL) {};
        };

        lex *inplace_build_tag(lexer &lx);
        lex *inplace_build_basetype(lexer &lx);
        lex *inplace_build_hook_ref(lexer &lx);
        lex *inplace_build_hook_def(lexer &lx);
        lex *inplace_build_link_def(lexer &lx);
        // lex *inplace_build_link_ref(lexer &lx);
        lex *inplace_build_literal(lexer &lx);
        lex *inplace_build_fn_expr(lexer &lx);
        lex *inplace_build_fn_ref(lexer &lx);

        offset_table<link_lex *> *build_lex_table(string &src);
    }
}

#endif