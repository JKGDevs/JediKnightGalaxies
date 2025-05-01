// cppStringHelp.cpp -- provides commonly useful functions for std::string
#include "cppStringHelp.h"



/* 
--------------------
SPACES & WHITESPACE
--------------------
*/

//find the location of the last space from the start of the string
//returns -1 if no spaces found
int findLastSpaceInString(const std::string& s)
{
	int spaceFromEnd = -1;

	//check for space
	for (int i = s.length(); i > -1; i--)
	{
		if (s[i] == ' ')
		{
			spaceFromEnd = i;
			break;
		}
	}

	//no spaces found in string
	if (spaceFromEnd == -1)
		return -1;

	return spaceFromEnd;
}

//returns a string with all extra spaces stripped out
//eg: " bantha  fodder      is gross  " = "bantha fodder is gross"
std::string removeExtraSpacesInString(const std::string& s)
{
	std::string output_string;
	bool foundSpace = false; // Flag to check if spaces have occurred

	for (size_t i = 0; i < s.length(); ++i)
	{
		if (s[i] != ' ')
		{
			if (foundSpace)
			{
				if (s[i] == '.' || s[i] == '?' || s[i] == ',')
					;// Do nothing
				else
					output_string += ' ';

				foundSpace = false;
			}
			output_string += s[i];
		}
		else if (i > 0 && s[i - 1] != ' ')
			foundSpace = true;
	}
	return output_string;
}

// trim from start (left side)
std::string ltrimer(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {return !std::isspace(ch); }));
	return s;
}

// trim from end (right side)
std::string rtrimer(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch);}).base(), s.end());
	return s;
}

// trim from both ends
std::string trimer(std::string& s)
{
	ltrimer(s);
	rtrimer(s);
	return s;
}

//remove all whitespace from string
std::string removeWS(std::string& s)
{
	//s.erase(std::find(s.begin(), s.end(), ' '));
	s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());	//note: isspace is technically unsafe due to unicode chars
	return s;
}