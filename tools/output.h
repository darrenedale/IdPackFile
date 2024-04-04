#ifndef TOOLS_OUTPUT_H
#define TOOLS_OUTPUT_H

#include <iostream>

namespace Id::Pack::Tools
{
    void info(const std::string &msg) noexcept;
    void error(const std::string &msg) noexcept;
}

#endif
