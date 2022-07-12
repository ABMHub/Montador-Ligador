#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <cstring>
#include <algorithm>
// #include <list>

using namespace std;

string clean_code(string dirty_code) {
  dirty_code.insert(0, "\n");
  dirty_code.append("\n");

  vector<pair<regex, string>> regex_vec;

  regex_vec.push_back({regex("[\n ]*;(.*?)[\n]") , "\n"}); // remove comentarios
  regex_vec.push_back({regex(" +")               , " " }); // remove espacos redundantes
  regex_vec.push_back({regex(" ?\n+ ?")          , "\n"}); 
  regex_vec.push_back({regex("\n+")              , "\n"});
  regex_vec.push_back({regex(": ?\n")            , ": "}); // coloca labels na mesma linha
  // regex_vec.push_back({regex(" ")                , "_"}); // debug

  for (int i = 0; i < regex_vec.size(); i++) 
    dirty_code = regex_replace(dirty_code, regex_vec[i].first, regex_vec[i].second);   

  dirty_code.erase(0, 1);
  dirty_code.erase(dirty_code.size()-1, 1);
  return dirty_code;
}

string passage_zero(string code) {
  code = clean_code(code);
  transform(code.begin(), code.end(), code.begin(), ::toupper); // bota tudo em caso superior
  return code;
}

int main () {
  string str;
  ifstream file("test/fat_modA.asm");

  stringstream buffer;
  buffer << file.rdbuf();

  str = buffer.str();
  str = passage_zero(str);

  cout << str << endl;
  return 0;
}
