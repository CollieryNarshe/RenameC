#ifndef RNFUNCTIONS_H
#define RNFUNCTIONS_H

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;


std::string strReplaceAll(const std::string& origin, const std::string& pat, 
                          const std::string& newPat, const size_t start = 0);

void printFileChange(const fs::path oldPath, const std::string& newFilename);

std::string getReplacementInput(int16_t index);

std::string removeDotEnds(const fs::path& f, bool& dotAtStart);

std::string restoreDotEnds(const fs::path& file, std::string newFilename, 
                           bool& dotAtStart);

std::string renameFiles(const fs::path& file, const std::string& pat, 
                        const std::string& newPat);

bool renameErrorCheck(fs::path path, fs::path new_path);

std::map<int16_t, fs::path> getFilenames(const fs::path& dir);

void printFilenames(const std::map<int16_t, fs::path>& paths, 
                    const bool showNums=false);

std::string deleteBetween(const fs::path& path, 
                          const std::string& lpat, const std::string& rpat);

void printPause();

bool betweenQuitQuery(const Filenames& matchedPaths);

// Used with splitString to remove spaces from ends of a string
std::string removeSpace(std::string s);

// Returns a vector of a string split along a delimiter
std::vector<std::string> splitString(const std::string& str, 
                                     const std::string& delimiter, 
                                     bool removeSpaces = true);

#endif