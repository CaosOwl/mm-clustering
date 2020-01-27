//
// ***************************************************************************
// **** General-Purpose ConfigManager class used to manage configurations ****
// ***************************************************************************
//
// $Id: ConfigManager.conf.hh Wed 2016-10-26 12:02:18 UTC Michael W. Heiss $
//
/// \file ConfigManager.conf.hh
/// \brief Configuration of the ConfigManager class

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

// Avoid double-loading of the header (note #endif at EOF)
#ifndef ConfigManager_conf
#define ConfigManager_conf

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

// Set the strings to be included in the standard help and version message here
// if you don't want to use these see comment below
#define VERSION_STRING "Analysis for NA64 visible mode data v0.1\n\n\t(C)(C)(C)(C)(C)(C)(C)(C)\n\t(C) Michael W. Heiss (C)\n\t(C) ETH Zürich - IPP (C)\n\t(C)(C)(C) 2016 (C)(C)(C)\n"
#define HELP_STRING "This program runs an analysis for na64 data and simulation"

// If you don't want custom help or version strings, but the standard ones uncomment the following lines
#undef VERSION_STRING
// #undef HELP_STRING

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

constexpr const char* ConfigManagerFileHandlerMode =
    // Set the mode for the handling of in- and output files here
    // The follwing use cases are possible:
    // "none"   for     ./program [OPTIONS]
    // "in"     for     ./program [OPTIONS] input-file
    // "ins"    for     ./program [OPTIONS] input-file1 input-file2 ...
    // "out"    for     ./program [OPTIONS] output-file
    // "outs"   for     ./program [OPTIONS] output-file1 output-file2 ...
    // "inout"  for     ./program [OPTIONS] input-file output-file
        "none"
;

constexpr const char* ConfigManagerDefaultConfigFile =
    // Set the path to the default config file to be used
    // if you don't want to use this feature, just set it to the empty string ""
    // Please note that the command-line options will always override the config file ones
        "config/default.cfg"
;

constexpr bool ConfigManagerOverrideConfigFile =
    // Set this to true if you want the option to use a custom config file per command-line option
    // Note that this will override values from the default config file
    // If you don't want to use this feature set it to false
    // This will add a Configuration entry like {"config","string","path to configuration file"}
         true
;

constexpr const char* ConfigManagerConfiguration[][4] =
{
    // List all possible configuration options here
    // use three strings {"name","type","description","mode"} for options and add a , between all lines
    // if the option is to be used without an argument use "switch"
    // single option entries can either be "string", "double" or "int"
    // for multiple option entries use "strings", "doubles" or "ints"
    // for short command-line options you can add a letter to the name, like so: "name,N"
    // the mode can be set to:
    // "all" : option can be used on the command-line and in the config file
    // "all-nohelp" : same as above, but is hidden in the help page
    // "cmd" : option can only be used at the command-line
    // "cmd-nohelp" : same as above, but is hidden in the help page
    // "cfg" : option can only be used in the config-file and is hidden in the help page
    // e.g.
    //  {"compression-level,c","int","sets the desired level of compression"},{"all"}
    //  ,
    //  {"include,I","strings","add include path"}
    // the standard options "version,v", "help,h" are already included!
        {"events","int","sets number of events to create / process","all"}
        ,
        {"outdir","string","select destination where to move the outputs","all"}
        ,
        {"use-file","string","use this file for multiplexing map and noise","all"}
        ,        
        {"verbose","int","set verbose level","all"}
        ,
        {"channels","int","set number of channels","all"}
        ,
        {"mfactor","int","set multiplex factor","all"}
        ,
        {"nclusters","int","Number of clusters per event","all"}
        ,        
        {"charge","int","set most probable charge of the landau","all"}
        ,
        {"amplitude","double","set amplitude of clusters in sigma","all"}
        ,
        {"from-file","switch","take multiplex map from file","all"}
        ,
        {"save-waveforms","switch","save all created cluster in histograms","all"}
        ,
        {"apply-minimization","switch","apply minimization procedure to strips","all"}
        ,        
        {"compression","int","compression level of the root file","all"}
};

// _.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-.__.-~+*^°^*+~-._

#endif
