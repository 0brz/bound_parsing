#ifndef _PARSE_TOKEN_H_
#define _PARSE_TOKEN_H_

#include <map>
#include <string>
#include <vector>

using namespace std;

namespace parcel
{
    namespace tokens
    {
        enum type
        {
            word,
            number,
            list,
            vec,
            set,
            literal_string,
            literal_char
        };

        map<type, const char *> token_names{
            {word, "word"},
            {number, "number"},
            {list, "list"},
            {vec, "vec"},
            {set, "set"},
            {literal_string, "litr.string"}};

        const char *nameof(type type)
        {
            auto fnd = token_names.find(type);
            if (fnd != end(token_names))
            {
                return (*fnd).second;
            }
            else
                return "<null>";
        };

        // struct token;

        struct tvalue
        {
            virtual const char *str() = 0;
        };

        class token
        {
        public:
            type type;
            tvalue *val;
            string *dbg_val;
            // str?
        };

        struct val_word : tvalue
        {
            string v;
            val_word(string &v) : v(v) {};
            const char *str()
            {
                return v.c_str();
            }
        };

        struct val_string : tvalue
        {
            string v;
            val_string(string &v) : v(v) {};
            const char *str()
            {
                return v.c_str();
            }
        };

        struct val_char : tvalue
        {
            char v;
            val_char(char v) : v(v) {};
            const char *str()
            {
                return "<char>";
            }
        };

        struct val_num : tvalue
        {
            string v;
            val_num(string &v) : v(v) {};
            const char *str()
            {
                // string s = std::to_string(v);
                return v.c_str();
            }
        };

        struct val_list : tvalue
        {
            std::vector<token *> v;
            val_list(std::vector<token *> &v) : v(v) {};
            val_list() {};
            const char *str()
            {
                return "<list>";
            }
        };

        struct val_vector : tvalue
        {
            std::vector<token *> v;
            val_vector(std::vector<token *> &v) : v(v) {};
            const char *str()
            {
                return "<vec>";
            }
        };

        struct val_set : tvalue
        {
            std::vector<token *> v;
            val_set(std::vector<token *> &v) : v(v) {};
            val_set() {};
            const char *str()
            {
                return "<set>";
            }
        };
    }
}

#endif