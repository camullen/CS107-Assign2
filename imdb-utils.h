#ifndef __imdb_utils__
#define __imdb_utils__

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cctype>

using namespace std;

/**
 * Convenience struct: film
 * ------------------------
 * Bundles the name of a film and the year it was made
 * into a single struct.  It is a true struct in that the 
 * client is free to access both fields and change them at will
 * without issue.  operator== and operator< are implemented
 * so that films can be stored in STL containers requiring 
 * such methods.
 */

struct film {
  
  string title;
  int year;
  
  /** 
   * Methods: operator==
   *          operator<
   * -------------------
   * Compares the two films for equality, where films are considered to be equal
   * if and only if they share the same name and appeared in the same year.  
   * film is considered to be less than another film if its title is 
   * lexicographically less than the second title, or if their titles are the same 
   * but the first's year is precedes the second's.
   *
   * @param rhs the film to which the receiving film is being compared.
   * @return boolean value which is true if and only if the required constraints
   *         between receiving object and argument are met.
   */

  bool operator==(const film& rhs) const { 
    return this->title == rhs.title && (this->year == rhs.year); 
  }
  
  bool operator<(const film& rhs) const { 
    return this->title < rhs.title || 
      (this->title == rhs.title && this->year < rhs.year); 
  }
};

/**
 * Non-OS dependent function that gives the proper path to the binaries
 * appropriate for the endianness of the system
 * 
 * Requires that the data files be stored in: ./data
 * 
 * @return one of two data paths.
 */

inline const char *determinePathToData(const char *userSelectedPath = NULL)
{
  if (userSelectedPath != NULL) return userSelectedPath;
  
  //Check to see if running Windows - the directory slashes go the wrong way
  //The environment variable OS contains the operating system type on windows machines
  char* ostype = getenv("OS");
  if(ostype != NULL){
    //Convert the ostype variable to lowercase
    char ostypeLocal[strlen(ostype) + 1]; 
    strcpy(ostypeLocal, ostype);
    for(size_t i = 0; i < strlen(ostypeLocal); i++)
      ostypeLocal[i] = (char)tolower((int)ostypeLocal[i]);

    //Check to see if the variable is windows
    if(strstr("windows", ostypeLocal) != NULL)
      return ".\\data\\little-endian\\";
  }

  int testNum = 57;
  if(*(char*)&testNum == 57)
    return "./data/updated/little-endian/";
  else return "./data/updated/big-endian/";
}

#endif
