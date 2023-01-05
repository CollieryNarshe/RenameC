#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <filesystem>
#include <string_view>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;

void renameAndMenuUpdate(Filenames& matchedPaths, Filenames& originalPaths);

void keywordDefaultReplace(std::string& pattern, 
                           Filenames& filePaths);

void keywordHelpMenu();

void keywordChangeDir(fs::path& path, Filenames& filePaths, Filenames& filePaths_copy);

void keywordRemoveFilename(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy);

void keywordRemoveDots(Filenames& filePaths);

void keywordBetween(Filenames& filePaths);

void keywordCapOrLower(Filenames& filePaths, std::string_view pattern);

#endif