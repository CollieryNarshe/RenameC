#include "keywords.h"
#include "colors.h"
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
    std::string pattern{};
    std::set<fs::path> directories{programName.parent_path()};
    Filenames filePaths{getFilenames(directories)};
    Filenames filePaths_copy{filePaths};      // Used to restore filenames
    bool showNums{};                          // Toggle printing index #

    while (true)
    {
        removeFileByName(filePaths, programName); // Remove program name from menu
        printFilenames(filePaths, showNums);      // Print filenames

        setColor(Color::pink);
        std::cout << "\nKeywords examples: !help, chdir, !dots, between, q\n";
        resetColor();
        std::cout << "Enter keyword or pattern to change: ";
        getline(std::cin, pattern);

        // Check for keywords:
        if (pattern == "q" || pattern == "" || pattern == "exit")
            { std::cout << '\n'; break; }

        else if (pattern == "!index") 
            showNums = !showNums;
            
        else if (pattern == "!help" ) 
            keywordHelpMenu();

        else if (pattern == "chdir")
            keywordChangeDir(directories, filePaths, filePaths_copy);

        else if (pattern == "adir")
            keywordChangeDir(directories, filePaths, filePaths_copy, true);

        else if (pattern == "adir+")
            keywordAddAllDirs(directories, filePaths, filePaths_copy);

        else if (pattern == "adir-")
            keywordRemoveDir(directories, filePaths, filePaths_copy);

        else if (pattern == "!pwd")
            keywordPWD(directories);

        else if (pattern == "!reload"){
            filePaths = getFilenames(directories);
            filePaths_copy = filePaths;}

        // Remove files from list
        else if (pattern.rfind("rm", 0) == 0)
            keywordRemoveFilename(pattern, filePaths, filePaths_copy);

        else if (pattern == "!dots")
            keywordRemoveDots(filePaths);

        else if (pattern == "between")
            keywordBetween(filePaths);

        else if (pattern == "!lower" || pattern == "!cap")
            keywordCapOrLower(filePaths, pattern);

        // Get second pattern:
        else
            keywordDefaultReplace(pattern, filePaths);
    }

    return 0;
}