//
// Created by darren on 03/04/24.
//

#include <format>
#include <iomanip>
#include "list.h"
#include "../../output.h"
#include "../../ExitCode.h"
#include "../../../sdk/Reader"

using Id::Pack::Tools::PackFile::ActionArguments;
using Id::Pack::Tools::error;
using Id::Pack::Tools::PackFile::ExitCode;
using Id::Pack::Reader;


int Id::Pack::Tools::PackFile::Actions::list(const ActionArguments & args) noexcept
{
    bool verbose = false;
    ActionArguments::const_iterator it;

    for (it = args.cbegin(); it != args.cend(); ++it) {
        const auto & arg = *it;

        if ("-v" == arg || "--verbose" == arg) {
            verbose = true;
        } else {
            break;
        }
    }

    if (it == args.cend()) {
        error("Missing .pak file name(s)");
        return ExitCode::MissingArgument;
    }

    while (it != args.cend()) {
        try {
            auto reader = Reader(*it);

            if (verbose) {
                for (int idx = 0; idx < reader.fileCount(); ++idx) {
                    std::cout << std::dec << std::setw(4) << std::setfill(' ') << idx << ": " << reader.fileName(idx) << " " << reader.fileSize(idx) << " bytes @ 0x" << std::setw(8) << std::setfill('0') << std::hex << reader.fileOffset(idx) <<"\n";
                }
            } else {
                for (int idx = 0; idx < reader.fileCount(); ++idx) {
                    std::cout << reader.fileName(idx) << "\n";
                }
            }
        } catch (const std::runtime_error & err) {
            error(std::format(R"(Failed reading file "{}": {})", *it, err.what()));
        }

        ++it;
    }

    return ExitCode::Ok;
}
