// Data loader for ANN annotation files.
#pragma once

#include "datareader.h"
#include "joint.h"
#include <fstream>

// Forward declarations.
class GPCMSupplementaryData;
class GPCMSupplementaryANN;

class GPCMDataReaderANN : public GPCMDataReader
{
protected:
public:
    // Create the data reader.
    GPCMDataReaderANN();
    // Load a single file.
    virtual MatrixXd loadFile(std::string filename);
    // Destructor.
    virtual ~GPCMDataReaderANN();
};
