// Supplementary data structure for ANN data.
#pragma once

#include "supplementary.h"
#include "joint.h"

class GPCMSupplementaryANN : public GPCMSupplementaryData
{
protected:
public:
    // Create supplementary data.
    GPCMSupplementaryANN(double frameTime);
    // Write supplementary data.
    virtual void write(GPCMMatWriter *file);
    // Destructor.
    virtual ~GPCMSupplementaryANN();
    // Get frame length.
    double getFrameTime();
};
