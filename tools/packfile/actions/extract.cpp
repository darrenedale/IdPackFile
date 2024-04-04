#include <format>
#include <iomanip>
#include "extract.h"
#include "../../output.h"
#include "../../ExitCode.h"
#include "../../../sdk/Reader"

using Id::Pack::Tools::PackFile::ActionArguments;
using Id::Pack::Tools::ExitCode;
using Id::Pack::Tools::error;
using Id::Pack::Reader;

extern std::string g_executable;

namespace
{
    /**
     * The options controlling the extraction.
     */
    struct Options
    {
        bool verbose = false;
        std::string pacFileName;
        std::string destination;
        std::list<int> numberedFiles;
        std::list<std::string> namedFiles;
    };

    /**
     * Parse an int from a string.
     *
     * The string must have no other content than the int, other than optional leading whitespace.
     *
     * @param str The string to parse.
     * @return The int if it was parsed successfully, empty otherwise.
     */
    std::optional<int> parseInt(const std::string & str) noexcept
    {
        static std::size_t pos;
        auto value = std::stoi(str, &pos);

        if (pos != str.length()) {
            return {};
        }

        return value;
    }


    /**
     * Show the usage message for the extract action.
     */
    void usage() noexcept
    {
        std::cout << g_executable << R"( extract [-v] packfile {file | -n index} [...{file | -n index}] destination

  Options
    -v  print verbose output

  Arguments
    packfile     The path to the PACK file from which to extract content
    file         One or more filenames to extract from the PACK file
    index        The index of one or more files to extract from the PACK file. Each index you wish to extract must be
                 preceded by -n so that it's known to be a file index not a file name
    destination  Where to store the extracted files. If there is more than one file being extracted this must be a
                 directory, to which the extracted file's name is appended. If a single file is being extracted, this is
                 the name of the file to save it to.
)";
    }

    /**
     * Parse the command-line arguments into a set of Options.
     *
     * @param args
     * @return The parsed options.
     * @throws std::runtime_error if the args are not valid.
     */
    Options parseArguments(const ActionArguments & args)
    {
        if (args.empty()) {
            throw std::runtime_error("You must provide the pac file to read, one file to extract, and a destination path.");
        }

        Options opts;
        ActionArguments::const_iterator it;

        for (it = args.cbegin(); it != args.cend(); ++it) {
            const auto & arg = *it;

            if ("-v" == arg || "--verbose" == arg) {
                opts.verbose = true;
            } else if ("-n" == arg) {
                if (opts.pacFileName.empty()) {
                    throw std::runtime_error("PACK file name must be given before any files to extract.");
                }

                ++it;
                auto idx = parseInt(*it);

                if (!idx) {
                    throw std::runtime_error(std::format("Expected valid int as argument for -n, found {}", *it));
                }

                opts.numberedFiles.push_back(*idx);
            } else {
                if (opts.pacFileName.empty()) {
                    opts.pacFileName = *it;
                } else {
                    opts.namedFiles.push_back(*it);
                }
            }
        }

        if (opts.namedFiles.empty()) {
            throw std::runtime_error("Destination for extracted file(s) must be given as the last command-line argument.");
        }

        opts.destination = opts.namedFiles.back();
        opts.namedFiles.pop_back();

        if (0 == opts.namedFiles.size() + opts.numberedFiles.size()) {
            throw std::runtime_error("No files to extract - did you forget to specify the destination?");
        }

        return std::move(opts);
    }

    /**
     * Summarise what the options will do.
     *
     * @param opts
     */
    void summarise(const Options & opts) noexcept
    {
        auto totalExtractions = opts.namedFiles.size() + opts.numberedFiles.size();
        std::cout << "Extracting " << totalExtractions << " file" << (1 == totalExtractions ? "" : "s") << " ";

        for (const auto & name : opts.namedFiles) {
            std::cout << "\"" << name << "\", ";
        }

        for (const auto idx : opts.numberedFiles) {
            std::cout << idx << ", ";
        }

        std::cout << "from \"" << opts.pacFileName << "\" to \"" << opts.destination << "\"\n";
    }
}


/**
 * Extract one or more files from an ID PACk archive.
 *
 * @param args The command-line arguments provided to the extract action.
 *
 * @return ExitCode::Ok on success, another ExitCode if the command is not valid, a negative int if something went wrong
 * trying to extract the files.
 */
int Id::Pack::Tools::PackFile::Actions::extract(const ActionArguments & args) noexcept
{
    Options opts;

    try {
        opts = parseArguments(args);
    } catch (const std::runtime_error & err) {
        error(err.what());
        usage();
        return ExitCode::InvalidArgument;
    }

    auto totalExtractions = opts.namedFiles.size() + opts.numberedFiles.size();

    try {
        auto reader = Reader(opts.pacFileName);

        if (opts.verbose) {
            summarise(opts);
        }

        // extract the named files
        for (const auto & name : opts.namedFiles) {
            std::string outputPath = opts.destination;

            if (1 < totalExtractions) {
                outputPath = std::format("{}/{}", outputPath, name);
            }

            if (opts.verbose) {
                std::cout << "Extracting " << reader.fileSize(name) << " bytes from offset " << reader.fileOffset(name) << " of file \"" << name << "\" to \"" << outputPath << "\"\n";
            }

            reader.extract(name, outputPath);
        }

        // extract the numbered files
        for (const auto idx : opts.numberedFiles) {
            std::string outputPath = std::format("{}/{}", opts.destination, reader.fileName(idx));

            if (opts.verbose) {
                std::cout << "Extracting " << reader.fileSize(idx) << " bytes from offset " << reader.fileOffset(idx) << " of file #" << idx << " (\"" << reader.fileName(idx) << "\") to \"" << outputPath << "\"\n";
            }

            reader.extract(idx, outputPath);
        }
    } catch (const std::runtime_error & err) {
        error(std::format(R"(Failed extracting from PACK file "{}": {})", opts.pacFileName, err.what()));
        return -1;
    }

    return ExitCode::Ok;
}
