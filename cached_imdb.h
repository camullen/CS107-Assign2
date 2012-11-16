#ifndef cached_imdb
#define cached_imdb

#include "imdb.h"
#include "imdb-utils.h"
#include <map>

using namespace std;

class cached_imdb: public imdb {
 public:
  cached_imdb(const string& directory){
    actorCache = map<string, vector<film> >();
    movieCache = map<film, vector<string> >();
  } 

  bool getCredits(const string& player, vector<film>& films) const{
    map<player, vector<film> >::iterator found actorCache.find(player);
    if (found == actorCache.end()){
      bool returnVal = imdb::getCredits(player, films);
      if (returnVal) actorCache[player] = films;
	return returnVal;
    } else 
  }
 private:
  map<string, vector<film> > actorCache;
  map<film, vector<string> > movieCache;

};


#endif
