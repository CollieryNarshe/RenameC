#ifndef RNFUNCTIONS_H
#define RNFUNCTIONS_H

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;

// Replaces all instances of a pattern with a new pattern
std::string strReplaceAll(std::string origin, const std::string& pat, 
                          const std::string& newPat, const size_t start = 0);

std::string lowercase(std::string s);

// Print: old filename ---> new filename
void printFileChange(const fs::path oldPath, const std::string& newFilename);

// Print number of matches and ask for second pattern
std::string getReplacementInput(int16_t index);

// Takes a path and removes the extension and dot at start
std::string removeDotEnds(const fs::path& f, bool& dotAtStart);

// Restors the extension and dot at start
std::string restoreDotEnds(const fs::path& file, std::string newFilename, 
                           bool& dotAtStart);

// Rename a file using given two patterns
std::string renameFile(const fs::path& file, const std::string& pat, 
                        const std::string& newPat);

// Rename a file given full paths
bool renameErrorCheck(fs::path path, fs::path new_path);

// Returns a <map> of filenames in a given directory
std::map<int16_t, fs::path> getFilenames(const fs::path& dir);

// Remove the program name from the menu
void removeFileByName(Filenames& files, const fs::path path);

// Print filenames for menu
void printFilenames(const std::map<int16_t, fs::path>& paths, 
                    const bool showNums=false);


// Remove part of string between two patterns
std::string deleteBetween(const fs::path& path, 
                          const std::string& lpat, const std::string& rpat,
                          const std::string& replacement, std::vector<std::string>& renamed);

// Pause program with cin and printed message
void printPause();

// Checks if any patterns were matched and chance to cancel
bool betweenQuitQuery(const Filenames& matchedPaths);

// Used with splitString to remove spaces from ends of a string
std::string removeSpace(std::string s);

// Returns a vector of a string split along a delimiter
std::vector<std::string> splitString(const std::string& str, 
                                     const std::string& delimiter, 
                                     bool removeSpaces = true);

void capitalize(std::string& s, bool allLower=false);


#endif