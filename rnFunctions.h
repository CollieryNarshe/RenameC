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

// bool check for pattern, converting ? into any number
bool checkPatternWithRegex(const std::string& filename, const std::string& pattern);

// Convert pattern, converting ? into number in filename
std::string convertPatternWithRegex(std::string filename, std::string pattern,
                                    bool lower = true);

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
void printFilenames(const Filenames& paths, 
                    const bool showNums=false);

bool checkBetweenMatches(const fs::path& path, 
                         std::string lpat, std::string rpat);

fs::path getBetweenFilename(const fs::path& path, 
                          std::string lpat, std::string rpat,
                          const std::string& replacement, bool plus);

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

void printToFile(Filenames& filePaths, std::set<fs::path> directories, 
                 std::string separator);

Filenames replaceSubtitleFilenames(Filenames& filePaths, Filenames subtitlePaths);

void defaultPrintFilenameWithColor(const fs::path& filePath, std::string pat);

void betweenPrintFilenameWithColor(const fs::path& filePath, std::string pattern1,
                                std::string pattern2);

#endif