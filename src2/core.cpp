
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "parcel.h"

using namespace std;

struct lex_graph_path {
  string name;
  vector<lex_graph_path> paths;
};

class graph_line {
  map<int, vector<lex_graph_path>> _entries;
  int _last_level;
  int _min_level;

public:
  graph_line() : _last_level(0), _min_level(6){};

  // find any path in graph by level and name
  bool find(int offset, string &entry_name, lex_graph_path &out) {
    auto fn = _entries.find(offset);
    if (fn != end(_entries)) {

      for (const auto &en : (*fn).second) {
        if (en.name == entry_name) {
          out = en;
          return true;
        }
      }
    }

    return false;
  };

  // return first paths in graph (entrypoints)
  bool head(vector<lex_graph_path> &out) {
    auto fn = _entries.find(_min_level);
    if (fn != end(_entries)) {
      out = (*fn).second;
      return true;
    }

    return false;
  }

  // return last paths in graph (most deepest)
  bool tail(vector<lex_graph_path> &out) {
    auto fn = _entries.find(_last_level);
    if (fn != end(_entries)) {
      out = (*fn).second;
      return true;
    }

    return false;
  }

  // return last path of the last added level
  bool last(lex_graph_path &out) {
    auto fn = _entries.find(_last_level);
    if (fn != end(_entries)) {
      auto vec = (*fn).second;
      if (vec.size() > 0) {
        out = vec.back();
        return true;
      }

      return false;
    }

    return false;
  };

  // return last parent path in graph
  bool parent(int offset, lex_graph_path &out) {
    auto fn = _entries.find(offset);
    if (fn != end(_entries)) {
      auto vec = (*fn).second;
      if (vec.size() > 0) {
        out = vec.back();
        return true;
      }

      return false;
    }

    return false;
  }

  void add(lex_graph_path &p, int offset) {
    auto fn = _entries.find(offset);
    if (fn != end(_entries)) {
      (*fn).second.push_back(p);
    } else {
      vector<lex_graph_path> v;
      v.push_back(p);
      _entries.insert({offset, v});
    }

    _last_level = offset;
    _min_level = min(_min_level, offset);
  };

  void dump() {
    printf("[graph_line]\n");
    for (auto &e : _entries) {
      for (auto &p : e.second) {
        printf("[graph_line].[%i] %s\n", e.first, p.name.c_str());
      }
    }
  };

  ~graph_line() { _entries.clear(); };
};

/*

    INSTR GRAPH
    [path]  [func_val]      [func_val]
    text -> func.begins ->(and) func.ends
    text -> func.begins ->(or) func.ends

    BASEBLOCKS:
        tagval
        sequence
        list
        pair
        ----
        literal
*/

// ------------------- builder



#include "graph_table.h"


// for example
graph_line *build_graph(string &src) {
  graph_line *floor = new graph_line();
  lexer lx(src);

  string cur;
  size_t line_offset = 0;

  while (lx.can_read()) {
    if (lx.next_id(cur) != lx.npos) {

      char delim;
      lx.next_symbol(delim);

      if (delim == ':') {
        // next_entry (relate previos path)
        // add path to parent path

        lex_graph_path pt;
        pt.name = string(cur.c_str());
        floor->add(pt, line_offset);

        // par.add()
        lex_graph_path _par;
        if (floor->parent(line_offset - 2, _par)) {
          printf("[g] %s . %s\n", _par.name.c_str(), cur.c_str());
          _par.paths.push_back(pt);
        } else {
          printf("[ERR] _par not found\n");
        }
      } else if (delim == '|') {

        lex_graph_path pt;
        pt.name = string(cur.c_str());
        floor->add(pt, line_offset);

        // prev().add()
        lex_graph_path _tag;
        if (floor->last(_tag)) {
          printf("[g] %s . %s\n", _tag.name.c_str(), cur.c_str());
          _tag.paths.push_back(pt);
        } else {
          printf("[ERR] _tag not found\n");
        }
      } else {

        lx.go_back(-1);
        // printf("[val] (%s) %s\n", par.name.c_str(), cur.c_str());

        // next tag started.
        // add to paths in parent
        // prev().add()
        lex_graph_path pt;
        pt.name = string(cur.c_str());

        lex_graph_path _tag;
        if (floor->last(_tag)) {
          printf("[g] %s . %s\n", _tag.name.c_str(), cur.c_str());
          _tag.paths.push_back(pt);
        } else {
          printf("[ERR] _tag not found\n");
        }
      }
    } else {
      char delim;
      lx.next_symbol(delim);
      if (delim == '\n') {
        // this is signal to next line
        // we can left padding size of next level
        line_offset = lx.skip(" \t");
      }
    }
  }

  floor->dump();

  return floor;
}

#include <sstream>

string file_read_all(const char *path) {
  ifstream fs(path);
  if (fs.is_open()) {
    ostringstream ss;
    ss << fs.rdbuf();
    return ss.str();
  } else {
    cout << "file bad open\n";
  }
}

int main() {

  // string sr =
  // file_read_all("C:/git.local/parsing/bound_parsing/src/fmt2.yml");
  // printf("s=%s\n", sr.c_str());
  // build_lex_graph(sr);


  // string s1 = "'a' appe";
  string s1 = " \"apple\" good";
  string s2 = "tagval: tag: literal val: lit_word|lit_number";
  lexer lx(s1);
  string cur;
  cout << lx.next_like_rounded(cur, "\"", "\"", "") << endl;
  cout << cur.c_str() << endl;
  // get_literals();
}

/*

    # check/build (block)
    tagval:
    1) tagval
    2) tag,val

    str = pick()
    -> map.find(st)r


    BUILDING LEX GRAPH
    g:
        T? val
        entries<GraphPath>

    path
    -name
    -type
    -entries

    [lex_graph]
        building simple objects from source yml file.
        build:
             hooks
             funcs
             blocks
             sequences
    ---

    [format_graph]
        building a data structure for parsing.
        data parse distribution for collect data.

        actions:
            1) build funcs with args
            2) build basic blocks
            3) get hooks
            4) get vars?
    ---

    [value_graph]
        parse source text files and collect data by provided format graph
        acts:
            get values by rules/collect.

*/