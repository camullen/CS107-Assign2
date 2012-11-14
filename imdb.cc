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



struct bsearchKey {
 const void* key;
 const void* file;
};

struct fRecord {
    char* name;
    short numContents;
    int year;
    int* offsets;
};

fRecord getRecord(const void* file, size_t offset, int type){
  fRecord found;
  char* name = (char*)file + offset;
  size_t nameLength = strlen(name);
  found.name = name;
  size_t padding;

  if(type == imdb::ACTOR){
    if(nameLength % 2 == 1) padding = 2;
    else padding = 1;
  } else if(type == imdb::MOVIE){
    found.year = 1900 + *(name + nameLength + 1);
    if(nameLength % 2 == 1) padding = 3;
    else padding = 2;
  } else {
    assert(type == imdb::ACTOR || type == imdb::MOVIE);
  }
  
  short* numContentsPtr = (short*)(name + nameLength + padding);
  found.numContents = *numContentsPtr;
    
  int numBytes = (char*)(numContentsPtr) + 2 - name;
  if(numBytes % 4 == 0) padding = 0;
  else padding = 2;
    
  found.offsets = (int*)((char*) numContentsPtr + 2 + padding);  
  return found;
}

film filmFromRecord(const fRecord& rec){
  film f;
  f.title = rec.name;
  f.year = rec.year;
  return f;
}


int compareActors(const void* pkey, const void* pelem){
  bsearchKey* bskey = (bsearchKey*)pkey;
  char* actorNameKey = (char*)bskey->key;
  const void* actorFile = bskey->file;
  int offset = *(int*)pelem;
  char* foundNameKey = getRecord(actorFile,(size_t)offset, imdb::ACTOR).name;
  return strcmp(actorNameKey, foundNameKey);
}

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

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const { 
  int* foundID = searchFile(&player, actorFile, compareActors);
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
