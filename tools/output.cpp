//
// Created by darren on 03/04/24.
//

#include "output.h"

namespace Id::Pack::Tools
{
    void out(std::ostream &out, const std::string &msg) noexcept {
        out << msg << "\n";
    }


    void message(const std::string &msg) noexcept {
        out(std::cout, msg);
    }


    void error(const std::string &msg) noexcept {
        out(std::cerr, msg);
    }


    void fatal(int code, const std::string &msg) noexcept {
        out(std::cerr, msg);
        exit(code);
    }
}
