#include "file_parse.h"
#include "ray_trace.h"
    extern config_data cfg_data;
    extern config cfg;
    extern image_window image_w;
    extern lens_window lens_w;
    extern lensSystem  lens_sys;
    extern Ray_Tracer   rt;
    extern int microlens;
    extern int interactive;
    extern int autofocus;
    
  void config::add_string( const string &str )
  { 
    _strings.push_back( str );
  }

  void config::set_switch( const string &name, const string &value )
  { 
    if ( name == "config" ) 
      parse_config_file( value );
    //cout<<name<<"  "<<value<<" "<<endl;
    _switches[name] = value;
    _multi_switches.insert( pair<string,string> (name,value) );
  }

//parse -config file
  bool config::parse_config_file( const string &filename )
  {
    FILE *f = fopen( filename.c_str(), "r" );
    if ( f==NULL ) {
      cout<< "could not read config file " << filename << endl;
      return false;
    }

    char line[600];
    while (!feof( f )) {
    	if ( fgets( line, 500, f ) == NULL ) 
    	break;

      	char *name = NULL;
      	char *value = NULL;
      	int count = sscanf( line, "%as %as\n", &name, &value );
     	if ( count == 0 ) 
			continue;

      	string svalue = "";
      	if ( count > 1 && value != NULL ) {
			svalue = value;
			free( value );
      	}

      	if ( name != NULL ) {
			if ( name[0] == '#' ) 
	  			continue;

			string sname = name;
			if ( svalue != "" ) 
	  			set_switch( sname, svalue );
			else 
	  		add_string( sname );
		}	  	
    }
    return true;
  }
//find -config file
bool config::parse_command_line( int argn, char **argv ){
    
    int pos = 1;
    while ( pos < argn ) {

      char *name = NULL;
      char *value = NULL;
      string svalue = "1";
      if ( strlen( argv[pos] ) > 0 ) {

		if ( argv[pos][0] == '-' ) {
		  // look for argument value
		  name = argv[pos] + 1;

		  if ( pos+1 < argn ) {
			value = argv[pos+1];
			svalue = value;
		  }

		  // set argument switch
		  string sname( name );
		  set_switch( sname, svalue );
		  pos ++;
		}
		else {
			name = argv[pos];
			string sname = name;
			if ( sname != "" ) 
			add_string( argv[pos] );
		}
      }
      pos++;
    }
    return true;
return false;

}

  // single switch (return string)
  bool config::get_switch( const string &name, string &value ) const
  {
    map<string,string>::const_iterator it = _switches.find( name );
    if ( it != _switches.end() ) {
      value = (*it).second;
      return true;
    }
    return false;
  }

  bool config::get_switch( const string &name, int &value ) const
  {
    map<string,string>::const_iterator it = _switches.find( name );
    if ( it != _switches.end() ) {
      value = atoi( (*it).second.c_str() );
    }
    return false;
  }

  bool config::get_switch( const string &name, float &value ) const
  {
    map<string,string>::const_iterator it = _switches.find( name );
    if ( it != _switches.end() ) {
      //To do:: fix for locales which use "," delimiter
      // always force "."
      std::istringstream istr( (*it).second );
      istr.imbue(std::locale("C"));
      istr >> value;
    }
    return false;
  }

  bool parse_config_data(config cfg, config_data& cfg_data){

    //---------load mode-------------------------
    cfg.get_switch( "interactive",interactive);
    cfg.get_switch( "microlens",microlens);
    cfg.get_switch( "autofocus", autofocus);
    //---------load lens configuration-----------      
    cfg.get_switch( "lens_file",    cfg_data._lens_disc_file );    
    lens_sys.load(cfg_data._lens_disc_file.c_str());
    lens_sys.parse_spec(cfg_data._lens_disc_file.c_str());


    //---------load optics configuration---------
    if (interactive==1){
        cfg.get_switch( "pt_z",lens_w.lvoPlane.z);
        cfg.get_switch( "pt_y",lens_w.lvoPlane.y);
        cfg.get_switch( "pt_x",lens_w.lvoPlane.x);
    }
    else {
        cfg.get_switch( "pt_z_d" , lens_w.lvoPlane.z);
        lens_w.lvoPlane.y=0;
        lens_w.lvoPlane.x=0;
    }

    if (!(autofocus==1)){
        cfg.get_switch( "image_plane_pos_z",  lens_w.lviPlane.z);
        cfg.get_switch( "image_plane_pos_h",  lens_w.lviPlane.y);
    }

    //---------load microlens array configuration
    cfg.get_switch( "ml_r",  ml.r);
    cfg.get_switch( "ml_f",  ml.f);
    cfg.get_switch( "ml_w",  ml.w);
    cfg.get_switch( "ml_h",  ml.h);
    cfg.get_switch( "ml_z",  ml.z);
    
     
    //---------load display configuration-----------
    cfg.get_switch( "samples1_num", rt.light_s.sample_low_num);
    cfg.get_switch( "samples2_num", rt.light_s.sample_high_num);
    cfg.get_switch( "window1_w", lens_w.lvWidth);
    cfg.get_switch( "window1_h", lens_w.lvHeight);

    if (interactive==1) {
        cfg.get_switch( "window2_w", image_w.w);
        cfg.get_switch( "window2_h", image_w.h);

        cfg.get_switch( "image_x", image_w.ray_w);
        cfg.get_switch( "image_y", image_w.ray_h);
    }
    else {
        cfg.get_switch( "sensor_w", image_w.ray_w);
        cfg.get_switch( "sensor_h", image_w.ray_h);

        cfg.get_switch( "window2_array_h", image_w.h);
        image_w.w=image_w.h*image_w.ray_w/image_w.ray_h;
        //DON'T DISPLAY
    }
     
    return true;
}


