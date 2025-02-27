#ifndef _LEXEMS_
#define _LEXEMS_

#include "../types/types.h"
#include <vector>

using namespace parcel::type;

namespace parcel
{
    /*
    All lexems. all descriptions
    */

    namespace lexems
    {
        struct lvalue
        {
        };

        struct lex
        {
            lvalue *value;
            lex_type type;

            inline const char *name() { return nameof(type); };
            lex(lex_type type, lvalue *val) : type(type), value(val) {};
        };

        template <typename ValueType, lex_type LexType>
        struct literal_basic_value : public lvalue
        {
            ValueType value;

            literal_basic_value(ValueType &vl)
            {
                this->value = vl;
            };

            ~literal_basic_value() {
                // DEBUG_DCTOR;
            };
        };

        using value_litr_char = literal_basic_value<char, LITR_CHAR>;
        using value_litr_string = literal_basic_value<string, LITR_STR>;
        using value_litr_int = literal_basic_value<int, LITR_INT>;
        using value_litr_float = literal_basic_value<float, LITR_FLOAT>;

        struct hook_def : public lvalue
        {
            string name;

            hook_def(string &name)
            {
                this->name = name;
            }

            ~hook_def() {
            };
        };

        struct hook_ref : public lvalue
        {
            string name;

            hook_ref(string &name)
            {
                this->name = name;
            }

            ~hook_ref() {
            };
        };

        struct link_def : public lvalue
        {
            string name;

            link_def(string &name)
            {
                this->name = name;
            }

            ~link_def() {
            };
        };

        struct link_ref : public lvalue
        {
            string name;

            link_ref(string &name)
            {
                this->name = name;
            }

            ~link_ref() {
            };
        };

    }
}

#endif