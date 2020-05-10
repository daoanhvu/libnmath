
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "common.hpp"
#include "nlablexer.h"
#include "nfunction.hpp"
#include "SimpleCriteria.hpp"
#include "logging.h"

void testFunction0() {
	nmath::NFunction<float> *f;
	nmath::NLabLexer lexer;
	nmath::NLabParser<float> parser;
	int tokenInUse;
	int errorCode;
	int errorColumn;
	std::ifstream dataFile("/Users/vdao/Documents/projects/demo/libnmath/test/criteria.txt");
	std::string line;
	float v[1] = {-2.0f};

	if(dataFile.is_open()) {
		while( getline(dataFile, line) ) {
			f = new nmath::NFunction<float>();
			std::cout << "Process " << line << std::endl;
			int error = f->parse(line, &lexer, &parser);
			if(error != NMATH_NO_ERROR) {
				std::cout << "Error with code: " << error << " at " << f->getErrorColumn() <<std::endl;
				print_with_color(std::cout, "Test failed! Cannot parse the expression " + line, FG_RED) << std::endl;
				continue;
			}

			float val = f->calc(v);
			errorCode = f->getErrorCode();
			if(errorCode != NMATH_NO_ERROR) {
				if(errorCode == ERROR_OUT_OF_DOMAIN) {
					print_with_color(std::cout, "Input value out of domain ", FG_GREEN) << std::endl;
				} else {
					print_with_color(std::cout, "Test failed!!!!", FG_RED) << std::endl;
				}
			} else {
				print_with_color(std::cout, "Value = ", FG_GREEN) << val << std::endl;
			}

			delete f;
		}// end while

		dataFile.close();
	}
}


int main(int argc, char* argv[]) {
	testFunction0();
	return 0;
}