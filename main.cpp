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
    fs::path directory{".\\"};
    Filenames filePaths{getFilenames(directory)};
    Filenames filePaths_copy{filePaths};      // Used to restore filenames
    removeFileByName(filePaths, programName); // Remove program name from menu
    bool showNums{};                          // Toggle printing index #

    while (true)
    {
        // Print filenames and input prompt text
        printFilenames(filePaths, showNums);

        setColor(Color::pink);
        std::cout << "\nKeywords: !help, chdir, !dots, between, q\n";
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
            keywordChangeDir(directory, filePaths, filePaths_copy);

        else if (pattern == "!reload"){
            filePaths = getFilenames(directory);
            filePaths_copy = filePaths; }

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