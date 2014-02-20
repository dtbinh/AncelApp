// General object that writes to a MATLAB mat file.
#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include <Eigen/Core>

using namespace Eigen;

// Number of bytes to allocate for struct field name.
#define FIELD_NAME_LENGTH   32

// MATLAB data types.
enum MATLABDataTypes
{
    miINT8 = 1,
    miUINT8,
    miINT16,
    miUINT16,
    miINT32,
    miUINT32,
    miSINGLE,
    miRESERVED1,
    miDOUBLE,
    miRESERVED2,
    miRESERVED3,
    miINT64,
    miUINT64,
    miMATRIX,
    miCOMPRESSED,
    miUTF8,
    miUTF16,
    miUTF32
};

// MATLAB array types.
enum MATLABArrayTypes
{
    mxCELL_CLASS = 1,
    mxSTRUCT_CLASS,
    mxOBJECT_CLASS,
    mxCHAR_CLASS,
    mxSPARSE_CLASS,
    mxDOUBLE_CLASS,
    mxSINGLE_CLASS,
    mxINT8_CLASS,
    mxUINT8_CLASS,
    mxINT16_CLASS,
    mxUINT16_CLASS,
    mxINT32_CLASS,
    mxUINT32_CLASS,
    mxINT64_CLASS,
    mxUINT64_CLASS
};

class GPCMMatWriter
{
protected:
    // Binary output stream for writing to the file.
    std::ostream &writestream;
    // Currently active writing implement.
    GPCMMatWriter *active;
    // Number of bytes written.
    int bytes;
    // Binary output stream for current active struct.
    std::ostringstream structStream;
    // Number of bytes written so far in current struct or cell.
    int structBytes;
    // Number of entries written in current sturct or cell array.
    int structIdx;
    // Total number of entries.
    int structEntries;
    // Size of padded name of current struct or cell array.
    int structNamePad;
    // Helper function for writing a tag.
    void writeTag(unsigned int tag, unsigned int bytes);
    // Helper function for writing a small tag.
    void writeSmallTag(unsigned int tag, unsigned int bytes);
    // Helper function for preparing to write a general numeric array.
    int arrayHeader(std::string name, int rows, int cols, int bytes, int clss, int type);
public:
    // Create writer to use the specified file stream.
    GPCMMatWriter(std::ostream &writestream);
    // Write a single number to the file.
    virtual void writeDouble(double val, std::string name);
    // Write a matrix to the file.
    virtual void writeMatrix(MatrixXd val, std::string name);
    // Write a string to the file.
    virtual void writeString(std::string val, std::string name);
    // Write an empty matrix to the file.
    virtual void writeBlank(std::string name);
    // Begin writing a struct.
    virtual GPCMMatWriter *writeStruct(std::string name, int rows, int cols);
    // Begin writing a cell array.
    virtual GPCMMatWriter *writeCell(std::string name, int rows, int cols);
    // Finish writing a struct.
    virtual GPCMMatWriter *closeStruct();
    // Finish writing a cell array entry.
    virtual void closeCell();
    // Get number of bytes written.
    virtual int getBytes();
    // Destructor.
    virtual ~GPCMMatWriter();
};
