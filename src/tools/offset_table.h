#ifndef _OFFSET_TABLE_
#define _OFFSET_TABLE_

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <type_traits>

using namespace std;

template <typename Element>
class offset_table
{
private:
    map<int, vector<Element>> _entries;
    int _last_level;
    int _min_level;
    int _diff;
    int _max_level;

public:
    // return first paths in graph (entrypoints)
    vector<Element> get_roots()
    {
        std::vector<Element> v;
        auto it = _entries.find(_min_level);
        if (it != end(_entries))
        {
            for (const Element &child : (*it).second)
            {
                v.push_back(child);
            }
        }

        return v;
    };

    vector<pair<int, vector<Element>>> as_list()
    {
        std::vector<pair<int, std::vector<Element>>> v;

        int offset = _min_level;
        while (offset <= _max_level)
        {
            auto it = _entries.find(offset);
            if (it != end(_entries))
            {
                std::vector<Element> level{};
                for (const Element &child : (*it).second)
                {
                    level.push_back(child);
                }

                pair<int, std::vector<Element>> cur{(*it).first, level};
                v.push_back(cur);
            }

            offset += _diff;
        }

        return v;
    };

    bool head(vector<Element> &out)
    {
        auto fn = _entries.find(_min_level);
        if (fn != end(_entries))
        {
            out = (*fn).second;
            return true;
        }

        return false;
    };

    // return last paths in graph (most deepest)
    bool tail(vector<Element> &out)
    {
        auto fn = _entries.find(_last_level);
        if (fn != end(_entries))
        {
            out = (*fn).second;
            return true;
        }

        return false;
    };

    // return last path of the last added level
    bool last(Element &out)
    {
        auto fn = _entries.find(_last_level);
        if (fn != end(_entries))
        {
            auto vec = (*fn).second;
            if (vec.size() > 0)
            {
                out = vec.back();
                return true;
            }

            return false;
        }

        return false;
    };

    // return last parent path in graph
    bool parent(int offset, Element &out)
    {
        auto fn = _entries.find(offset);
        if (fn != end(_entries))
        {
            auto vec = (*fn).second;
            if (vec.size() > 0)
            {
                out = vec.back();
                return true;
            }

            return false;
        }

        return false;
    };

    void add(Element el, int offset)
    {
        auto fn = _entries.find(offset);
        if (fn != end(_entries))
        {
            (*fn).second.push_back(el);
        }
        else
        {
            vector<Element> v;
            v.push_back(el);
            _entries.insert({offset, v});
        }

        _last_level = offset;
        _max_level = max(_max_level, offset);
        _min_level = min(_min_level, offset);
    };

    offset_table() : _last_level(0), _min_level(6), _diff(2), _max_level(-1) {};

    ~offset_table()
    {
        for (auto &entry : _entries)
        {
            for (auto &ge : entry.second)
            {
                if constexpr (is_pointer<Element>::value)
                {
                    delete ge;
                }
            }
        }
    };
};

#endif