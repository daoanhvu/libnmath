#include "logging.h"

std::ostream& print_with_color(std::ostream& out, const char* text, text_color tc) {
	out << "\033[" << tc << "m" << text << "\033[0m";
	return out;
}

std::ostream& print_with_color(std::ostream& out, std::string text, text_color tc) {
	out << "\033[" << tc << "m" << text << "\033[0m";
	return out;
}