#include <vector>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "imdb.h"
#include "path.h"
using namespace std;

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */


void printPath(path& p){

}

path getShortestPath(const string& startActor, const string& goalActor, imdb& db){
  list<path> partialPaths = list<path>();
  set<string> previouslySeenActors = set<string>();
  set<film> previouslySeenFilms = set<film>();
  path initialPath = path(startActor);
  partialPaths.push_back(initialPath);
  while(!partialPaths.empty() && partialPaths.front().getLength() <= 5) { //this should be checked
    path thisPath = partialPaths.front();
    partialPaths.pop_front();
    string thisActor = thisPath.getLastPlayer();
    vector<film> thisActorMovies = vector<film>();
    db.getCredits(thisActor, thisActorMovies);
    //need to do a check here to make sure that the start actor was foudn
    for(unsigned int i = 0; i < thisActorMovies.size(); i++){
      film currMovie = thisActorMovies[i];
      if (previouslySeenFilms.insert(currMovie).second){
	vector<string> otherActors = vector<string>();
	db.getCast(currMovie, otherActors);
	//need to do a check to make sure that the other movies work
	for(unsigned int j= 0; j < otherActors.size(); j++){
	  string otherActor = otherActors[j];
	  if(previouslySeenActors.insert(otherActor).second){
	    path newPath = thisPath; //not sure what type of copy to do here
	    newPath.addConnection(currMovie, otherActor);
	    if(otherActor == goalActor) { //not sure we can do equals here
	      return newPath;
	    } else {
	      partialPaths.push_back(newPath);
	    }

	  }
	}
      }
    }
  }
  path returnPath = path("");
  return returnPath;
}



int main(int argc, const char *argv[])
{
  imdb db(determinePathToData(argv[1])); // inlined in imdb-utils.h
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    exit(1);
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      path foundPath = getShortestPath(source,target,db);
      if (foundPath.getLength() == 0 && foundPath.getLastPlayer() == "")
	cout << endl << "No path between those two people could be found." << endl << endl;
      else cout << foundPath << endl << endl;
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}

