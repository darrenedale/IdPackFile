#ifndef TOOLS_EXITCODE_H
#define TOOLS_EXITCODE_H

namespace Id::Pack::Tools
{
    /** Common exit codes for tools. */
    enum ExitCode
    : int {
        Ok = 0,
        MissingAction = 1,
        UnrecognisedAction = 2,
        MissingArgument = 50,
        InvalidArgument = 51,
    };
}

#endif
