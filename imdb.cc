using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

/**
 * Convenience struct for passing in a key to bsearch that contains both 
 * the search term and the file location, so that a comparison function
 * can do the proper pointer arithmetic to locate the record within the
 * file on the hard drive
 */

struct bsearchKey {
 const void* key;
 const void* file;
};


/**
 * Another convenience struct for populating either a movie or actor
 * record from the data file.
 * 
 */

struct fRecord {
  char* name; /// The ponter to the base of the record (as well as the name)
  short numContents; /** The number of other records that the record points to.
		      *  e.g. for actors, the number or movies that actor has
		      *  been in
		      */
  int year; /// The year a movie was made (nonsensical for actor records)
  int* offsets; /** A pointer to the base of the integer array containing the
		 *  offsets in the other file for the linked records.
		 *
		 *  e.g. for actor records, the base of the array of offsets into
		 *  the movie file for the movies that the referenced actor
		 *  appeared in
		 */
};


/**
 * Helper method for getRecord.
 *
 * @param type An int representing the type of record (actor or movie) - use
 * imdb class defined constants ACTOR and MOVIE when passing in values
 *
 * @param nameLength The length of the name of the actor or movie associated
 * with the record
 *
 * @return the padding needed to be added to the length of the name of the
 * movie or actor to produce an appropriate offset from the base of a record
 * to access the number of elements within the array of offsets held within
 * the record
 */

size_t getShortPadding(int type, size_t nameLength){
  size_t padding;
  if(type == imdb::ACTOR){
    if(nameLength % 2 == 1) padding = 1;
    else padding = 2;
  } else if(type == imdb::MOVIE){
    if(nameLength % 2 == 1) padding = 3;
    else padding = 2;
  } else {
    assert(type == imdb::ACTOR || type == imdb::MOVIE);
  }
  return padding;
}


/**
 * Retrieves records from a data file
 *
 * @param file A pointer to the base of the relevant, properly formatted
 * data file
 *
 * @param offset The offset (in bytes) from the base of the file where
 * the record being retrieved is stored
 *
 * @param type An int representing the type of record (actor or movie) - use
 * imdb class defined constants ACTOR and MOVIE when passing in values
 * 
 * @return An fRecord struct containing the salient details of the record
 */

fRecord getRecord(const void* file, size_t offset, int type){
  fRecord found;
  char* name = (char*)file + offset;
  size_t nameLength = strlen(name);
  found.name = name;
  found.year = 1900 + *(name + nameLength + 1); // Year will be nonsensical for actors
  size_t padding = getShortPadding(type, nameLength);
  short* numContentsPtr = (short*)(name + nameLength + padding);
  found.numContents = *numContentsPtr;
    
  int numBytes = (char*)(numContentsPtr) + 2 - name;
  if(numBytes % 4 == 0) padding = 0;
  else padding = 2;
    
  found.offsets = (int*)((char*) numContentsPtr + 2 + padding);  
  return found;
}

/**
 * Convenience function for converting an fRecord to a film
 *
 * @param rec The fRecord to be converted into a film struct
 *
 * @return The film struct representation of the fRecord
 */


film filmFromRecord(const fRecord& rec){
  film f;
  f.title = rec.name;
  f.year = rec.year;
  return f;
}

/**
 * A comparison function for use with bsearch to search through a data file to find
 * a specific actor
 *
 * @param pkey A pointer to a bsearchKey struct which contains a pointer to the
 * true "key" being searched for (the actor name) and a pointer to the
 * base of the data file containing the actor records
 *
 * @param pelem A pointer to the element being inspected within an array of int offsets
 *
 * @return an int following the comparison function paradigm (0 if equal, -1 if key <
 * pelem, 1 if key > pelem)
 *
 */

int compareActors(const void* pkey, const void* pelem){
  bsearchKey* bskey = (bsearchKey*)pkey; // Cast the pkey void* to a bsearchKey*
  char* actorNameKey = (char*)bskey->key; // Get the actors name from the key
  const void* actorFile = bskey->file; // Get the base of the actor file from the key
  int offset = *(int*)pelem; // Get the int offset from the array within the file
  char* foundNameKey = getRecord(actorFile,(size_t)offset, imdb::ACTOR).name;
  cout << "Searching for " << actorNameKey << ", found " << foundNameKey << endl;
  return strcmp(actorNameKey, foundNameKey);
}


/**
 * A comparison function for use with bsearch to search through a data file to find
 * a specific movie
 *
 * @param pkey A pointer to a bsearchKey struct which contains a pointer to the
 * true "key" being searched for (the film struct) and a pointer to the base of the
 * file containing the movie records
 *
 * @param pelem A pointer to the element being inspected within an array of int offsets
 *
 * @return an int following the comparison function paradigm (0 if equal, -1 if key <
 * pelem, 1 if key > pelem)
 *
 */

int compareMovies(const void* pkey, const void* pelem){
  bsearchKey* bskey = (bsearchKey*)pkey;
  film* filmKeyPtr = (film*)bskey->key;
  const void* movieFile = bskey->file;
  int offset = *(int*)pelem;
  film foundFilm = filmFromRecord(getRecord(movieFile, (size_t)offset,imdb::MOVIE));
  if (*filmKeyPtr == foundFilm) return 0;
  if (*filmKeyPtr < foundFilm) return -1;
  else return 1;
}



imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}




bool imdb::getCredits(const string& player, vector<film>& films) const { 
  int* foundID = searchFile(player.c_str(), actorFile, compareActors);
  if (foundID == NULL) return false;
  fRecord rec = getRecord(actorFile,*foundID, ACTOR);
  assert(strcmp(rec.name,player.c_str()) == 0);
  for (int i = 0; i  < rec.numContents; i++)
    films.push_back(filmFromRecord(getRecord(movieFile,rec.offsets[i],MOVIE)));
  return true; 
}





bool imdb::getCast(const film& movie, vector<string>& players) const { 
  int* foundID = searchFile(&movie, movieFile, compareMovies);
  if (foundID == NULL) return false;
  fRecord rec = getRecord(movieFile, *foundID, MOVIE);
  film foundFilm = filmFromRecord(rec);
  assert(foundFilm == movie);
  for(int i = 0; i < rec.numContents; i++)
    players.push_back(getRecord(actorFile,rec.offsets[i],ACTOR).name);
  return true; 
}


int* imdb::searchFile(const void* key, const void* file, int (*cmpr)(const void*, const void*)){
  bsearchKey bskey;
  bskey.file = file;
  bskey.key = key;
  int numElems = *(int*)file;
  int* base = (int*)file + 1;
  return (int*)bsearch(&bskey, base, numElems, sizeof(int), cmpr);
}










imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
