//
// Created by darren on 03/04/24.
//

#ifndef ID_PACK_TOOLS_PACKFILE_OUTPUT_H
#define ID_PACK_TOOLS_PACKFILE_OUTPUT_H

#include<iostream>

namespace Id::Pack::Tools
{
    void out(std::ostream &out, const std::string &msg) noexcept;

    void message(const std::string &msg) noexcept;

    void error(const std::string &msg) noexcept;

    void fatal(int code, const std::string &msg) noexcept;
}

#endif
