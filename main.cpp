#include "keywords.h"
#include "rnFunctions.h"
#include <iostream>
#include <filesystem>
#include <utility>
#include <vector>
#include <set>

namespace fs = std::filesystem;


int main(){
    // std::cout << std::unitbuf; // TESTING - cout will output immediately
    std::string pattern{};
    std::string path{".\\"};
    std::vector<fs::path> filePaths{};
    std::set<int> removedFiles{};
    bool showNums{true};  // toggle printing index # for filenames

    while (true)
    {
        // Reset filename list
        filePaths = getFilenames(path);

        // Print filenames and input prompt text
        printFilenames(filePaths, removedFiles, showNums);

        std::cout << "\nKeywords: ?, chdir, dots, between, q\n"
                 "Enter keyword or pattern to change: ";
        getline(std::cin, pattern);

        // Check for keywords:
        if (pattern == "q" || pattern == "" || pattern == "exit")
            { std::cout << '\n'; break; }

        else if (pattern == "shownums") 
            showNums = !showNums;
            
        else if (pattern == "?" ) 
            keywordHelpMenu();

        else if (pattern == "chdir")
            keywordChangeDir(path, removedFiles);

        else if ( pattern.rfind("rm+", 0) == 0 )
            keywordRestoreFilename(pattern, filePaths, removedFiles);            

        else if (pattern == "rm-restore")
            removedFiles.clear();

        // Remove files from list
        else if (pattern.rfind("rm", 0) == 0)
            keywordRemoveFilename(pattern, filePaths, removedFiles);

        else if (pattern == "dots")
            keywordRemoveDots(pattern, filePaths, removedFiles);

        else if (pattern == "between")
            keywordBetween(filePaths, removedFiles);

        // Get second pattern:
        else
            keywordDefaultReplace(pattern, filePaths, removedFiles);
    }

    return 0;
}