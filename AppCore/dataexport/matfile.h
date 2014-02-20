// Object for creating and writing to a .mat file.
#pragma once

#include "matwriter.h"

class GPCMMatFile : public GPCMMatWriter
{
protected:
    // The actual file stream.
    std::ofstream file;
public:
    // Create file with specified name and prepare for opening.
    GPCMMatFile(std::string filename);
    // Destructor - close file & clean up.
    virtual ~GPCMMatFile();
};
