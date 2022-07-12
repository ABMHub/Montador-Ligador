#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <stdexcept>
// #include <list>

using namespace std;

string clean_code(string dirty_code) {
  dirty_code.insert(0, "\n");
  dirty_code.append("\n");

  vector<pair<regex, string>> regex_vec;

  regex_vec.push_back({regex("\r") , "\n"}); // remove comentarios
  regex_vec.push_back({regex("[\n ]*;(.*?)[\n]") , "\n"}); // remove comentarios
  regex_vec.push_back({regex(" +")               , " " }); // remove espacos redundantes
  regex_vec.push_back({regex(" ?\n+ ?")          , "\n"}); 
  regex_vec.push_back({regex("\n+")              , "\n"});
  regex_vec.push_back({regex(": ?\n")            , ": "}); // coloca labels na mesma linha
  // regex_vec.push_back({regex(" ")                , "_"}); // debug

  for (unsigned int i = 0; i < regex_vec.size(); i++) 
    dirty_code = regex_replace(dirty_code, regex_vec[i].first, regex_vec[i].second);   

  dirty_code.erase(0, 1);
  dirty_code.erase(dirty_code.size()-1, 1);
  return dirty_code;
}

vector<string> string_split (string input, string delim = "\n")
{
    int len = input.length();
    char cmd[len + 1];
    strcpy(cmd, input.c_str());

    len = delim.length();
    char delimiter[len + 1];
    strcpy(delimiter, delim.c_str());

	char *ptr = strtok(cmd, delimiter);

	vector<string> inst;

  while(ptr != NULL)
  {
      string tmp = ptr;
      inst.push_back(tmp);
      ptr = strtok(NULL, delimiter);
  }

  return inst;
}

string passage_zero(string code) {
  code = clean_code(code);
  transform(code.begin(), code.end(), code.begin(), ::toupper); // bota tudo em caso superior
  
  vector<string> instructions = string_split(code);
  
  map<string, string> equ_labels;

  vector<string> new_instructions;

  for(unsigned int i = 0; i < instructions.size(); i++)
  {
    vector<string> tokens = string_split(instructions[i], " ");
    
    if(regex_match(instructions[i], regex(".* EQU .*")))
    {
      if(tokens.size() != 3)
      {
        cout << "EQU utilizado incorretamente na linha " << i << endl;
        throw invalid_argument("Erro de pre-processamento"); 
      }else
      {
        tokens[0].erase(tokens[0].size()-1, 1);
        equ_labels[tokens[0]] = tokens[2];
      }
    }
    else if(regex_match(instructions[i], regex("IF .*")))
    {
    
      if(tokens.size() != 2)
      {
        cout << "IF utilizado incorretamente na linha " << i << endl;
        throw invalid_argument("Erro de pre-processamento");
      }else{
        if(equ_labels[tokens[1]] == "0")            
        {
          //IF falhou, pular a proxima instrucao
          i+=1;
        }
      }
    }else
    {
      new_instructions.push_back(instructions[i]);
    }
  }

  string new_code = "";
  for (auto el: new_instructions)
  {
    new_code += el + "\n";
  }



  map<string, string>::iterator it;
  for (it = equ_labels.begin(); it != equ_labels.end(); it++)
  {
    new_code = regex_replace(new_code, regex( (" " + it->first) ), (" " + it->second) );
  }

  return new_code;
}

int main () {
  string str;
  ifstream file("test/equ_if_test1.asm");

  stringstream buffer;
  buffer << file.rdbuf();

  str = buffer.str();

  str = passage_zero(str);

  cout << str << endl;
  return 0;
}
