// General object that reads from a MATLAB mat file.

#include <stdint.h>

#include "matwriter.h"
#include "matreader.h"
#include "debugprint.h"

// Size of the header in bytes, including version and endianness marker.
#define FULL_HEADER_SIZE    128

// Size of a tag in bytes.
#define TAG_SIZE            8

// Maximum variable name length we can handle.
#define MAX_NAME_LENGTH     256

// Create a new reader from filename.
GPCMMatReader::GPCMMatReader(
    const std::string &filename             // Path for file to load.
    )
{
    // Open the file.
    std::ifstream file(filename.c_str(),std::ios_base::binary);

    // Load the file.
    readFile(file);

    // Close the file.
    file.close();
}

// Create a new reader from file stream.
GPCMMatReader::GPCMMatReader(
    std::ifstream &file,                    // File stream to read from.
    int mode,                               // Type of structure to read.
    int rows,                               // Optional row count.
    int cols                                // Optional column count.
    )
{
    if (mode == 0)
        readFile(file);
    else if (mode == 1)
        readStruct(file,rows,cols);
    else if (mode == 2)
        readCell(file,rows,cols);
}

// Read a single variable.
void GPCMMatReader::readVariable(
    std::ifstream &file,                    // File stream to read from.
    std::string namestr,                    // Variable name.
    bool bNameProvided                      // Indicates whether the name is provided.
    )
{
    // Skip over the tag.
    file.seekg(TAG_SIZE,std::ios_base::cur);

    // Skip over the flags tag.
    file.seekg(TAG_SIZE,std::ios_base::cur);

    // Read the class.
    uint32_t type;
    file.read((char*)&type,4);
    file.seekg(4,std::ios_base::cur);

    // Skip over dimensions tag.
    file.seekg(TAG_SIZE,std::ios_base::cur);

    // Read the dimensions.
    uint32_t rows, cols;
    file.read((char*)&rows,4);
    file.read((char*)&cols,4);

    // Read the name.
    char name[MAX_NAME_LENGTH];
    uint32_t nameLength;
    file.seekg(4,std::ios_base::cur); // This skips data type.
    file.read((char*)&nameLength,4);
    assert(nameLength < MAX_NAME_LENGTH);
    file.read(name,nameLength);
    name[nameLength] = '\0';
    if (!bNameProvided)
        namestr = std::string(name);
    
    // Compute and skip name pad bytes.
    int pad = 8-(nameLength%8);
    if (pad == 8) pad = 0;
    file.seekg(pad,std::ios_base::cur);

    // Determine what type of variable this is.
    switch (type)
    {
    case mxCHAR_CLASS:
        { // String.
            // Skip the data tag.
            file.seekg(TAG_SIZE,std::ios_base::cur);

            // Reading back strings is unsupported - just skip the data.
            int entries = rows*cols*2;
            int pad = 8-(entries%8);
            if (pad == 8) pad = 0;
            file.seekg(entries+pad,std::ios_base::cur);
        }
        break;
    case mxDOUBLE_CLASS:
        { // Standard matrix.
            // Skip the data tag.
            file.seekg(TAG_SIZE,std::ios_base::cur);

            if (rows != 0 && cols != 0)
            {
                MatrixXd data(rows,cols);
                file.read((char*)data.data(),rows*cols*8);
                variables[std::string(namestr)] = data;
                variableOrder.push_back(std::string(namestr));
            }
        }
        break;
    case mxSTRUCT_CLASS:
        { // Struct (array).
            GPCMMatReader *reader = new GPCMMatReader(file,1,rows,cols);
            structs[std::string(namestr)] = reader;
            structOrder.push_back(std::string(namestr));
        }
        break;
    case mxCELL_CLASS:
        { // Cell array.
            GPCMMatReader *reader = new GPCMMatReader(file,2,rows,cols);
            structs[std::string(namestr)] = reader;
            structOrder.push_back(std::string(namestr));
        }
        break;
    default:
        DBERROR("Unknown variable type " << type << " encountered!");
        break;
    }
}

// Read the contents of a file.
void GPCMMatReader::readFile(
    std::ifstream &file                     // File stream to read from.
    )
{
    // Advance past the header, version and endianness marker.
    file.seekg(FULL_HEADER_SIZE,std::ios_base::cur);

    // Read variables until we get to the end of the file.
    while (file.peek() != EOF)
    {
        readVariable(file,"",false);
    }
}

// Read the contents of a struct or struct array.
void GPCMMatReader::readStruct(
    std::ifstream &file,                    // File stream to read from.
    int rows,                               // Number of rows.
    int cols                                // Number of columns.
    )
{
     // Read field name length.
    uint32_t fieldNameLength;
    file.seekg(4,std::ios_base::cur); // Skip data type.
    file.read((char*)&fieldNameLength,4);
    assert(fieldNameLength <= MAX_NAME_LENGTH);

    // Read number of fields.
    uint32_t fieldNamesSize;
    int numFields;
    file.seekg(4,std::ios_base::cur); // Skip data type.
    file.read((char*)&fieldNamesSize,4);
    numFields = fieldNamesSize/fieldNameLength;

    // Read the field names.
    std::vector<std::string> names;
    for (int i = 0; i < numFields; i++)
    {
        char buffer[MAX_NAME_LENGTH];
        file.read(buffer,fieldNameLength);
        names.push_back(std::string(buffer));
    }

    if (rows == 1 && cols == 1)
    { // Simply read in the variables.
        for (int i = 0; i < numFields; i++)
        {
            readVariable(file,names[i],true);
        }
    }
    else
    { // Create structs for each entry.
        int entries = rows*cols;
        for (int e = 0; e < entries; e++)
        {
            // Create name.
            std::stringstream ss;
            ss << "entry" << e;
            // Create subentry.
            GPCMMatReader *subentry = new GPCMMatReader(file,3);
            for (int i = 0; i < numFields; i++)
            {
                subentry->readVariable(file,names[i],true);
            }
            // Add subentry to structs list.
            structs[ss.str()] = subentry;
            structOrder.push_back(ss.str());
        }
    }
}

// Read the contents of a cell array.
void GPCMMatReader::readCell(
    std::ifstream &file,                    // File stream to read from.
    int rows,                               // Number of rows.
    int cols                                // Number of columns.
    )
{
    int vars = rows*cols;
    for (int i = 0; i < vars; i++)
    {
        std::stringstream ss;
        ss << "field" << i;
        readVariable(file,ss.str(),true);
    }
}

// Get number of variables in this structure/file.
int GPCMMatReader::variableCount()
{
    return variables.size();
}

// Get variable by index.
MatrixXd &GPCMMatReader::getVariable(
    int index                               // Index of desired variable.
    )
{
    return variables[variableOrder[index]];
}

// Get variable by name.
MatrixXd &GPCMMatReader::getVariable(
    const std::string &name                 // Name of desired variables.
    )
{
    return variables[name];
}

// Get number of structs in this structure/file.
int GPCMMatReader::structCount()
{
    return structs.size();
}

// Get struct by index.
GPCMMatReader *GPCMMatReader::getStruct(
    int index                               // Index of desired variables.
    )
{
    return structs[structOrder[index]];
}

// Get struct by name.
GPCMMatReader *GPCMMatReader::getStruct(
    const std::string &name                 // Name of desired variable.
    )
{
    return structs[name];
}

// Destructor - release all variables.
GPCMMatReader::~GPCMMatReader()
{
    // Destroy all structs.
    for (std::map<std::string, GPCMMatReader*>::iterator itr = structs.begin();
         itr != structs.end(); ++itr)
    {
        delete itr->second;
    }
}
