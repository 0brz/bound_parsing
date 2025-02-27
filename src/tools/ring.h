#ifndef _TOOLS_RING_H
#define _TOOLS_RING_H

template <typename RingType>
class ring
{
private:
    size_t size;
    size_t cursor;
    vector<RingType> row;
    bool self_closed() { return size <= 0; };

public:
    // clear with tokens
    // template<...Params> void for_each(){}; // <T, Op, Params...>
    ring()
    {
        size = 0;
        cursor = 0;
    }
    ring(vector<RingType> &ring_elems)
    {
        for (RingType &t : ring_elems)
        {
            row.push_back(t);
        }

        size = ring_elems.size();
        cursor = 0;
    };
    RingType get_head() { return row[0]; };
    RingType get_current() { return row[cursor]; };
    size_t len() { return size; };
    size_t pos() { return cursor; };
    void to_begin() { cursor = 0; };
    void to_next()
    {
        if (!self_closed())
        {
            if (cursor + 1 < size)
                cursor++;
            else
                cursor = 0;
        }
        else
            cursor = 0;
    };
    void add_end(RingType &el)
    {
        row.push_back(el);
        size++;
    };
    void pop_end()
    {
        if (!self_closed())
        {

            if (cursor == size - 1)
                cursor--;
            row.pop_back();
            size--;
        }
    };
    const vector<RingType> &as_row()
    {
        return row;
    };
};

#endif