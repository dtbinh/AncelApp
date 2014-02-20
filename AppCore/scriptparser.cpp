// Object for parsing a script file and returning an options structure.

#include "matwriter.h"
#include "scriptparser.h"
#include "options.h"
#include "debugprint.h"

#include <fstream>
#include <boost/config.hpp>
#include <boost/program_options/detail/config_file.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/tokenizer.hpp>

// Helper function to parse a string representing a vector.
VectorXd GPCMScriptParser::parseVector(
    std::string &str                        // String to parse.
    )
{
    // Tokenize string.
    boost::tokenizer<> tokenizer(str,boost::char_delimiters_separator<char>(false,"","; ,\t\n\r"));

    // Pull out values.
    std::vector<double> nums;
    for (boost::tokenizer<>::iterator itr = tokenizer.begin();
         itr != tokenizer.end(); ++itr)
    {
        nums.push_back(atof(itr->c_str()));
    }

    // Copy into Eigen vector.
    VectorXd result(nums.size());
    for (unsigned i = 0; i < nums.size(); i++)
        result(i) = nums[i];

    // Return vector.
    return result;
}

// Constructor.
GPCMScriptParser::GPCMScriptParser(
    std::string script,                     // Script to load.
    std::string dir                         // Working directory.
    )
{
    // Store variables.
    scriptName = script;

    // Read the scripts file.
    this->script = readFile(script,dir);

    // Read in any includes.
    for (unsigned i = 0; i < options["include"]["file"].size(); i++)
    {
        readFile(options["include"]["file"][i],dir);
    }

    // Write dir.
    options["dir"]["dir"].push_back(dir);

    // Write name.
    options["dir"]["filename"].push_back(script);
}

// Read a script file.
std::string GPCMScriptParser::readFile(
    std::string script,                     // Script to load.
    std::string dir                         // Working directory.
    )
{
    std::string scriptFile = dir;
    scriptFile.append(script);
    std::ifstream scriptstream(scriptFile.c_str());
    if(!scriptstream)
    {
        DBERROR("Failed to open script file: " << script << " in " << dir);
        return std::string("");
    }

    // Read in the entire script file.
    std::string scriptStr = std::string(std::istreambuf_iterator<char>(scriptstream),
                               std::istreambuf_iterator<char>());

    // Rewind stream reader.
    scriptstream.seekg(0,std::ios_base::beg);

    // Read in the script.
    std::set<std::string> scriptreadoptions;
    scriptreadoptions.insert("*");
    try
    {
        for (boost::program_options::detail::config_file_iterator i(scriptstream, scriptreadoptions), e;
             i != e; ++i)
        {
            // Split key into category and label.
            std::string key = i->string_key;
            int idx = key.find_last_of('.');
            std::string category = key.substr(0,idx);
            std::string label = key.substr(idx+1);

            // Insert value.
            options[category][label].push_back(i->value[0]);
        }
    }
    catch(std::exception& e)    
    {
        std::cerr<<"Exception while parsing script: " << e.what() << std::endl;
    }

    // Return string.
    return scriptStr;
}

// Get options.
GPCMOptions &GPCMScriptParser::getOptions()
{
    return options;
}

// Write debugging information.
void GPCMScriptParser::write(
    GPCMMatWriter *writer                   // Writer to write result to.
    )
{
    // Create structure.
    GPCMMatWriter *optionsStruct = writer->writeStruct("script",1,1);

    // Write debugging stuff.
    std::string datetime(__DATE__);
    datetime += std::string(" ") + std::string(__TIME__);
    optionsStruct->writeString(datetime,"training_program_build");
    optionsStruct->writeString(scriptName,"script_filename");
    //writer->writeString(script,"script_text");

    // Write options.
    for (GPCMOptions::iterator itr = options.begin(); itr != options.end(); ++itr)
    {
        GPCMMatWriter *paramsStruct = optionsStruct->writeStruct(itr->first,1,1);
        for (GPCMParams::iterator pitr = itr->second.begin(); pitr != itr->second.end(); ++pitr)
        {
            GPCMMatWriter *strCell = paramsStruct->writeCell(pitr->first,1,pitr->second.size());
            for (std::vector<std::string>::iterator sitr = pitr->second.begin(); sitr != pitr->second.end(); ++sitr)
            {
                strCell->writeString(*sitr,"");
            }
            paramsStruct->closeCell();
        }
        optionsStruct->closeStruct();
    }
    writer->closeStruct();
}

// Destructor.
GPCMScriptParser::~GPCMScriptParser()
{
}
