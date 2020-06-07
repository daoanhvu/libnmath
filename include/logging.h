#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <iostream>
#include <string>

enum text_color {
	FG_RED      = 31,
        FG_GREEN    = 32,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
};

std::ostream& print_with_color(std::ostream& out, const char* text, text_color tc);
std::ostream& print_with_color(std::ostream& out, std::string text, text_color tc);

#endif