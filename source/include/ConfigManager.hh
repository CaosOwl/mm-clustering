//
// ***************************************************************************
// **** General-Purpose ConfigManager class used to manage configurations ****
// ***************************************************************************
//
// $Id: ConfigManager.hh Wed 2016-10-26 12:02:18 UTC Michael W. Heiss $
//
/// \file ConfigManager.hh
/// \brief Definition of the ConfigManager class

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

// Avoid double-loading of the header (note #endif at EOF)
#ifndef ConfigManager_h
#define ConfigManager_h

// include program_options from boost libs
#include <boost/program_options.hpp>
using namespace boost::program_options;

// include std libraries
#include <list>
#include <iostream>
#include <fstream>
using namespace std;

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

class  ConfigManager
{
	public:
		ConfigManager(int, char**);
		virtual ~ConfigManager();
		bool	Initialize();
        bool GetOption(int&,const char*);
        bool GetOption(vector<int>&,const char*);
        bool GetOption(double&,const char*);
        bool GetOption(vector<double>&,const char*);
        bool GetOption(string&,const char*);
        bool GetOption(vector<string>&,const char*);
        bool GetOption(bool&,const char*);
        bool GetOption(int&,string);
        bool GetOption(vector<int>&,string);
        bool GetOption(double&,string);
        bool GetOption(vector<double>&,string);
        bool GetOption(string&,string);
        bool GetOption(vector<string>&,string);
        bool GetOption(bool&,string);

	private:
	    int		    argc;
	    char**      argv;
	    options_description desc_generic;
	    options_description desc_parameters;
	    options_description desc_parameters_hidden;
	    options_description desc_parameters_cmd;
	    options_description desc_parameters_cmd_hidden;
	    options_description desc_parameters_cfg;
	    options_description cmdline_options;
        options_description cfgfile_options;
        positional_options_description desc_positional;
        variables_map var_map;
        bool HelpRequested();
        bool VersionRequested();
        void CreateCustomDescription();
        void AddDescriptionEntry(options_description&,const char*,const char*,const char*);
        void CheckNameAndType(const char*,const char*);
};

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

// Declare custom exception
class CustomException: public exception
{
    public:
        CustomException(const char* text) noexcept { exctext = text; }
        virtual const char* what() const noexcept { return exctext; }
    private:
        const char* exctext;
};

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

#endif
