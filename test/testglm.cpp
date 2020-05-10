#include <iostream>
#include "glm/glm.hpp"

int main(int argc, const char* argv[]) {
	glm::mat4 m = glm::mat4(1.0f);
	std::cout << std::endl;
	for(auto i=0; i<4; i++) {
		for(auto j=0; j<4; j++) {
			std::cout << m[i][j] << " ";
		}
		std::cout << std::endl;
	}
	return 0;
}