#ifndef TOOLS_PACKFILE_ACTIONS_H
#define TOOLS_PACKFILE_ACTIONS_H

#include <list>
#include <string>
#include <vector>

namespace Id::Pack::Tools::PackFile
{
    using ActionArguments = std::vector<std::string>;
    using ActionFunction = int(*)(const ActionArguments & args);

    struct Action {
        std::string command;
        std::string description;
        ActionFunction function;
    };

    using ActionList = std::list<Action>;
}

#endif
