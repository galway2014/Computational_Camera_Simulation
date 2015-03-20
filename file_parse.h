#ifndef _FILEPARSE_H_
#define _FILEPARSE_H_

#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <locale>
#include <sstream>
#include <map>
#include <iomanip>

using namespace std;
//configuration data should contains
//1. standard lenses descriptions file
//2. computational components desciption file
//3. optics configuration file
//4. rendering configuration file

typedef struct {

string _lens_disc_file;

float object_dist;
float image_dist;
//----5----
bool tracing_ray_dir;
int  sample_rate_2d;
int  sample_rate_4d;


}config_data;


class config
{
public:

// SETUP CONFIG

// read from file
bool parse_config_file( const string &filename );
// read from command line
bool parse_command_line( int argn, char **argv );

void add_string( const string &str );
void set_switch( const string &name, const string &value );
// single switch (return string)
bool get_switch( const string &name, string &value ) const;
// single switch (return integer)
bool get_switch( const string &name, int &value ) const;
// single switch (return float)
bool get_switch( const string &name, float &value ) const;
// single switch (return double)
bool get_switch( const string &name, double &value ) const;

private:  
    // global config strings
    std::vector<string> _strings;
    // switch map
    std::map<string,string> _switches;
    // switch map
    std::multimap<string,string> _multi_switches; 
};



bool parse_config_data(config cfg, config_data& cfg_data);

#endif

