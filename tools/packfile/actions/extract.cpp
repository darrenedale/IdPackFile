#include <format>
#include <iomanip>
#include "extract.h"
#include "../../output.h"
#include "../../ExitCode.h"
#include "../../../sdk/Reader"

using Id::Pack::Tools::PackFile::ActionArguments;
using Id::Pack::Tools::error;
using Id::Pack::Tools::PackFile::ExitCode;
using Id::Pack::Reader;


std::optional<int> parseInt(const std::string & str) noexcept
{
    static std::size_t pos;
    auto value = std::stoi(str, &pos);

    if (pos != str.length()) {
        return {};
    }

    return value;
}


int Id::Pack::Tools::PackFile::Actions::extract(const ActionArguments & args) noexcept
{
    if (args.empty()) {
        error("You must provide the pac file to read, one file to extract, and a destination path.");
        return ExitCode::MissingArgument;
    }

    std::string pacFileName;

    try {
        bool verbose = false;
        std::list<int> numberedFiles;
        std::list<std::string> namedFiles;
        ActionArguments::const_iterator it;

        for (it = args.cbegin(); it != args.cend(); ++it) {
            const auto & arg = *it;

            if ("-v" == arg || "--verbose" == arg) {
                verbose = true;
            } else if ("-n" == arg) {
                if (pacFileName.empty()) {
                    error("PACK file name must be given before any files to extract.");
                    return ExitCode::MissingArgument;
                }

                ++it;
                auto idx = parseInt(*it);

                if (!idx) {
                    error(std::format("Expected valid int as argument for -n, found {}", *it));
                    return ExitCode::InvalidArgument;
                }

                numberedFiles.push_back(*idx);
            } else {
                if (pacFileName.empty()) {
                    pacFileName = *it;
                } else {
                    namedFiles.push_back(*it);
                }
            }
        }

        if (namedFiles.empty()) {
            error("Destination for extracted file(s) must be given as the last command-line argument.");
            return ExitCode::MissingArgument;
        }

        auto reader = Reader(pacFileName);
        auto dest = namedFiles.back();
        namedFiles.pop_back();
        auto totalExtractions = namedFiles.size() + numberedFiles.size();

        if (0 == totalExtractions) {
            error("No files to extract - did you forget to specify the destination?");
            return ExitCode::MissingArgument;
        }

        if (verbose) {
            std::cout << "Extracting " << totalExtractions << " file" << (1 == totalExtractions ? "" : "s") << " ";

            for (const auto & name: namedFiles) {
                std::cout << "\"" << name << "\", ";
            }

            for (const auto idx: numberedFiles) {
                std::cout << idx << ", ";
            }

            std::cout << "from \"" << pacFileName << "\" to \"" << dest << "\"\n";
        }

        for (const auto & name: namedFiles) {
            std::string outputPath = dest;

            if (1 < totalExtractions) {
                outputPath = std::format("{}/{}", outputPath, name);
            }

            if (verbose) {
                std::cout << "Extracting " << reader.fileSize(name) << " bytes from offset " << reader.fileOffset(name) << " of file \"" << name << "\" to \"" << outputPath << "\"\n";
            }

            reader.extract(name, outputPath);
        }

        for (const auto idx: numberedFiles) {
            std::string outputPath = std::format("{}/{}", dest, reader.fileName(idx));

            if (verbose) {
                std::cout << "Extracting " << reader.fileSize(idx) << " bytes from offset " << reader.fileOffset(idx) << " of file #" << idx << " (\"" << reader.fileName(idx) << "\") to \"" << outputPath << "\"\n";
            }

            reader.extract(idx, outputPath);
        }
    } catch (const std::runtime_error & err) {
        error(std::format(R"(Failed extracting from PACK file "{}": {})", pacFileName, err.what()));
        return -1;
    }

    return ExitCode::Ok;
}
