#ifndef RNFUNCTIONS_H
#define RNFUNCTIONS_H

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstddef>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;

// Replaces all instances of a pattern with a new pattern
std::string strReplaceAll(std::string origin, const std::string& pat, 
                          const std::string& newPat, const std::size_t start = 0);

std::string lowercase(std::string s);

// Print: old filename ---> new filename
void printFileChange(const fs::path& oldPath, const fs::path& newPath);

// Checks if map is empty and prints message if it is
bool checkForMatches(const Filenames& matchedPaths);

// Print number of matches and ask for second pattern
bool checkIfQuit(std::size_t index);

// Takes a path and removes the extension and dot at start
void removeDotEnds(fs::path& file, bool& dotAtStart);

// Restors the extension and dot at start
void restoreDotEnds(fs::path& newFile, const fs::path& file, 
                    bool dotAtStart);

// Rename a file using given two patterns
fs::path renameFile(const fs::path& file, const std::string& pat, 
                        const std::string& newPat);

// Rename a file given full paths
bool renameErrorCheck(fs::path path, fs::path new_path);

// Returns a <map> of filenames in a given directory
Filenames getFilenames(const std::set<fs::path>& dirs);

// Remove the program name from the menu
void removeFileByName(Filenames& files, const fs::path path);

// Print filenames for menu
void printFilenames(const std::map<int16_t, fs::path>& paths, 
                    const bool showNums=false);

fs::path getBetweenFilename(const fs::path& path, 
                          const std::string& lpat, const std::string& rpat,
                          const std::string& replacement);

// Pause program with cin and printed message
void printPause();

// Used with splitString to remove spaces from ends of a string
std::string removeSpace(std::string s);

// Returns a vector of a string split along a delimiter (uses function removeSpace)
std::vector<std::string> splitString(const std::string& str, 
                                     const std::string& delimiter, 
                                     bool removeSpaces = true);

void toLowercase(std::string& s);

void capitalize(std::string& s);


#endif