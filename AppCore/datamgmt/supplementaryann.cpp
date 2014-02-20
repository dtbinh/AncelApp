// Supplementary data structure for ANN data.

#include "supplementaryann.h"
#include "matwriter.h"

#include <vector>

// Create supplementary data.
GPCMSupplementaryANN::GPCMSupplementaryANN(
    double frameTime                        // Number of seconds per frame.
    )
{
    this->frameTime = frameTime;
}

// Write supplementary data.
void GPCMSupplementaryANN::write(
    GPCMMatWriter *writer                   // Stream to write supplementary data to.
    )
{
    // Nothing to write for now.
}

// Destructor.
GPCMSupplementaryANN::~GPCMSupplementaryANN()
{
}

// Get frame length.
double GPCMSupplementaryANN::getFrameTime()
{
    return this->frameTime;
}
