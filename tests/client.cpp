#include <iostream>

#define PARCEL_DEV
#include <../include/parcel.h>
#include <../include/tools.h>
#include <../src/builder/lex_build.h>
#include <../src/tools/lexer/lex.h>
#include <../src/tools/dump.h>
#include <../src/builder/parse_build.h>

int main()
{
    printf("Client code...\n");
    string sr = lexer::read_source_file("C:/git.local/parsing/bound_parsing/tests/lang.yml");
    offset_table<parcel::build::link_lex *> *gt = parcel::build::build_lex_table(sr);
    if (gt != NULL)
    {
        tools::dump::lextable(gt);

        instr ins;
        ins.build(gt);

        // working with tokenizer.
        std::vector<string> toks{
            "13", "w1", "w2", "w3", "1235"};
        for (string &s : toks)
        {
            ins.propagate(s);
        }

        printf("propagate=ok\n");
        parcel::parser::token_hook *k1 = ins.find_hook("ex1");
        if (k1 != NULL)
        {
            tools::dump::hook(k1);
        }

        delete gt;
    }
}