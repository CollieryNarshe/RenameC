#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "history.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include <filesystem>
#include <string_view>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;

void renameAndMenuUpdate(Filenames& matchedPaths, Filenames& originalPaths);

void keywordDefaultReplace(std::string& pattern, Filenames& filePaths, 
                           HistoryData& history);

void keywordHelpMenu();

void keywordAddAllDirs(std::set<fs::path>& directories,
                       Filenames& filePaths, Filenames& filePaths_copy);

void keywordRemoveDir(std::string pattern, std::set<fs::path>& directories,
                       Filenames& filePaths, Filenames& filePaths_copy);

void keywordChangeDir(const std::string& pattern, std::set<fs::path>& directories, 
                      Filenames& filePaths, Filenames& filePaths_copy, 
                      const bool add = false);

void keywordRemoveFilename(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy);

void keywordRemoveDots(Filenames& filePaths, HistoryData& history);

void keywordBetween(Filenames& filePaths, HistoryData& history, bool plus=false);

void keywordCapOrLower(Filenames& filePaths, std::string_view pattern,
                       HistoryData& history);

void keywordPWD(const std::set<fs::path>& directories);

void keywordSeries(Filenames& filePaths, HistoryData& history);

void keywordPrintToFile(Filenames& filePaths, bool& showNums, std::set<fs::path> directories);

void keywordRenameSubs(Filenames& filePaths, HistoryData& history);

void keywordRemoveDirectories(Filenames& filePaths, bool remove = true);

void keywordWordCount(Filenames& filePaths);

void keywordHistory(HistoryData& history);

void keywordToggleHistory(HistoryData& history);

#endif