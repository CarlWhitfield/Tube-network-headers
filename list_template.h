#ifndef LIST_TEMPLATE_H
#define LIST_TEMPLATE_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "file_manip.h"
#include "globals.h"

//classes for reading and storing options and parameters

namespace inlist
{
	//generic template for options and params lists
	template <class T1, class T2>
	class List
	{
	protected:
		std::unordered_map<std::string, T1> dict1;
		std::unordered_map<std::string, T2> dict2;
	public:
		//functions
		List(){};

		void read_file(const std::string &);
		//parse template function
		bool parse(const std::string &name, const std::string &nc);
		inline void add(const std::string & name, const T1 & entry){ this->dict1[name] = entry; }
		inline void add(const std::string & name, const T2 & entry){ this->dict2[name] = entry; }
		template<class T3> T3 get(const std::string & name){ return NULL; }
		template<> T1 get<T1>(const std::string & name){ return this->dict1[name]; }
		template<> T2 get<T2>(const std::string & name){ return this->dict2[name]; }
	};

	template<class T1, class T2> 
	void List<T1,T2>::read_file(const std::string &filename)  //constructer with filename argument
	{
		char buff[1000];   //stores line
		unsigned buffsize = 1000;   
		std::string temp, nc;   //stores parsed info
		std::stringstream ss;
	
		if (!check_infile(filename))
		{
			std::ifstream infile;
			infile.open(filename);
			while (infile.good())                            //read
			{
				infile.getline(buff, buffsize);             //read line from file
				ss.str(buff);
				temp.clear();
				ss >> std::skipws >> temp;                //parse line
				if (temp.size() > 0 && temp[0] != '%')
				{
					ss >> std::skipws >> nc;                     //read value into string
					if (!parse(temp, nc))    //send to parse option
					{
						std::cout << "Did not recognise option/parameter " << temp << ".\n";
					}
				}
				ss.str("");
				ss.clear();				//throw away rest of line after
			}
			infile.close();
		}
	}

	template <class T1, class T2>
	inline bool List<T1, T2>::parse(const std::string &name, const std::string &nc)
	{
		//check it is a valid option
		if(dict1.find(name) != dict1.end()) 
		{
			dict1[name]->read(nc);
			return true;
		}
		if(dict2.find(name) != dict2.end()) //check it is a valid option
		{
			dict2[name]->read(nc);
			return true;
		}

		return false;
	}

	//template for generic input, can be optino or parameter
	template <typename T>
	class Input{
	protected:
		T Default_value, value;
		std::string name;
	public:
		Input(){};
		Input(const T & val, const std::string & nam)
		{
			value = val;
			Default_value = val;
			name = nam;
		}

		T get_value(){ return this->value; }
		inline std::string get_name(){ return this->name; }
		inline void set_default_value(const T & val)
		{ 
			Default_value = val;
		}
		virtual void set_to_default()
		{ 
			value = Default_value;
		}
		virtual void update_value( const T & v ){ value = v; }
		virtual void read(const std::string &){};
		virtual std::string print(){return "Undefined";}
	};

	//option class
	template <typename T>
	class Option: public Input<T>{
	public:
		std::vector<T> possible_values;
		std::vector<std::string> value_names;
		std::string print()
		{
			for (unsigned i = 0; i < ((unsigned) possible_values.size()); i++)
			{
				if (this->value == possible_values[i])
				{
					return value_names[i];
				}
			}
			return "unknown";
		}

		Option<T>():Input<T>(){};
		Option<T>(const T & val, const std::string & nam):Input<T>(val, nam){};
		Option<T>(const T & val, const std::string & nam, const T *option_list,
				  const std::string *option_name_list, const int & option_count):Input<T>(val, nam)
		{
			possible_values = vectorise(option_list, option_count);
			value_names = vectorise(option_name_list, option_count);
		}
		void read(const std::string & code){};
	};

	//character option -- for multiple choice options
	template<> void Option<char>::read(const std::string & code)
	{
			bool error = true;
			for (unsigned i = 0; i < ((unsigned) possible_values.size()); ++i)
			{
				if(code[0] == possible_values[i]) 
				{
					this->value = possible_values[i];
					error = false;
				}
			}
			if(error)
			{
				std::cout << name << " option code " << code << " not recognised\n";
			}
	}

	//boolian option
	template<> void Option<bool>::read(const std::string & code)
	{
		switch(code[0])
		{
			case 't':
			case 'T':
			{
				this->value = true;
			} break;
		
			case 'f':
			case 'F':
			{
				this->value = false;
			} break;
		
			default:
			{
				std::cout << name << " option code " << code << " not recognised\n";
			}
		}
	}

	//parameter -- for storing values that may have physical units
	template <typename T>
	class Parameter: public Input<T>
	{
	public:
		Parameter<T>():Input<T>(){};
		Parameter(const T & val, const std::string & nam):Input<T>(val, nam){};
		virtual void read(const std::string & code){};

		virtual void set_conversion( T * ){};
		virtual bool isOK(){ return true; }
		virtual T get_phys_value(){ return this->value; }
		virtual T get_SI_value(){ return this->value; }
		virtual void set_phys_value( const T & val ){ this->value = val; }
		virtual void calc_sim_from_phys_value(){};
		virtual std::string phys_value_string(){ return ""; }
	};

	//integer parameters
	template<> void Parameter<int>::read(const std::string &c){ value = atoi(c.c_str());}  //this is common to all params of type double
	
	//double parameters
	template<> void Parameter<double>::read(const std::string &c){ value = atof(c.c_str()); }

	//template for list of two types of options
	template<typename T1, typename T2> class OptionList: public List<Option<T1>*, Option<T2>*>
	{
	protected:
		std::unordered_map<std::string, std::vector<std::string>> filenames;
	public:

		OptionList(){};   //default constructor is blank
		template<typename T3> Option<T3>* get_option(const std::string & name){};
		template<> Option<T1>* get_option<T1>(const std::string & name){ return this->dict1[name]; }
		template<> Option<T2>* get_option<T2>(const std::string & name){ return this->dict2[name]; }

		inline void add_filename(const std::string & code, const std::string & fname)
		{ 
			if(!this->filename_exists(code)) this->filenames[code] = std::vector<std::string>(); 
			this->filenames[code].push_back(fname);
		}
		inline size_t count_files_with_ext(const std::string & code){ return this->filenames[code].size(); }
		inline std::string get_filename(const std::string & code)
		{ 
			if(this->filenames[code].size() > 1) std::cout << "Warning, more than one file with extension " << code << '\n';
			return this->filenames[code][0]; 
		}
		inline std::string get_filename(const std::string & code, const size_t & n){ return this->filenames[code][n]; }
		inline bool filename_exists(const std::string & code){ return (this->filenames.find(code) != this->filenames.end()); }
		void get_filenames_from_args(const std::vector<std::string> & extensions, const int &argc, char** argv);
	};

	template<typename T1, typename T2> void OptionList<T1,T2>::get_filenames_from_args(const std::vector<std::string> & extensions, const int &argc, char** argv)
	{
		//arguments sorted into ordered map, where keyword is their file extension
		for(int n = 1; n < argc; n++)
		{
			std::string fname = argv[n];
			size_t i = fname.rfind('.', fname.length());
			std::string ext = fname.substr(i+1, fname.length()-1);   //get file extension
			bool sorted = false;
			for (unsigned m = 0; m < extensions.size(); m++)
			{
				if(extensions[m] == ext)  //if filename extension matches
				{
					this->add_filename(ext, fname);
					sorted = true;
				}
			}
			if (!sorted)
			{
				std::cout << "Input " << fname << " not used -- extension not recognised.\n";  //error message
			}
		}
		//tested
	}

	//template for two types of parameters
	template<typename T1, typename T2> class ParameterList: public List<Parameter<T1>*, Parameter<T2>*>
	{
	protected:
		std::unordered_map<std::string, double> conversions_phys_to_sim;
		inline void check_OK()
		{
			for (auto it = this->dict1.begin(); it!= this->dict1.end(); ++it)
			{
				if(!it->second->isOK()) it->second->set_to_default();
			}
			for (auto it =  this->dict2.begin(); it!= this->dict2.end(); ++it)
			{
				if(!it->second->isOK()) it->second->set_to_default();
			}
		}
	public:
		ParameterList(){};    //default constructor uses pre-defined default params
		template<typename T3> Parameter<T3>* get_param(const std::string & name){};
		template<> Parameter<T1>* get_param<T1>(const std::string & name){ return this->dict1[name]; }
		template<> Parameter<T2>* get_param<T2>(const std::string & name){ return this->dict2[name]; }

		inline void set_conversion(const std::string & name, const double & val){ this->conversions_phys_to_sim[name] = val; }
		inline double get_conversion(const std::string & name){ return (this->conversions_phys_to_sim[name]); }
		inline double* get_conversion_ptr(const std::string & name){ return &(this->conversions_phys_to_sim[name]); }
		virtual void check_and_convert(OptionList<char, bool> *){ this->check_OK(); }
	};

}

#endif