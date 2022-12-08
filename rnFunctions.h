#ifndef RNFUNCTIONS_H
#define RNFUNCTIONS_H

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;


std::string strReplaceAll(const std::string& origin, const std::string& pat, 
                          const std::string& newPat, const size_t start = 0);

void printFileChange(const fs::path oldPath, const std::string& newFilename);

std::string getReplacementInput(const std::vector<fs::path>& matchedPaths);

std::string removeDotEnds(const fs::path& f, bool& dotAtStart);

std::string restoreDotEnds(const fs::path& file, std::string newFilename, 
                           bool& dotAtStart);

std::string renameFiles(const fs::path& file, const std::string& pat, 
                        const std::string& newPat);

bool renameErrorCheck(fs::path path, fs::path new_path);

std::vector<fs::path> getFilenames(const std::string& dir);

void printFilenames(const std::vector<fs::path>& paths, 
                    const std::set<int>& removedFiles, 
                    const bool showNums=false);

std::string deleteBetween(const fs::path& path, 
                          const std::string& lpat, const std::string& rpat);

void printPause();

bool betweenQuitQuery(std::vector<fs::path> matchedPaths);

#endif