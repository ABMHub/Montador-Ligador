#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <cstring>

using namespace std;

map<int, int> opcodes;

void initialize_opcode_table() {
	opcodes[1] =  2;
	opcodes[2] =  2;
	opcodes[3] =  2;
	opcodes[4] =  2;
	opcodes[5] =  2;
	opcodes[6] =  2;
	opcodes[7] =  2;
	opcodes[8] =  2;
	opcodes[9] =  3;
	opcodes[10] = 2;
	opcodes[11] = 2;
	opcodes[12] = 2;
	opcodes[13] = 2;
	opcodes[14] = 1;
}

struct objeto {
  vector<int> obj;
  map<string, int> tabela_uso;
  map<string, int> tabela_def;
  set<int> relative_addr;
  int fator_correcao;
};

string readfile(string path) {
  string str;
	ifstream file(path);

	stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}

vector<string> string_split (string input, string delim = " ")
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

vector<int> parse_object(string str) {
  vector<string> obj_vec = string_split(str, " ");
  vector<int> obj_int;
  for (int i = 0; i < obj_vec.size(); i++) 
    obj_int.push_back(stoi(obj_vec[i]));
  
  return obj_int;
}

objeto parse_module(string str) {
  vector<string> obj_vec = string_split(str, "\n");
  bool t_uso = false;
  bool t_def = false;

  map<string, int> tabela_uso;
  map<string, int> tabela_def;
  vector<int> object;

  for (int i = 0; i < obj_vec.size(); i++) {
    if (obj_vec[i] == "TABELA USO") {
      t_uso = true;
      t_def = false;
    }

    else if (obj_vec[i] == "TABELA DEF") {
      t_uso = false;
      t_def = true;
    }

    else if (obj_vec[i] == "") {
      t_uso = false;
      t_def = false;
    }

    else if (t_uso || t_def) {
      vector<string> row = string_split(obj_vec[i], " ");
      string label = row[0];
      int address = stoi(row[1]);
      if (t_uso)
        tabela_uso[label] = address;

      else
        tabela_def[label] = address;
    }
    
    else {
      object = parse_object(obj_vec[i]);
    }
  }

  objeto ret;
  ret.obj = object;
  ret.tabela_def = tabela_def;
  ret.tabela_uso = tabela_uso;

  return ret;
}

map<string, int> create_global_table(objeto a, objeto b){
  map<string, int> ret;
  vector<objeto> {a, b};
  for (auto it = a.tabela_def.begin(); it != a.tabela_def.end(); ++it) {
    string label = it->first;
    int add = it->second + a.fator_correcao;
    ret[label] = add;
  }
}

set<int> create_object_table(vector<int> obj) {
  set<int> relative_addr;
  for (int i = 0; i < obj.size(); i++) {
    int n = opcodes[obj[i]];
    for (i += 1; i < n; i++) 
      relative_addr.insert(obj[i]);
    
    i--;
  }
}

vector<int> fix_addr(objeto data, map<string, int> tab_global) {
  cout << "entrou" << endl;
  for (auto it = data.tabela_uso.begin(); it != data.tabela_uso.end(); ++it) {
    string label_uso = it->first;
    int addr_uso = it->second;
    int value_global = tab_global[label_uso];

    data.obj[addr_uso] += value_global;
    data.relative_addr.erase(addr_uso);
  }
  cout << "entrou" << endl;
  for (auto it = data.relative_addr.begin(); it != data.relative_addr.end(); ++it) 
    data.obj[*it] += data.fator_correcao;
  
  cout << "entrou" << endl;
  return data.obj;
}

void print_result(vector<int> result) {
  for (int i = 0; i < result.size(); i++) {
    cout << result[i] << " ";
  }
  cout << endl;
}

int main () {
  vector<string> paths = {"test/ligador_teste1_A.o", "test/ligador_teste2_B.o"};
  pair<objeto, objeto> objects;

  string path = paths[0];
  string file_str = readfile(path);
  objects.first = parse_module(file_str);

  path = paths[1];
  file_str = readfile(path);
  objects.first = parse_module(file_str);
  
  cout << "Leu arquivos" << endl;

  objects.first.fator_correcao = 0;
  objects.second.fator_correcao = objects.first.obj.size();


  map<string, int> tab_global = create_global_table(objects.first, objects.second);

  objects.first.relative_addr = create_object_table(objects.first.obj);
  objects.second.relative_addr = create_object_table(objects.second.obj);

  cout << "a" << endl;

  vector<int> final_obj1 = fix_addr(objects.first, tab_global);
  vector<int> final_obj2 = fix_addr(objects.second, tab_global);

  final_obj1.insert(final_obj1.end(), final_obj2.begin(), final_obj2.end());
  
  print_result(final_obj1);
  return 0;
}