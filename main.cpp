#include "keywords.h"
#include "rnFunctions.h"
#include <iostream>
#include <filesystem>
#include <map>
#include <utility>
#include <vector>
#include <set>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;


int main(){
    // std::cout << std::unitbuf; // TESTING - cout will output immediately
    std::string pattern{};
    fs::path directory{".\\"};
    Filenames filePaths{getFilenames(directory)};
    Filenames filePaths_copy{filePaths};  // used to restore filenames
    bool showNums{};                      // toggle printing index #

    while (true)
    {
        // Print filenames and input prompt text
        printFilenames(filePaths, showNums);

        std::cout << "\nKeywords: ?, chdir, !dots, between, q\n"
                 "Enter keyword or pattern to change: ";
        getline(std::cin, pattern);

        // Check for keywords:
        if (pattern == "q" || pattern == "" || pattern == "exit")
            { std::cout << '\n'; break; }

        else if (pattern == "!index") 
            showNums = !showNums;
            
        else if (pattern == "?" ) 
            keywordHelpMenu();

        else if (pattern == "chdir")
            keywordChangeDir(directory, filePaths, filePaths_copy);

        else if (pattern == "!refresh"){
            filePaths = getFilenames(directory);
            filePaths_copy = filePaths; }

        // Remove files from list
        else if (pattern.rfind("rm", 0) == 0)
            keywordRemoveFilename(pattern, filePaths, filePaths_copy);

        else if (pattern == "!dots")
            keywordRemoveDots(filePaths);

        else if (pattern == "between")
            keywordBetween(filePaths);

        else if (pattern == "!cap")
            keywordCapitalize(filePaths);
        
        else if (pattern == "!lower")
            keywordLower(filePaths);

        // Get second pattern:
        else
            keywordDefaultReplace(pattern, filePaths);
    }

    return 0;
}