// Data loader for ANN annotation files.

#include "debugprint.h"
#include "datareaderann.h"
#include "supplementary.h"
#include "supplementaryann.h"
#include "mathutils.h"

#include <list>
#include <iterator>
#include <stdint.h>

// First frame to read from ANN file.
#define FIRST_FRAME 1

// Default frame time.
#define FRAME_TIME  0.0333333

// Create the data reader.
GPCMDataReaderANN::GPCMDataReaderANN()
{
    supplementary = NULL;
}

// Load a single file.
MatrixXd GPCMDataReaderANN::loadFile(
    std::string filename                    // File to load from.
    )
{
    // Open the file.
    std::ifstream annstream(filename.c_str(),std::ios::binary|std::ios::in);
    if (!annstream)
    {
        DBERROR("Failed to open file " << filename);
        return MatrixXd(0,0);
    }

    // Read columns and rows.
    uint32_t cols;
    uint32_t rows;
    annstream.read((char*)&cols,sizeof(uint32_t));
    annstream.read((char*)&rows,sizeof(uint32_t));

    // Create output matrix.
    MatrixXd data(cols,rows);

    // Read the data.
    annstream.read((char*)(data.data()),sizeof(double)*cols*rows);
    
    // Clean up.
    annstream.close();

    // Create supplementary data.
    if (supplementary) delete supplementary;
    supplementary = new GPCMSupplementaryANN(FRAME_TIME);

    // Return result.
    return data;
}

// Destructor.
GPCMDataReaderANN::~GPCMDataReaderANN()
{
}
