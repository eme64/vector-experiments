// Header only for CLI

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <getopt.h>
#include <algorithm>
#include <iomanip>

// -------------------- Idea:
//
// Takes input from argc, argv
//
// -------------------- How to use Cli:
// Cli cli(argc,argv,"peterem");
// cli.addFlag('x',"xopt","Some test flag");
// cli.addFlag('y',"yopt","Some test flag");
// cli.addOption('z',"zopt","defaultv","Some test option");
// cli.addOption('Z',"z-opt-two","defaultv","Some test option");
// cli.parse();
// 
// std::cout << "flag x: " << cli.flag('x') << "\n";
// std::cout << "flag y: " << cli.flag('y') << "\n";
// std::cout << "option z: " << cli.option('z') << "\n";
// std::cout << "option Z: " << cli.option('Z') << "\n";

#ifndef HEADER_CLI_HPP
#define HEADER_CLI_HPP

class Cli {
public:
   Cli(int argc, char** argv, std::string name = "")
   : argc_(argc), argv_(argv), name_(name) {
      addFlag('h',"help","print this help message");
   }
   
   // call after all flags, options, params declared
   bool parse() {
      signed char c; // character
      extern char *optarg; // getopt - optional string
      int option_index = -1;

      long_options_.push_back({0,0,0,0});
      
      while ((c = getopt_long(argc_, argv_, parse_.c_str(),
		              long_options_.data(), &option_index)) != -1) {
	 if(c=='?') {
	    usage();
	    std::exit(0);
	 }
	 std::string key = "";
	 if(option_index >= 0) {
            key = long_options_[option_index].name;
	 } else {
	    auto it = opt2long_.find(c);
	    if(it!=opt2long_.end()) {
	       key = it->second;
	    } else {
	       usage();
	       std::exit(0);
	    }
	 }

	 //std::cout << "getopt " << c << " idx: " << option_index << " key: " << key << "\n";
	 
	 if(!handleArg(key,optarg)) {
	    usage();
	    std::exit(0);
	 }
	 option_index = -1;
      }
      
     
      /* Print any remaining command line arguments (not options). */
      if (optind < argc_) {
	 std::cout << "Unexpected arguments:\n";
         while (optind < argc_)
            std::cout << "  " << argv_[optind++] << "\n";
         usage();
	 std::exit(0);
      }

      return true;
   }
   
   // internal: handles input from cl, assigns to flags, options, params
   bool virtual handleArg(const std::string &optlong, char* opt_arg) {
      if(optlong.compare("help")==0) {
	 usage();
         std::exit(0);
      }
      {// try find flag:
         auto it = flags_.find(optlong);
         if(it!=flags_.end()) {
            it->second = !(it->second);
            return true;
         }
      }
      {// try find option:
         auto it = option_.find(optlong);
         if(it!=option_.end()) {
            it->second = std::string(opt_arg);
            return true;
         }
      }
      
      std::cout << "arg not found: " << optlong << "\n";
      return false; // nothing found.
   }
   
   // print usage to screen
   void usage() {
      std::cout << "Usage: " << name_ << std::endl;

      for(std::map<std::string, std::string>::iterator it = desc_.begin(); it!=desc_.end(); it++) {
	 if(long2opt_[it->first]>0) {
	    std::cout << " -" << long2opt_[it->first] << " --";
	 } else {
	    std::cout << "    --";
	 }
	 std::cout << std::left << std::setw(30) << it->first;
	 std::cout << " " << it->second;
	 
	 auto itf = flags_.find(it->first);
	 if(itf!=flags_.end()) {
	    std::cout << " (flag)";
	 }
	 
	 std::cout << std::endl;
      }
   }
   
   void checkOpt(signed char opt) {
      if(opt==0) {return;}
      auto it = opt2long_.find(opt);
      if(it!=opt2long_.end()) {
         std::cout << "Error: -" << opt << " already exists!\n";
         std::exit(0);
      }
   }
   void checkOptLong(const std::string &optlong) {
      auto it = long2opt_.find(optlong);
      if(it!=long2opt_.end()) {
         std::cout << "Error: --" << optlong << " already exists!\n";
         std::exit(0);
      }
   }

   void addFlag(signed char opt, const std::string &optlong, std::string desc) {
      checkOpt(opt);
      checkOptLong(optlong);
      desc_.insert(std::pair<std::string,std::string>(optlong,desc));
      flags_.insert(std::pair<std::string,bool>(optlong,false));
      
      if(opt!=0) {
         parse_ += opt;
         opt2long_.insert(std::pair<signed char,std::string>(opt,optlong));
      }
      long2opt_.insert(std::pair<std::string,signed char>(optlong,opt));
      
      // did not just take optlong, because it may disappear. opt2long_[opt] should persist.
      long_options_.push_back({desc_.find(optlong)->first.c_str(),no_argument,0,opt});
   }

   bool flag(const std::string &optlong) {
      auto it = flags_.find(optlong);
      if(it!=flags_.end()) {
         return it->second;
      } else {
         std::cout << "Error: flag " << optlong << " never declared!\n";
         return false;
      }
   }

   void addOption(signed char opt, const std::string &optlong, std::string def, std::string desc) {
      checkOpt(opt);
      checkOptLong(optlong);
         
      desc += " (default: " + def + ")";
      desc_.insert(std::pair<std::string,std::string>(optlong,desc));
      option_.insert(std::pair<std::string,std::string>(optlong,def));
      
      if(opt!=0) {
         parse_ += opt;
         parse_ += ":";
         opt2long_.insert(std::pair<signed char,std::string>(opt,optlong));
      }
   
      long2opt_.insert(std::pair<std::string,signed char>(optlong,opt));
      
      // did not just take optlong, because it may disappear. opt2long_[opt] should persist.
      long_options_.push_back({desc_.find(optlong)->first.c_str(),required_argument,0,opt});
   }

   std::string option(const std::string &optlong) {
      auto it = option_.find(optlong);
      if(it!=option_.end()) {
         return it->second;
      } else {
         std::cout << "Error: option " << optlong << " never declared!\n";
         return "";
      }
   }

   bool isUsed(signed char opt) {return opt2long_.find(opt)!=opt2long_.end();}
   bool isUsedLong(const std::string &optlong) {return desc_.find(optlong)!=desc_.end();}

   std::string getPath(){
       std::string s(argv_[0]);
       return s;
   }

   std::string getPathFromExec(){
       std::string path = getPath();
       std::string res = "";
       reverse(path.begin(), path.end());
       size_t pos = path.find('/');
       // the executable is not in the current directory
       if(pos != std::string::npos){
           reverse(path.begin(), path.end());
           res = path.substr(0, path.length() - pos);
       }
       return res;
   }
    
protected:
   // arguments for main
   const int argc_;
   char** argv_;
   
   // App name
   const std::string name_;
   
   ///  // string listing the short opt characters
   std::string parse_ = "";
   // array of structs for long options
   std::vector<struct option> long_options_;
   
   std::map<std::string, std::string> desc_; // desc for usage display
   std::map<signed char, std::string> opt2long_; // long option name
   std::map<std::string, signed char> long2opt_; // long option name
   std::map<std::string, bool> flags_; // bool for flags, only exists if flag exists
   std::map<std::string, std::string> option_; /// string for a option, exists if option exists
};



#endif // HEADER_CLI_HPP

