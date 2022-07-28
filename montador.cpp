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

// nome da diretiva -> num_end
map<string, int> diretives;

// nome da instr -> {opcode, num_args}
map<string, pair<int, int>> opcodes;

void initialize_diretives_table();
void initialize_opcode_table();

vector<string> string_split (string input, string delim);

string clean_code(string dirty_code);
string sub_equ_if(string code);
string passage_zero(string code);

bool lexical_validation(string str);
tuple< map<string, int>, map<string, int>, map<string, vector<int>>, bool> passage_one(string code);

string int_vector2string(vector<int> int_vector);
string tables2string(map<string, int> def_table, map<string, vector<int>> use_table);
string passage_two(string code, map<string, int> symbol_table);

void assembler(string file_name, string obj_name);
void pre_process(string file_name, string new_file);
void help();


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

/* PASSAGEM 0 */
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

	for (unsigned int i = 0; i < regex_vec.size(); i++) 
		dirty_code = regex_replace(dirty_code, regex_vec[i].first, regex_vec[i].second);   

	dirty_code.erase(0, 1);
	dirty_code.erase(dirty_code.size()-1, 1);
	return dirty_code;
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

/* PASSAGEM 1 */
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

/*
	Summary: primeira passagem do algoritmo de duas passagens
		Faz a contagem dos rotulos e detecta os seguintes erros:
			Declaracoes ou rotulos repetidos
			Tokens invalidos
			Dois rotulos na mesma linha
			Instrucoes ou diretivas nas secoes erradas
			Uso de instrucoes/diretivas que nao existem
			Falta de secao de texto. 

	Input:
		string: codigo a ser avaliado (ja pre-procesado)
	Returns:
		Tripla, na seguinte ordem:
		map<string, int> tabela de simbolos
		map<string, int> tabela de definicoes
		map<string, int> tabela de uso
*/
tuple< map<string, int>, map<string, int>, map<string, vector<int>>, bool> passage_one(string code)
{
	int pos_counter = 0;
	code = regex_replace(code, regex(","), "");
	vector<string> lines = string_split(code);

	map<string, int> symbol_table;
	map<string, int> definition_table;
	map<string, vector<int>> use_table;

	map<string, bool> extern_labels;

	bool data_flag = false;
	bool text_flag = false;
	bool begin_flag = false;
	bool end_flag = false;
	
	for(unsigned int line_counter = 0; line_counter < lines.size(); line_counter++)
	{
		//Verifica se tem rotulo
		vector<string> tokens = string_split(lines[line_counter], ":");

		if (tokens.size() > 2)
		{
			cout << "Erro sintatico, duas labels na mesma linha (" << line_counter + 1 << ")" << endl;
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
			}

			if(symbol_table.count(label) > 0)
			{
				cout << "Erro semantico: rotulo " << label << " redefinido na linha " << line_counter + 1 << endl;
			}else
			{
				symbol_table[label] = pos_counter;

				vector<string> tokenized_instruction = string_split(instruction, " ");

				if(tokenized_instruction[0] == "EXTERN")
				{
					extern_labels[label] = true;
				}
			}
		}

		tokens = string_split(instruction, " ");

		if(tokens[0] == "SECAO")
		{
			if (tokens[1] == "TEXTO")
				text_flag = true;
			
			if (tokens[1] == "DADOS")
				data_flag = true;
		}

		string op = tokens[0];
		if(opcodes.count(op) > 0)
		{
			if (data_flag)
			{
				cout << "Erro semantico: instrucao na secao de dados na linha " << line_counter + 1 << endl;
			}
			
			if(tokens.size() > 1)
			{
				// Nao eh stop
				if(extern_labels.count(tokens[1]) > 0)
				{
					use_table[tokens[1]].push_back(pos_counter + 1);
				}
			}

			if(tokens.size() > 2)
			{
				// Eh copy
				if(extern_labels.count(tokens[2]) > 0)
				{
					//eh label externa, deve ir para a tabela de uso
					use_table[tokens[2]].push_back(pos_counter + 2);
				}
			}

			pos_counter += opcodes[op].second;
		}else if(diretives.count(op) > 0)
		{
			//Executar diretiva
			if ((op == "CONST" || op == "SPACE") && data_flag == false)
			{
				cout << "Erro semantico: diretivas CONST ou SPACE utilizadas fora do segmento de dados na linha " << line_counter + 1 << endl;
			}else
			if(op == "PUBLIC")
			{
				definition_table[tokens[1]] = pos_counter;
			}else
			if(op == "BEGIN")
			{
				begin_flag = true;
			}else
			if(op == "END")
			{
				end_flag = true;
			}
			pos_counter += diretives[op];
		}else
		{
			cout << "Erro sintatico: operacao " << op << " nao encontrada!" << endl;
		}
	}

	if(text_flag == false)
	{
		cout << "Erro semantico: nao existe secao de texto" << endl;
	}

	if(begin_flag == true && end_flag == false)
	{
		cout << "Erro semantico: BEGIN sem END" << endl;
	}else if(begin_flag == false && end_flag == true)
	{
		cout << "Erro semantico: END sem BEGIN" << endl;
	}

	map<string, int>::iterator it;
	for (it = definition_table.begin(); it != definition_table.end(); it++)
	{
		if(symbol_table.count(it->first) <= 0)
		{
			throw invalid_argument("A tabela de definicoes possui um simbolo que nao esta na tabela de simbolos!");
		}
		definition_table[it->first] = symbol_table[it->first];
	}

	return make_tuple(symbol_table, definition_table, use_table, begin_flag || end_flag);
}


/* PASSAGEM 2 */
string int_vector2string(vector<int> int_vector)
{
	string str = "";
	for (unsigned int i = 0; i < int_vector.size(); i++)
	{
		str += to_string(int_vector[i]) + " ";
	}
	return str;
}

string tables2string(map<string, int> def_table, map<string, vector<int>> use_table)
{
	string str = "TABELA DEF\n";
	map<string, int>::iterator it;
	for (it = def_table.begin(); it != def_table.end(); it++)
	{
		str += it->first + " " + to_string(it->second) + "\n";
	}

	str += "\nTABELA USO\n";

	map<string, vector<int>>::iterator it2;
	for (it2 = use_table.begin(); it2 != use_table.end(); it2++)
	{
		for(unsigned int i = 0; i < (it2->second).size(); i++)
		{
			str += it2->first + " " + to_string((it2->second)[i]) + "\n";
		}
	}

	return str + "\n";
}

string passage_two(string code, map<string, int> symbol_table) {
	code = regex_replace(code, regex(","), "");
	auto code_vec = string_split(code, "\n");
	vector<int> obj_code;
	for (unsigned int i = 0; i < code_vec.size(); i++) {
		auto instr_parse = string_split(code_vec[i], ":");
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
			unsigned int num_args = info.second;

			if (instr_parse.size() != num_args) {
				cout << "Erro Sintatico: numero de argumentos errados para a operacao " << instr_parse[0] << ". Linha: " << i+1 << endl;
			}

			obj_code.push_back(opcode);
			for (unsigned int j = 1; j < num_args; j++) {
				if (symbol_table.count(instr_parse[j]) == 0) {
					cout << "Erro Semantico: label " << instr_parse[j] << " nao definida. Linha: " << i+1 << endl;
				}	
				obj_code.push_back(symbol_table[instr_parse[j]]);
			}
		}

		// diretiva
		else {
			if (instr_parse[0] == "CONST") {
				int num;
				if (instr_parse[1].size() > 2 && instr_parse[1][0] == '0' && instr_parse[1][1] == 'X') 
					num = stoi(instr_parse[1], 0, 16);
				
				else
					num = stoi(instr_parse[1]);
				obj_code.push_back(num);
			}

			else if (instr_parse[0] == "SPACE")
				obj_code.push_back(0);
		}
	}
	
	return int_vector2string(obj_code);
}

/* MONTADOR */
void pre_process(string file_name, string new_file)
{
	ifstream file(file_name);

	if (file.fail())
	{
		cout << "Nao foi possivel abrir o arquivo " + file_name + ", verifique sua existencia e os privilegios do programa e tente novamente!" << endl;
		exit(1);
	}

	stringstream buffer;
	buffer << file.rdbuf();

	string str = buffer.str();

	string code = passage_zero(str);

	if(new_file == "")
	{
		new_file = "pre_processed_" + file_name + ".asm";
	}

	vector<string> aux = string_split(new_file, ".");
	
	if(aux.size() < 2)
	{
		new_file += ".asm";
	}

	std::ofstream out(new_file);
    out << code;
    out.close();
}


void assembler(string file_name, string obj_name)
{
	ifstream file(file_name);

	if (file.fail())
	{
		cout << "Nao foi possivel abrir o arquivo " + file_name + ", verifique sua existencia e os privilegios do programa e tente novamente!" << endl;
		exit(1);
	}

	stringstream buffer;
	buffer << file.rdbuf();

	string str = buffer.str();

	str = passage_zero(str);
	string tables = "";
	auto [symbol_table, definition_table, use_table, flag_table] = passage_one(str);
	
	if(flag_table)
	{
		tables = tables2string(definition_table, use_table);
	}
	
	string code = passage_two(str, symbol_table);

	string output = tables + code;

	if(obj_name == "")
	{
		obj_name = "object";
	}

	vector<string> aux = string_split(obj_name, ".");
	
	if(aux.size() < 2)
	{
		obj_name += ".obj";
	}

	std::ofstream out(obj_name);
    out << output;
    out.close();
}

void help()
{
	cout << "Exemplo de uso do programa:" << endl;
	cout << "Pre-processamento: ./montador -p codigo.asm codigo_pre_processado.asm" << endl;
	cout << "Geracao de arquivo objeto: ./montador -o codigo.asm objeto.obj" << endl;
}

int main (int argc, char** argv) 
{
	vector<string> args(argc);

	initialize_opcode_table();
	initialize_diretives_table();

	if(argc != 4)
	{
		cout << "Quantidade de parametros informados errada!\n" << endl;
		help();
		exit(1);
	}

	for (int i = 0; i < argc; i++)
	{
		args[i] = (string) argv[i];
	}

	if(args[1] == "-p")
	{
		pre_process(args[2], args[3]);
	}else if(args[1] == "-o")
	{
		assembler(args[2], args[3]);
	}else
	{
		cout << "Modo de uso " + args[1] + " nao suportado!\n" << endl;
		help();
	}

	return 0;
}


