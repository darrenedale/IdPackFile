//
// Created by darren on 03/04/24.
//

#ifndef IDPACKREADER_ACTIONS_H
#define IDPACKREADER_ACTIONS_H

#include <string>
#include <list>
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
