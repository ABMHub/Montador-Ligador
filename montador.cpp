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

// nome da diretiva -> num_end
map<string, int> diretives;

void initialize_diretives_table()
{
	diretives["BEGIN"]  = 0;
	diretives["END"]    = 0;
	diretives["EXTERN"] = 0;
	diretives["PUBLIC"] = 0;
	diretives["SECAO"]  = 0;
	diretives["SPACE"]  = 1;
	diretives["CONST"]  = 1;
}

// nome da instr -> {opcode, num_args}
map<string, pair<int, int>> opcodes;

void initialize_opcode_table() {
    opcodes["ADD"] =         {1, 2};
    opcodes["SUB"] =         {2, 2};
    opcodes["MUL"] =         {3, 2};
    opcodes["DIV"] =         {4, 2};
    opcodes["JMP"] =         {5, 2};
    opcodes["JMPN"] =     {6, 2};
    opcodes["JMPP"] =     {7, 2};
    opcodes["JMPZ"] =     {8, 2};
    opcodes["COPY"] =     {9, 3};
    opcodes["LOAD"] =     {10, 2};
    opcodes["STORE"] =     {11, 2};
    opcodes["INPUT"] =     {12, 2};
    opcodes["OUTPUT"] = {13, 2};
    opcodes["STOP"] =     {14, 1};
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

/*
	Summary: validacao lexica seguindo as regras da linguagem C
	Input:
		string: objeto de avaliacao
	Returns:
		True se a entrada for lexicamente INVALIDA e False caso contrario
*/
bool lexical_validation(string str)
{
	regex regex_exp ("^([_A-Z][_A-Z0-9]*)");   // matches words beginning by "sub"
	return ! regex_match (str, regex_exp);
}

map<string, int> passage_one(string code)
{
	int pos_counter = 0;
	vector<string> lines = string_split(code);
	map<string, int> symbol_table;
	
	bool data_flag = false;
	bool text_flag = false;
	
	for(unsigned int line_counter = 0; line_counter < lines.size(); line_counter++)
	{
		//Verifica se tem rotulo
		vector<string> tokens = string_split(lines[line_counter], ":");

		if (tokens.size() > 2)
		{
			cout << "Erro sintatico, duas labels na mesma linha (" << line_counter + 1 << ")" << endl;
			throw invalid_argument("Erro Sintatico");
		}

		string instruction = lines[line_counter];
		
		if (tokens.size() > 1)	//Tem rotulo
		{
			string label = tokens[0];

			if (tokens[1][0] == ' ')
				tokens[1].erase(0, 1);

			instruction = tokens[1];

			if(lexical_validation(label))
			{
				cout << "Erro lexico: rotulo " << label << " invalido, verifique as regras de formacao na linha " << line_counter + 1 << endl;
				throw invalid_argument("Erro lexico");
			}

			if(symbol_table.count(label) > 0)
			{
				cout << "Erro semantico: rotulo " << label << " redefinido na linha " << line_counter + 1 << endl;
				throw invalid_argument("Erro semantico");  
			}else
			{
				symbol_table[label] = pos_counter;
			}
		}

		tokens = string_split(instruction, " ");
		string op = tokens[0];

		if(op == "SECAO")
		{
			if (tokens[1] == "TEXTO")
				text_flag = true;
			
			if (tokens[1] == "DADOS")
				data_flag = true;
		}

		if(opcodes.count(op) > 0)
		{
			if (data_flag)
			{
				cout << "Erro semantico: instrucao na secao de dados na linha " << line_counter + 1 << endl;
				throw("Erro semantico");
			}

			pos_counter += opcodes[op].second;
		}else if(diretives.count(op) > 0)
		{
			//Executar diretiva
			if ((op == "CONST" || op == "SPACE") && data_flag == false)
			{
				cout << "Erro semantico: diretivas CONST ou SPACE utilizadas fora do segmento de dados na linha " << line_counter + 1 << endl;
				throw("Erro semantico");
			}
			pos_counter += diretives[op];
		}else
		{
			cout << "Erro sintatico: operacao " << op << " nao encontrada!" << endl;
			throw("Erro sintatico");
		}
	}

	if(text_flag == false)
	{
		cout << "Erro semantico: nao existe secao de texto" << endl;
		throw("Erro semantico");
	}

	return symbol_table;
}

void run(string path)
{
	cout << "--->>>>> " << path << endl;
	string str;
	ifstream file(path);

	stringstream buffer;
	buffer << file.rdbuf();

	str = buffer.str();

	str = passage_zero(str);

	cout << str << endl;

	map<string, int> symbol_table = passage_one(str);

	cout << "\nTabela de simbolos:" << endl;
	map<string, int>::iterator it;
	for (it = symbol_table.begin(); it != symbol_table.end(); it++)
	{
		cout << it->first << ": " << + it->second << endl;
	}
}

void batch_run()
{
	//run("test/passage_one_test2_error.asm");
	//run("test/passage_one_test3_error.asm");
	//run("test/passage_one_test4_error.asm");
	//run("test/passage_one_test5_error.asm");
	//run("test/passage_one_test6_error.asm");
	run("test/passage_one_test7_error.asm");

}

int main () {
	initialize_opcode_table();
	initialize_diretives_table();

	batch_run();

	// string str;
	// ifstream file("test/passage_one_test1.asm");

	// stringstream buffer;
	// buffer << file.rdbuf();

	// str = buffer.str();

	// str = passage_zero(str);

	// cout << str << endl;

	// map<string, int> symbol_table = passage_one(str);

	// cout << "\n\nTabela de simbolos:" << endl;
	// map<string, int>::iterator it;
	// for (it = symbol_table.begin(); it != symbol_table.end(); it++)
	// {
	// 	cout << it->first << ": " << + it->second << endl;
	// }

	// cout << "Deve dar False: " << endl;
	// cout << lexical_validation("_TEMPO") << endl;
	// cout << lexical_validation("_") << endl;
	// cout << lexical_validation("L1____") << endl;
	// cout << lexical_validation("SUB_ZERO212") << endl;
	// cout << lexical_validation("LOOP") << endl;
	// cout << lexical_validation("UWUW1111") << endl;
	// cout << lexical_validation("JOAO____PEDROFELIXD745144654EALMEIDA") << endl;
	// cout << lexical_validation("LU14CAS_DE_ALMEIDA_ARRRRRRRRR556RR") << endl;
	// cout << "\nDeve dar True: " << endl;
	// cout << lexical_validation("9522JOAOPEDRO") << endl;
	// cout << lexical_validation("$9_45") << endl;
	// cout << lexical_validation("9") << endl;
	// cout << lexical_validation("ARR@AZZZ") << endl;
	// cout << lexical_validation("TEMPOR$%_93") << endl;
	// cout << lexical_validation("_%4") << endl;


	return 0;
}
