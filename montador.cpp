#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <map>
// #include <list>

using namespace std;

// nome da instr -> {opcode, num_args}
map<string, pair<int, int>> opcodes;

void initialize_opcode_table() {
	opcodes["ADD"] = 		{1, 2};
	opcodes["SUB"] = 		{2, 2};
	opcodes["MUL"] = 		{3, 2};
	opcodes["DIV"] = 		{4, 2};
	opcodes["JMP"] = 		{5, 2};
	opcodes["JMPN"] = 	{6, 2};
	opcodes["JMPP"] = 	{7, 2};
	opcodes["JMPZ"] = 	{8, 2};
	opcodes["COPY"] = 	{9, 3};
	opcodes["LOAD"] = 	{10, 2};
	opcodes["STORE"] = 	{11, 2};
	opcodes["INPUT"] = 	{12, 2};
	opcodes["OUTPUT"] = {13, 2};
	opcodes["STOP"] = 	{14, 1};
}

string clean_code(string dirty_code) {
	dirty_code.insert(0, "\n");
	dirty_code.append("\n");

	vector<pair<regex, string>> regex_vec;

	regex_vec.push_back({regex("\r") , "\n"}); // formata strings do linux
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

string sub_equ_if(string code){
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

string passage_zero(string code) {
	code = clean_code(code);
	transform(code.begin(), code.end(), code.begin(), ::toupper); // bota tudo em caso superior
	return sub_equ_if(code);
	
}

vector<int> passage_two(string code, map<string, int> symbol_table) {
	auto code_vec = string_split(code, "\n");
	vector<int> obj_code;
	for (int i = 0; i < code_vec.size(); i++) {
		auto instr_parse = string_split(code, ":");
		string temp;
		if (instr_parse.size() > 1) 
			temp = instr_parse[1];
		
		else
			temp = instr_parse[0];

		if (temp[0] == ' ')
			temp.erase(0, 1);

		instr_parse = string_split(temp, " ");

		if (opcodes.count(instr_parse[0]) > 0) {

			auto info = opcodes[instr_parse[0]];
			int opcode = info.first;
			int num_args = info.second;

			if (instr_parse.size() != num_args) {
				cout << "Erro Sintatico: numero de argumentos errados para a operacao " << instr_parse[0] << endl;
				throw invalid_argument("Erro Sintatico");
			}

			obj_code.push_back(opcode);
			for (int i = 1; i < num_args; i++) {
				if (symbol_table.count(instr_parse[i]) == 0) {
					cout << "Erro Semantico: label " << instr_parse[i] << " nao definida" << endl;
					throw invalid_argument("Erro Semantico");
				}	
				obj_code.push_back(symbol_table[instr_parse[i]]);
			}
		}

		// diretiva
		else {
			if (instr_parse[0] == "CONST")
				obj_code.push_back(stoi(instr_parse[1]));

			else if (instr_parse[0] == "SPACE")
				obj_code.push_back(0);
		}
	}
	return obj_code;
}

int main () {
	string str;
	ifstream file("test/equ_if_test1.asm");

	stringstream buffer;
	buffer << file.rdbuf();

	str = buffer.str();

	str = passage_zero(str);

	initialize_opcode_table();

	cout << str << endl;
	return 0;
}
