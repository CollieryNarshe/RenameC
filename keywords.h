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

void keywordAddAllDirs(std::set<fs::path>& directories,
                       Filenames& filePaths, Filenames& filePaths_copy);

void keywordRemoveDir(std::set<fs::path>& directories,
                       Filenames& filePaths, Filenames& filePaths_copy);

void keywordChangeDir(std::set<fs::path>& paths, Filenames& filePaths, 
                      Filenames& filePaths_copy, const bool add = false);

void keywordRemoveFilename(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy);

void keywordRemoveDots(Filenames& filePaths);

void keywordBetween(Filenames& filePaths);

void keywordCapOrLower(Filenames& filePaths, std::string_view pattern);

void keywordPWD(const std::set<fs::path>& directories);

#endif