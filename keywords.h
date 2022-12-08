#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <set>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;


void keywordDefaultReplace(std::string& pattern, 
                           const std::vector<fs::path>& filePaths,
                           std::set<int>& removedFiles);

void keywordHelpMenu();

void keywordChangeDir(std::string& path, std::set<int>& removedFiles);

void keywordRestoreFilename(const std::string& pattern, 
                            const std::vector<fs::path>& filePaths,
                            std::set<int>& removedFiles);

void keywordRemoveFilename(const std::string& pattern, 
                           const std::vector<fs::path>& filePaths,
                           std::set<int>& removedFiles);

void keywordRemoveDots(std::string& pattern, 
                       const std::vector<fs::path>& filePaths,
                       const std::set<int>& removedFiles);

void keywordBetween(const std::vector<fs::path>& filePaths,
                    const std::set<int>& removedFiles);

#endif