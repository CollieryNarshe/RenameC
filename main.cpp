#include "keywords.h"
#include "colors.h"
#include "history.h"
#include "rnFunctions.h"
#include <windows.h>
#include <iostream>
#include <filesystem>
#include <map>
#include <utility>
#include <vector>
#include <set>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;

int main(int argc, char* argv[])
{
    const fs::path programName{argv[0]};
    HistoryData history{programName};
    std::string pattern{};
    std::set<fs::path> directories{fs::canonical(".\\")};
    Filenames filePaths{getFilenames(directories, programName)};
    Filenames filePaths_copy{filePaths};      // Used to restore filenames
    bool showNums{};                          // Toggle printing index #

    while (true)
    {
        filePaths_copy = getFilenames(directories, programName);
        printFilenames(filePaths, showNums);      // Print filenames

        setColor(Color::pink);
        std::cout << "\nKeyword examples: !help, chdir, between, !series, !history, q\n";
        resetColor();
        std::cout << "Enter a keyword or pattern to search:\n> ";
        getline(std::cin, pattern);

        // Check for keywords:
        if (pattern == "")
            { std::cout << '\n'; history.saveToFile(); break; }

        else if (pattern == "!help" ) 
            { keywordHelpMenu(); getline(std::cin, pattern); }

        if (pattern == "q" || pattern == "exit") // New if statement for help menu
            { std::cout << '\n'; history.saveToFile(); break; }

        else if (pattern == "!index") 
            showNums = !showNums;

        else if (pattern.rfind("chdir", 0) == 0){
            keywordChangeDir(pattern, directories, filePaths, filePaths_copy);
            reloadMenu(filePaths, filePaths_copy, directories, programName);}

        else if (pattern == "adir+"){
            keywordAddAllDirs(directories, filePaths, filePaths_copy);
            reloadMenu(filePaths, filePaths_copy, directories, programName);}

        else if (pattern.rfind("adir", 0) == 0){
            keywordChangeDir(pattern, directories, filePaths, filePaths_copy, true);
            reloadMenu(filePaths, filePaths_copy, directories, programName);}

        else if (pattern.rfind("rmdir", 0) == 0){
            keywordRemoveDir(pattern, directories, filePaths, filePaths_copy);
            reloadMenu(filePaths, filePaths_copy, directories, programName);}

        else if (pattern == "!pwd")
            keywordPWD(directories);

        else if (pattern == "!reload")
            reloadMenu(filePaths, filePaths_copy, directories, programName);

        else if (pattern.rfind("rm-", 0) == 0)
            keywordRemoveAllFilenames(pattern, filePaths, filePaths_copy);

        else if (pattern.rfind("rm", 0) == 0)
            keywordRemoveFilename(pattern, filePaths, filePaths_copy);

        else if (pattern == "!dots")
            keywordRemoveDots(filePaths, history);

        else if (pattern == "between")
            keywordBetween(filePaths, history);

        else if (pattern == "between+")
            keywordBetween(filePaths, history, true);

        else if (pattern == "!lower" || pattern == "!cap")
            keywordCapOrLower(filePaths, pattern, history);

        else if (pattern == "!series")
            keywordSeries(filePaths, history);

        else if (pattern == "!print")
            keywordPrintToFile(filePaths, showNums, directories);

        else if (pattern == "!wordcount")
            keywordWordCount(filePaths);

        else if (pattern == "!rnsubs")
            keywordRenameSubs(filePaths, history);

        else if (pattern == "!rmdirs")
            keywordRemoveDirectories(filePaths);

        else if (pattern == "!rmfiles")
            keywordRemoveDirectories(filePaths, false);

        else if (pattern == "!undo")
            undoRename(history, 0, filePaths);

        else if (pattern == "!history")
            keywordHistory(history, filePaths);

        else if (pattern == "!togglehistory")
            keywordToggleHistory(history);
        
        // Get second pattern:
        else if (pattern != "")  // Pattern check for help menu (skip to filename menu)
            keywordDefaultReplace(pattern, filePaths, history);
    }
    return 0;
}