// Object for creating and writing to a struct inside a mat file.

#include "matstruct.h"

#include <stdint.h>

// Helper function to seek back and enter field name into struct header.
void GPCMMatStruct::seekWriteName(
    std::string name                        // Name to write.
    )
{
    // Write the field name into name stream.
    uint8_t padbytes[FIELD_NAME_LENGTH];
    memset(padbytes,0,FIELD_NAME_LENGTH);
    int pad = FIELD_NAME_LENGTH-name.length();
    assert(pad >= 0);
    const char *namestr = name.c_str();
    namestream.write(namestr,name.length());
    namestream.write((char*)padbytes,pad);

    // Increment number of fields.
    fields++;
}

// Return the number of fields.
int GPCMMatStruct::getFieldCount()
{
    return fields;
}

// Dump fields data into specified stream.
void GPCMMatStruct::writeFields(
    std::ostream &nameoutstream             // Stream to write name into.
    )
{
    // Get name buffer.
    std::string namestr = namestream.str();
    const char *namebuf = namestr.c_str();

    // Write buffer.
    assert(namestr.length()%FIELD_NAME_LENGTH == 0);
    nameoutstream.write(namebuf,namestr.length());
}

// Create struct writer with specified initial byte count.
GPCMMatStruct::GPCMMatStruct(
    std::ostream &writestream,              // Stream to use.
    int bytes
    ) : GPCMMatWriter(writestream), fields(0), namestream(std::stringstream::binary)
{
    this->bytes = bytes;
}

// Write a matrix to the file.
void GPCMMatStruct::writeMatrix(
    MatrixXd val,                           // Value to store.
    std::string name                        // Matlab name for variable.
    )
{
    seekWriteName(name);
    GPCMMatWriter::writeMatrix(val,"");
}

// Write a string to the file.
void GPCMMatStruct::writeString(
    std::string val,                        // String text.
    std::string name                        // Matlab name for variable.
    )
{
    seekWriteName(name);
    GPCMMatWriter::writeString(val,"");
}

// Write an empty matrix to the file.
void GPCMMatStruct::writeBlank(
    std::string name                        // Matlab name for variable.
    )
{
    seekWriteName(name);
    GPCMMatWriter::writeBlank("");
}

// Begin writing a struct.
GPCMMatWriter *GPCMMatStruct::writeStruct(
    std::string name,                       // Name of struct to write.
    int r,                                  // Number of rows in struct array.
    int c                                   // Number of columns in struct array.
    )
{
    seekWriteName(name);
    return GPCMMatWriter::writeStruct("",r,c);
}

// Begin writing a cell array.
GPCMMatWriter *GPCMMatStruct::writeCell(
    std::string name,                       // Name of struct to write.
    int r,                                  // Number of rows in cell array.
    int c                                   // Number of columns in cell array.
    )
{
    seekWriteName(name);
    return GPCMMatWriter::writeCell("",r,c);
}

// Destructor - close file & clean up.
GPCMMatStruct::~GPCMMatStruct()
{
}
