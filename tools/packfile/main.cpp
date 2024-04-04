//
// Created by darren on 03/04/24.
//

#include <format>
#include <iostream>
#include "actions.h"
#include "actions/list.h"
#include "actions/extract.h"
#include "../ExitCode.h"
#include "../output.h"


using namespace Id::Pack::Tools::PackFile;
using Id::Pack::Tools::error;

namespace
{
    std::string executable;
}


const ActionList & actions() noexcept
{
    static ActionList actions;

    if (actions.empty()) {
        actions.emplace_back("list", "List the files in one or more PACK file(s)", Actions::list);
        actions.emplace_back("extract", "Extract one or more files from a single PACK file", Actions::extract);
    }

    return actions;
}


void usage() noexcept
{
    std::cout << std::format(
R"(Process ID software PACK (.pak) files.

Usage: {} action [...args]

  action:
)", executable);

    for (const auto & action : actions()) {
        std::cout << std::format("    {: <10} - {}", action.command, action.description) << "\n";
    }
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
