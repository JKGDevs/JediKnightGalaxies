// cppStringHelp.h -- provides commonly useful functions for std::string
#ifndef _JKG_STRINGHELP
#define _JKG_STRINGHELP
#include <string>
#include <algorithm> 
#include <cctype>
#include <locale>

//spaces & whitespace
int findLastSpaceInString(const std::string& s);				//find the location of the last space from the start of the string, returns -1 if no spaces found
std::string removeExtraSpacesInString(const std::string& s);	//returns a copy of a string with all extra spaces stripped out
std::string ltrimer(std::string& s);							//trim white space from leftside of string
std::string rtrimer(std::string& s);							//trim white space from rightside of string
std::string trimer(std::string& s);								//trim both sides of string
std::string removeWS(std::string& s);							//remove all whitespace
#endif