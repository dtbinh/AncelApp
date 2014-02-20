// Object for creating and writing to a struct inside a mat file.
#pragma once

#include "matwriter.h"

class GPCMMatStruct : public GPCMMatWriter
{
protected:
    // Buffer for writing field names.
    std::ostringstream namestream;
    // Number of fields written so far.
    int fields;
    // Helper function to seek back and enter field name into struct header.
    void seekWriteName(std::string name);
public:
    // Create struct writer with specified initial byte count.
    GPCMMatStruct(std::ostream &writestream, int bytes);
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
    // Return the number of fields.
    int getFieldCount();
    // Dump fields data into specified stream and return number of fields.
    void writeFields(std::ostream &nameoutstream);
    // Destructor - close file & clean up.
    ~GPCMMatStruct();
};
