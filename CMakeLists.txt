cmake_minimum_required(VERSION 3.2)

project(bound_parsing)

include_directories(include)

add_executable(tt
    tests/client.cpp
    #src/tools/lex.h

    
    src/types/types.cpp
    src/types/types.h
    src/lang/lang.cpp
    src/lang/lang.h
    src/lexems/lexem.h
    src/expr/expr.cpp
    src/expr/expr.h
    src/funcs/fn.h
    
    src/tools/offset_table.h
    src/tools/lexer/lex.h
    src/tools/btree.h
    src/tools/logger.h
    src/builder/lex_build.cpp
    src/builder/lex_build.h

    src/tools/ring.h
    src/parse/parser.h
    src/parse/token.h

    src/tools/dump.h
)