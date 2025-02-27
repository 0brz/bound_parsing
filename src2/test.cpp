
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "parcel.h"
#include "builder.h"

using namespace std;
using namespace lex;

int main()
{

  string sr = lexer::read_source_file("C:/git.local/parsing/bound_parsing/src/data/fmt3.yml");
  graph_table<graph_block *> *gt = builder::build_lex_graph(sr);
  if (gt != NULL)
  {
    delete gt;
  }
  // file_read_all("C:/git.local/parsing/bound_parsing/src/fmt2.yml");
  // printf("s=%s\n", sr.c_str());
  // build_lex_graph(sr);

  /*
// string s1 = "'a' appe";
string s1 = " \"apple\" good";
string s2 = "tagval: tag: literal val: lit_word|lit_number";
lexer lx(s1);
string cur;
cout << lx.next_like_rounded(cur, "\"", "\"", "") << endl;
cout << cur.c_str() << endl;
// get_literals();
*/
}