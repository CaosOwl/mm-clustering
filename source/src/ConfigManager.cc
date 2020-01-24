//
// ***************************************************************************
// **** General-Purpose ConfigManager class used to manage configurations ****
// ***************************************************************************
//
// $Id: ConfigManager.cc Wed 2016-10-26 12:02:18 UTC Michael W. Heiss $
//
/// \file ConfigManager.cc
/// \brief Implementation of the ConfigManager class

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

// Include corresponding header
#include "ConfigManager.hh"

// include configuration for ConfigManager
#include "ConfigManager.conf.hh"

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

// Declare helper function for command-line / config file options
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
    return os;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

ConfigManager::ConfigManager(int ac, char** av) :
	argc(ac),
	argv(av),
	desc_generic("Generic options"),
	desc_parameters("Program parameters"),
    desc_parameters_hidden("Hidden parameters"),
    desc_parameters_cmd("Additional command-line parameters"),
    desc_parameters_cmd_hidden("Additional command-line hidden parameters"),
    desc_parameters_cfg("Additional config file parameters")
{}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

ConfigManager::~ConfigManager()
{}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::Initialize()
{
    // Declare a group of options that will be
    // allowed only on command line
    desc_generic.add_options()
        ("version", "print version string")
        ("help", "print help message");

    if (ConfigManagerOverrideConfigFile) desc_generic.add_options()
        ("config", value<string>(),"path to configuration file");

    CreateCustomDescription();

    if (strncmp(ConfigManagerFileHandlerMode,"in",2) == 0)
    {
        desc_parameters_cmd_hidden.add_options()
            ("cmdline-input-filename", value< vector<string> >(), "cmdline-input-filename");
        desc_positional.add("cmdline-input-filename", ((strcmp(ConfigManagerFileHandlerMode,"ins") == 0) ? -1 : 1));
    }

    if ( (strncmp(ConfigManagerFileHandlerMode,"out",3) == 0) || (strcmp(ConfigManagerFileHandlerMode,"inout") == 0) )
    {
        desc_parameters_cmd_hidden.add_options()
            ("cmdline-output-filename", value< vector<string> >(), "cmdline-output-filename");
        desc_positional.add("cmdline-output-filename", ((strcmp(ConfigManagerFileHandlerMode,"outs") == 0) ? -1 : 1));
    }

    cmdline_options.add(desc_generic).
                    add(desc_parameters).
                    add(desc_parameters_hidden).
                    add(desc_parameters_cmd).
                    add(desc_parameters_cmd_hidden);

    if (strcmp(ConfigManagerFileHandlerMode,"none") == 0)
    {
        boost::program_options::store(parse_command_line(argc, argv, cmdline_options), var_map);
    }
    else
    {
        boost::program_options::store(command_line_parser(argc, argv).options(cmdline_options).positional(desc_positional).run(), var_map);
    }

    notify(var_map);

    if (HelpRequested()) return false;

    if (VersionRequested()) return false;

    cfgfile_options.add(desc_generic).
                add(desc_parameters).
                add(desc_parameters_hidden).
                add(desc_parameters_cfg);

    if (var_map.count("config"))
    {
        ifstream override_config(var_map["config"].as<string>());
        if (!override_config)
        {
            string error_message = "File not found: ";
            error_message.append(var_map["config"].as<string>());
            CustomException new_exc(error_message.c_str());
            throw new_exc;
        }
        else
        {
            boost::program_options::store(parse_config_file(override_config, cfgfile_options), var_map);
            notify(var_map);
        }
    }

    if(strcmp(ConfigManagerDefaultConfigFile,"") != 0)
    {
        ifstream default_config(ConfigManagerDefaultConfigFile);
        if (!default_config)
        {
            if (!var_map.count("config"))
            {
                string error_message = "File not found: ";
                error_message.append(ConfigManagerDefaultConfigFile);
                CustomException new_exc(error_message.c_str());
                throw new_exc;
            }
            else
            {
                cerr << endl << "WARNING: Could not open default config-file in path: "
                     << ConfigManagerDefaultConfigFile << endl
                     << "\t Continuing with override config file..." << endl;
            }
        }
        else
        {
            boost::program_options::store(parse_config_file(default_config, cfgfile_options), var_map);
            notify(var_map);
        }
    }

    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::HelpRequested()
{
    if (var_map.count("help")) {
        #ifdef HELP_STRING
            cout << endl << HELP_STRING << endl;
        #endif
        string filename = argv[0];
        size_t found = filename.find_last_of("/\\");
        cout << endl << "Usage: " << filename.substr(found+1) << " [OPTIONS]" << endl;
        cout << endl << desc_generic << endl << desc_parameters << endl << desc_parameters_cmd << endl;
        return true;
    }
    return false;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::VersionRequested()
{
    if (var_map.count("version")) {
        #ifdef VERSION_STRING
            cout << endl << VERSION_STRING << endl;
        #else
            string filename = argv[0];
            size_t found = filename.find_last_of("/\\");
            cout << endl << filename.substr(found+1) << " - development version" << endl << "compiled: " << __DATE__ << endl << endl;
        #endif
            return true;
    }
    return false;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

// Code to check data types in configuration header at compile time (recursive, since can't use loops in constexpr!)
constexpr bool CheckConfigurationTypes(const unsigned int index)
{
    return (
            (index >= sizeof(ConfigManagerConfiguration)/sizeof(ConfigManagerConfiguration[0])) ||
            (
               (  strcmp(ConfigManagerConfiguration[index][1],"switch")  == 0 ||
                  strcmp(ConfigManagerConfiguration[index][1],"string")  == 0 ||
                  strcmp(ConfigManagerConfiguration[index][1],"double")  == 0 ||
                  strcmp(ConfigManagerConfiguration[index][1],"int")     == 0 ||
                  strcmp(ConfigManagerConfiguration[index][1],"strings") == 0 ||
                  strcmp(ConfigManagerConfiguration[index][1],"doubles") == 0 ||
                  strcmp(ConfigManagerConfiguration[index][1],"ints")    == 0    )
               && CheckConfigurationTypes(index+1)                     )
           );
};

constexpr bool CheckConfigurationModes(const unsigned int index)
{
    return (
            (index >= sizeof(ConfigManagerConfiguration)/sizeof(ConfigManagerConfiguration[0])) ||
            (
               (  strcmp(ConfigManagerConfiguration[index][3],"all")        == 0 ||
                  strcmp(ConfigManagerConfiguration[index][3],"all-nohelp") == 0 ||
                  strcmp(ConfigManagerConfiguration[index][3],"cmd")        == 0 ||
                  strcmp(ConfigManagerConfiguration[index][3],"cmd-nohelp") == 0 ||
                  strcmp(ConfigManagerConfiguration[index][3],"cfg")        == 0    )
               && CheckConfigurationModes(index+1)                     )
           );
};

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

void ConfigManager::CreateCustomDescription()
{
    // Throw compile time (!) error if configuration includes a wrong type or mode
    static_assert(CheckConfigurationTypes(0),"Encountered invalid data type in Config Manager Configuration, please check config header!");
    static_assert(CheckConfigurationModes(0),"Encountered invalid option mode in Config Manager Configuration, please check config header!");


    for(unsigned int i = 0; i < sizeof(ConfigManagerConfiguration)/sizeof(ConfigManagerConfiguration[0]); i++)
    {
        const char* name = ConfigManagerConfiguration[i][0];
        const char* type = ConfigManagerConfiguration[i][1];
        const char* description = ConfigManagerConfiguration[i][2];
        const char* mode = ConfigManagerConfiguration[i][3];

        if (strcmp(mode,"all") == 0)        AddDescriptionEntry(desc_parameters,name,type,description);
        if (strcmp(mode,"all-nohelp") == 0) AddDescriptionEntry(desc_parameters_hidden,name,type,description);
        if (strcmp(mode,"cmd") == 0)        AddDescriptionEntry(desc_parameters_cmd,name,type,description);
        if (strcmp(mode,"cmd-nohelp") == 0) AddDescriptionEntry(desc_parameters_cmd_hidden,name,type,description);
        if (strcmp(mode,"cfg") == 0)        AddDescriptionEntry(desc_parameters_cfg,name,type,description);
    }
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

void ConfigManager::AddDescriptionEntry(options_description &desc_return, const char* name, const char* type, const char* description)
{
    if (strcmp(type,"switch") == 0) desc_return.add_options()(name, description);
    if (strcmp(type,"string") == 0) desc_return.add_options()(name, value<string>(), description);
    if (strcmp(type,"double") == 0) desc_return.add_options()(name, value<double>(), description);
    if (strcmp(type,"int") == 0)    desc_return.add_options()(name, value<int>(),    description);

    string new_description = " (can be specified multiple times)";
    description = new_description.insert(0,description).c_str();
    if (strcmp(type,"strings") == 0) desc_return.add_options()(name, value<vector<string>>()->composing(), description);
    if (strcmp(type,"doubles") == 0) desc_return.add_options()(name, value<vector<double>>()->composing(), description);
    if (strcmp(type,"ints") == 0)    desc_return.add_options()(name, value<vector<int>>()->composing(),    description);
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(int &option, const char* name)
{
    CheckNameAndType(name,"int");

    if (!var_map.count(name)) return false;

    option =  var_map[name].as<int>();
    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(int &option, string name)
{
    return GetOption(option, name.c_str());
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(vector<int> &option, const char* name)
{
    CheckNameAndType(name,"ints");

    if (!var_map.count(name)) return false;

    option =  var_map[name].as<vector<int>>();
    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(vector<int> &option, string name)
{
    return GetOption(option, name.c_str());
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(double &option, const char* name)
{
    CheckNameAndType(name,"double");

    if (!var_map.count(name)) return false;

    option =  var_map[name].as<double>();
    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(double &option, string name)
{
    return GetOption(option, name.c_str());
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(vector<double> &option, const char* name)
{
    CheckNameAndType(name,"doubles");

    if (!var_map.count(name)) return false;

    option =  var_map[name].as<vector<double>>();
    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(vector<double> &option, string name)
{
    return GetOption(option, name.c_str());
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(string &option, const char* name)
{
    CheckNameAndType(name,"string");

    if (!var_map.count(name)) return false;

    option =  var_map[name].as<string>();
    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(string &option, string name)
{
    return GetOption(option, name.c_str());
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(vector<string> &option, const char* name)
{
    CheckNameAndType(name,"strings");

    if (!var_map.count(name)) return false;

    option =  var_map[name].as<vector<string>>();
    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(vector<string> &option, string name)
{
    return GetOption(option, name.c_str());
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(bool &option, const char* name)
{
    CheckNameAndType(name,"switch");

    option = false;
    if (!var_map.count(name)) return false;

    option =  true;
    return true;
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

bool ConfigManager::GetOption(bool &option, string name)
{
    return GetOption(option, name.c_str());
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

void ConfigManager::CheckNameAndType(const char* name,const char* type)
{
    bool found = false;
    bool correct_type = false;
    for (unsigned int i = 0; i < sizeof(ConfigManagerConfiguration)/sizeof(ConfigManagerConfiguration[0]); i++)
    {
        if (strcmp(ConfigManagerConfiguration[i][0],name) == 0)
        {
            found = true;
            if (strcmp(ConfigManagerConfiguration[i][1],type) == 0) correct_type = true;
        }
    }
    if (!found)
    {
        cerr << endl << "WARNING: Requested variable \"" << name
             << "\" without it being defined in ConfigManager configuration."
             << endl << "\t Continuing..." << endl;
    }
    else
    {
        if (!correct_type)
        {
            string error_message = "Requested incorrect datatype \"";
            error_message.append(type);
            error_message.append("\" for option \"");
            error_message.append(name);
            error_message.append("\".");
            CustomException new_exc(error_message.c_str());
            throw new_exc;
        }
    }
}

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._
