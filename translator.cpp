#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <cctype>
#include <algorithm>

const std::map<std::string, char> ULA_Map = {
	{"nA",     '0'}, {"AoBn",   '1'}, {"nAeB",   '2'}, {"zeroL",  '3'},
	{"AeBn",   '4'}, {"nB",     '5'}, {"AxB",    '6'}, {"AenB",   '7'},
	{"nAoB",   '8'}, {"AxBn",   '9'}, {"copiaB", 'A'}, {"AeB",    'B'},
	{"umL",    'C'}, {"AonB",   'D'}, {"AoB",    'E'}, {"copiaA", 'F'}
};

void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}
void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

void trimLine(std::string &line){
	rtrim(line);
	ltrim(line);
}

char itoh(int i){
	if (i >= 0 && i <= 9) return '0' + i;
	if (i >= 10 && i <= 15) return 'A' + (i - 10);
	return '0';
}

int main(){
	const std::string inputFile = "TESTEULA.ula";
	const std::string outputFile= "TESTEULA.hex";

	std::ifstream input(inputFile);
	if (!input.is_open()) return 1;

	std::ofstream output(outputFile);
	if (!output.is_open()) return 1;

	std::cout << "Starting translation" << std::endl;

	std::string line;
	size_t lineNum = 0;

	uint8_t x = 0, y = 0, instructionCount = 0;

	while(std::getline(input, line)){
		lineNum++;
		trimLine(line);

		if(line.empty()){
			std::cout << lineNum << " is blank" << std::endl;
			continue;
		}
		if (line == "inicio:" || line == "fim.") continue;

		if (line[0] == 'X'){
			try{
				size_t semicolonPos = line.rfind(';');
				std::string numStr = line.substr(2, semicolonPos - 2);
				x = std::stoi(numStr);
			}catch (...){
				std::cout << "Syntax error on line " << lineNum << std::endl;
			}
		}else if(line[0] == 'Y'){
			try{
				size_t semicolonPos = line.rfind(';');
				std::string numStr = line.substr(2, semicolonPos - 2);
				y = std::stoi(numStr);
			}catch(...){
				std::cout << "Syntax error on line " << lineNum << std::endl;
			}
		}else if(line[0] == 'W'){
			size_t semicolonPos = line.rfind(';');
			std::string mnemonic = line.substr(2, semicolonPos - 2);
			auto it = ULA_Map.find(mnemonic);

			if(it == ULA_Map.end()){
				std::cout << "Weird mnemonic in line " << lineNum << std::endl;
				continue;
			}

			char charX = itoh(x);
			char charY = itoh(y);
			char charW = it->second;

			output << charX << charY << charW << " ";
			instructionCount++;
		}
	}
	input.close();
	output.close();
	std::cout << "Translated " << (int)instructionCount << " instructions to " << outputFile << std::endl;

	return 0;
}

/* Example file -> test.ula
	inicio:
	X=12;
	Y=6;
	W=AeB;
	X=10;
	Y=3;
	W=AoB;
	W=AeBn;
	X=13;
	W=nB;
	fim.
*/
 
