#ifndef _GRAPH_TABLE_
#define _GRAPH_TABLE_

#include "./parcel.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <type_traits>

using namespace std;

template <typename GraphElement>
class graph_table
{
private:
    map<int, vector<GraphElement>> _entries;
    int _last_level;
    int _min_level;

public:
    // return first paths in graph (entrypoints)
    bool head(vector<GraphElement> &out)
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
    bool tail(vector<GraphElement> &out)
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
    bool last(GraphElement &out)
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
    bool parent(int offset, GraphElement &out)
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

    void add(GraphElement el, int offset)
    {
        auto fn = _entries.find(offset);
        if (fn != end(_entries))
        {
            (*fn).second.push_back(el);
        }
        else
        {
            vector<GraphElement> v;
            v.push_back(el);
            _entries.insert({offset, v});
        }

        _last_level = offset;
        _min_level = min(_min_level, offset);
    };

    graph_table() : _last_level(0), _min_level(6) {};

    ~graph_table()
    {
        DEBUG_DCTOR
        for (auto &entry : _entries)
        {
            for (auto &ge : entry.second)
            {
                // cout << ge;
                if constexpr (is_pointer<GraphElement>::value)
                {
                    delete ge;
                }
            }
        }
    };
};

#endif