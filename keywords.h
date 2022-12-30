#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;

void keywordDefaultReplace(std::string& pattern, 
                           Filenames& filePaths);

void keywordHelpMenu();

void keywordChangeDir(fs::path& path, Filenames& filePaths_copy);

void keywordRemoveFilename(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy);

void keywordRemoveDots(std::string& pattern, 
                       Filenames& filePaths);

void keywordBetween(Filenames& filePaths);

#endif