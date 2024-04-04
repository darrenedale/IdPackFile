//
// Created by darren on 03/04/24.
//

#include <format>
#include <iomanip>
#include "list.h"
#include "../../output.h"
#include "../../ExitCode.h"
#include "../../../sdk/Reader"

using Id::Pack::Tools::error;
using Id::Pack::Tools::ExitCode;
using Id::Pack::Tools::PackFile::ActionArguments;
using Id::Pack::Reader;

extern std::string g_executable;

namespace
{
    void usage() noexcept
    {
        std::cout << std::format(R"(Usage: {} list [-v|--verbose] file [...file]

  Options
    -v, --verbose
      print verbose output - includes the file index, byte offset and byte size for each file in the archive(s)

  Arguments
    file  One or more paths to PACK files whose contents should be listed
)", g_executable);
    }
}


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
        usage();
        return ExitCode::MissingArgument;
    }

    while (it != args.cend()) {
        try {
            auto reader = Reader(*it);

            if (verbose) {
                // work out how many digits we need for the file index
                int digits = 1;
                int count = reader.fileCount();

                while (10 < count) {
                    ++digits;
                    count /= 10;
                }

                for (int idx = 0; idx < reader.fileCount(); ++idx) {
                    std::cout << std::format("{: >{}}: {} {} bytes @ {:#010x}", idx, digits, reader.fileName(idx), reader.fileSize(idx), reader.fileOffset(idx)) << "\n";
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
