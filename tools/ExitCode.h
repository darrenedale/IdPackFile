//
// Created by darren on 03/04/24.
//

#ifndef ID_PACK_TOOLS_PACKFILE_EXITCODE_H
#define ID_PACK_TOOLS_PACKFILE_EXITCODE_H

namespace Id::Pack::Tools::PackFile
{
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
