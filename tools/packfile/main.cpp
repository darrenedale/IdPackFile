//
// Created by darren on 03/04/24.
//

#include <format>
#include <iostream>
#include "actions.h"
#include "actions/list.h"
#include "../ExitCode.h"
#include "../output.h"


using namespace Id::Pack::Tools::PackFile;
using Id::Pack::Tools::error;

namespace
{
    std::string executable;
}


void usage() noexcept
{
    std::cout << std::format(
R"(Process ID software PACK (.pac) files.

Usage: {} action [...args]

  action:
    list     - list the files in the PACK file
)", executable);
}


const ActionList & actions() noexcept
{
    static ActionList actions;

    if (actions.empty()) {
        actions.emplace_back("list", "List the files in the PACk file", Actions::list);
    }

    return actions;
}


int main(int argc, char ** argv)
{
    executable = argv[0];

    if (1 == argc) {
        error("Missing action");
        usage();
        return ExitCode::MissingAction;
    }

    const auto command = std::string(argv[1]);

    for (const auto & action : actions()) {
        if (command == action.command) {
            ActionArguments args;
            args.reserve(argc - 2);

            for (int idx = 2; idx < argc; ++idx) {
                args.emplace_back(argv[idx]);
            }

            return action.function(args);
        }
    }

    error(std::format(R"(Unrecognised action "{}")", command));
    return ExitCode::UnrecognisedAction;
}
