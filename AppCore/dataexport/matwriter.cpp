// General object that writes to a MATLAB mat file.

#include <stdint.h>

#include "matwriter.h"
#include "matstruct.h"

// Create writer to use the specified file stream.
GPCMMatWriter::GPCMMatWriter(
    std::ostream &writestream               // Stream to use.
    ) : writestream(writestream), active(NULL), bytes(0), structStream(std::stringstream::binary)
{
}

// Write a single number to the file.
void GPCMMatWriter::writeDouble(
    double val,                             // Value to store.
    std::string name                        // Matlab name for variable.
    )
{
    // Since MATLAB considers everything an array, just use the array function.
    MatrixXd valarray(1,1);
    valarray(0,0) = val;
    writeMatrix(valarray,name);
}

// Helper function for preparing to write a general numeric array.
int GPCMMatWriter::arrayHeader(
    std::string name,                       // Matlab name for variable.
    int r,                                  // Number of rows in the array.
    int c,                                  // Number of columns in the array.
    int b,                                  // Number of bytes of data in the array.
    int clss,                               // Array class.
    int type                                // Array data type.
    )
{
    // Common variables.
    uint8_t padbytes[8];
    memset(padbytes,0,8);
    int pad = 8-(name.length()%8);
    if (pad == 8) pad = 0;
    uint32_t rows = r;
    uint32_t cols = c;

    // First write the variable tag.
    unsigned int bytes = 8*4 + // 4 tags.
                         8*2 + // 2 8-byte items.
           name.length()+pad + // bytes for the field name.
                           b;  // bytes for the data.
    if (!type) bytes -= 8; // If no type specified, do not write datatype tag.
    writeTag(miMATRIX,bytes);

    // Write flags.
    writeTag(miUINT32,8);
    uint32_t flags[2];
    memset(flags,0,8);
    flags[0] = clss;
    writestream.write((char*)&flags,8);

    // Write dimensions.
    writeTag(miINT32,8);
    writestream.write((char*)&rows,4);
    writestream.write((char*)&cols,4);

    // Write array name.
    writeTag(miINT8,name.length());
    const char *namestr = name.c_str();
    writestream.write(namestr,name.length());
    writestream.write((char*)padbytes,pad);

    // Write array tag.
    if (type)
        writeTag(type,b);

    // Return number of bytes written.
    return bytes-b;
}

// Write a matrix to the file.
void GPCMMatWriter::writeMatrix(
    MatrixXd val,                           // Value to store.
    std::string name                        // Matlab name for variable.
    )
{
    assert(!active); // Must not currently be inside a struct.

    int rows = val.rows();
    int cols = val.cols();
    int bytesToWrite = rows*cols*8;
    int headerBytes = arrayHeader(name,rows,cols,bytesToWrite,mxDOUBLE_CLASS,miDOUBLE);

    // Write array values.
    for (int c = 0; c < cols; c++)
    {
        MatrixXd column = val.col(c);
        const double *coldata = column.data();
        writestream.write((char*)coldata,8*rows);
    }

    // Count bytes.
    this->bytes += bytesToWrite+headerBytes+8;
}

// Write a string to the file.
void GPCMMatWriter::writeString(
    std::string val,                        // String text.
    std::string name                        // Matlab name for variable.
    )
{
    assert(!active); // Must not currently be inside a struct.

    int bytesToWrite = val.length()*2;
    int cols = val.length();
    int pad = 8-((val.length()*2)%8);
    if (pad == 8) pad = 0;
    uint8_t padbytes[8];
    memset(padbytes,0,8);
    int rows = 1;
    if (cols == 0) rows = 0;
    int headerBytes = arrayHeader(name,rows,cols,bytesToWrite,mxCHAR_CLASS,miUINT16);

    // Write string.
    const char *vstr = val.c_str();
    for (unsigned i = 0; i < val.length(); i++)
    {
        uint16_t val = vstr[i];
        writestream.write((char*)&val,2);
    }
    writestream.write((char*)padbytes,pad);

    // Count bytes.
    this->bytes += bytesToWrite+headerBytes+pad+8;
}

// Write an empty matrix to the file.
void GPCMMatWriter::writeBlank(
    std::string name                        // Matlab name for variable.
    )
{
    assert(!active); // Must not currently be inside a struct.

    int bytesToWrite = 0;
    int headerBytes = arrayHeader(name,0,0,bytesToWrite,mxDOUBLE_CLASS,miDOUBLE);

    // Count bytes.
    this->bytes += headerBytes+8;
}

// Begin writing a struct.
GPCMMatWriter *GPCMMatWriter::writeStruct(
    std::string name,                       // Name of struct to write.
    int r,                                  // Number of rows in struct array.
    int c                                   // Number of columns in struct array.
    )
{
    assert(!active); // Must not currently be inside a struct.

    // Common variables.
    uint8_t padbytes[8];
    memset(padbytes,0,8);
    int pad = 8-(name.length()%8);
    if (pad == 8) pad = 0;
    uint32_t rows = r;
    uint32_t cols = c;

    // First write the variable tag.
    unsigned int bytes = 0; // We will go back and edit this later.
    writeTag(miMATRIX,bytes);

    // Write flags.
    writeTag(miUINT32,8);
    uint32_t flags[2];
    memset(flags,0,8);
    flags[0] = mxSTRUCT_CLASS;
    writestream.write((char*)&flags,8);

    // Write dimensions.
    writeTag(miINT32,8);
    writestream.write((char*)&rows,4);
    writestream.write((char*)&cols,4);

    // Write name.
    writeTag(miINT8,name.length());
    const char *namestr = name.c_str();
    writestream.write(namestr,name.length());
    writestream.write((char*)padbytes,pad);

    // Write field name length.
    writeSmallTag(miINT32,4);
    uint32_t length = FIELD_NAME_LENGTH;
    writestream.write((char*)&length,4);

    // Create and return writer for first struct in array.
    this->structBytes = 0;
    this->structIdx = 0;
    this->structEntries = r*c;
    this->structNamePad = pad+name.length();
    this->structStream.str("");
    active = new GPCMMatStruct(this->structStream,0);
    return active;
}

// Finish writing a struct.
GPCMMatWriter *GPCMMatWriter::closeStruct()
{
    // Write data into stream - first cast to struct writer.
    GPCMMatStruct *structActive = dynamic_cast<GPCMMatStruct*>(active);

    // Get number of bytes.
    this->structBytes = active->getBytes();

    // If this is the first entry, write field information.
    if (this->structIdx == 0)
    {
        // Get field count.
        int fieldCount = structActive->getFieldCount();

        // Write field tag.
        writeTag(miINT8,FIELD_NAME_LENGTH*fieldCount);

        // Write out the names.
        structActive->writeFields(writestream);

        // Increment byte count.
        this->structBytes += FIELD_NAME_LENGTH*fieldCount;
    }

    // Write struct data.
    std::string structstr = this->structStream.str();
    const char *structbuf = structstr.c_str();
    assert(structstr.length()%8 == 0);
    writestream.write(structbuf,structstr.length());

    // Increment index and delete writer.
    this->structIdx++;
    delete active;
    active = NULL;

    if (this->structIdx == this->structEntries)
    { // Store final number of bytes written.
        // Compute how far to seek.
        int seekbytes = this->structBytes + // This takes us up to beginning of field names.
                                        8 + // This skips field name tag.
                                        8 + // This skips field name lengths.
                      this->structNamePad + // This skips padded struct name.
                                       40 + // This skips padded struct tag, dimensions, and flags
                                        4;  // This skips to beginning of byte count.


        // Seek to the place where we want to write the bytes.
        writestream.seekp(-seekbytes,std::ios_base::cur);

        // Write total bytes.
        uint32_t totalBytes = seekbytes - 4; // Simply subtract off the byte count bytes.
        writestream.write((char*)&totalBytes,4);

        // Seek back.
        writestream.seekp(totalBytes,std::ios_base::cur);

        // Increment byte count.
        this->bytes += totalBytes+8;
    }
    else
    { // Move on to the next struct in struct array.
        this->structStream.str("");
        active = new GPCMMatStruct(this->structStream,this->structBytes);
    }

    // Return new writer.
    return active;
}

// Begin writing a cell array.
GPCMMatWriter *GPCMMatWriter::writeCell(
    std::string name,                       // Name of struct to write.
    int r,                                  // Number of rows in struct array.
    int c                                   // Number of columns in struct array.
    )
{
    assert(!active); // Must not currently be inside a struct.

    // Create the header.
    int bytesToWrite = 0; // This will be filled in later.
    int headerBytes = arrayHeader(name,r,c,bytesToWrite,mxCELL_CLASS,0);

    // Create and return writer for first entry in cell array.
    this->structBytes = headerBytes;
    active = new GPCMMatWriter(writestream);
    return active;
}

// Finish writing a cell array entry.
void GPCMMatWriter::closeCell()
{
    // Get the number of bytes written into the cell array.
    this->structBytes += active->getBytes();
    delete active;
    active = NULL;

    // Seek to the place where we want to write the bytes.
    writestream.seekp(-this->structBytes-4,std::ios_base::cur);

    // Write total bytes.
    uint32_t totalBytes = this->structBytes;
    writestream.write((char*)&totalBytes,4);

    // Seek back.
    writestream.seekp(totalBytes,std::ios_base::cur);

    // Increment byte count.
    this->bytes += totalBytes+8;
}

// Get number of bytes written.
int GPCMMatWriter::getBytes()
{
    return this->bytes;
}

// Helper function for writing a small 4 byte tag.
void GPCMMatWriter::writeSmallTag(
    unsigned int tag,                       // Data type tag.
    unsigned int bytes                      // Number of bytes.
    )
{
    // First write the tag.
    uint16_t t = tag;
    writestream.write((char*)&t,2);

    // Next write the number of bytes.
    uint16_t b = bytes;
    writestream.write((char*)&b,2);
}

// Helper function for writing a tag.
void GPCMMatWriter::writeTag(
    unsigned int tag,                       // Data type tag.
    unsigned int bytes                      // Number of bytes.
    )
{
    // First write the tag.
    uint32_t t = tag;
    writestream.write((char*)&t,4);

    // Next write the number of bytes.
    uint32_t b = bytes;
    writestream.write((char*)&b,4);
}

// Destructor.
GPCMMatWriter::~GPCMMatWriter()
{
}
