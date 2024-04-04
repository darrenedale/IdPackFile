#include "output.h"

namespace
{
    void out(std::ostream &out, const std::string &msg) noexcept
    {
        out << msg << "\n";
    }
}

namespace Id::Pack::Tools
{
    void info(const std::string &msg) noexcept
    {
        out(std::cout, msg);
    }


    void error(const std::string &msg) noexcept
    {
        out(std::cerr, msg);
    }
}
