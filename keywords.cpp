#include "rnFunctions.h"
#include <iostream>
#include <string>
#include <set>
#include <vector>


void keywordDefaultReplace(std::string& pattern, 
                           const std::vector<fs::path>& filePaths,
                           std::set<int>& removedFiles)
{
    std::vector<fs::path> matchedPaths{};

    //Check for matches
    std::int16_t idx{};
    for (const auto& file: filePaths)
    {
        if ( !removedFiles.count(idx) && ( !(file.filename().string().find(pattern) == std::string::npos) ))
            matchedPaths.push_back(file);

        ++idx;
    }
    
    if ( !matchedPaths.size() )
    {
        std::cout << "\nNo filenames contain this pattern.\n";
        printPause();
        return;
    }

    // Get second input (replacement pattern)
    std::string replacement{ getReplacementInput(matchedPaths) };

    if ( replacement == "q" )
        return;

    for ( const auto& file: matchedPaths )
    {
        //Rename the file
        std::string newFilename{ renameFiles(file, pattern, replacement) };

        printFileChange(file, newFilename);
    }

    printPause();
}



void keywordHelpMenu()
{
    std::cout << 
            "\nchdir:       Change directory."
            "\ndots:        Delete periods from all filenames. Ignores prefixes and extensions."
            "\nbetween:     Remove text between (and not including) two patterns."
            "\nshownums:    Show index numbers for filenames."
            "\nrm #:        Remove the file at index # (to omit rename)."
            "\nrm+ #:       Restore the file at index #."
            "\nrm-restore:  Restore all removed files."
            "\nq, exit, '': Quit.\n";
            printPause();
}



void keywordChangeDir(std::string& path, std::set<int>& removedFiles)
{
    std::string newDir{};
    std::cout << "Enter new directory: ";
    getline(std::cin, newDir);

    if (fs::exists(newDir))
    {
        removedFiles.clear();
        std::cout << "\nDirectory changed.\n";
        printPause();
        path = newDir;
    }
    else
    {
        std::cout << "\nIncorrect directory path.\n";
        printPause();
    }
}



void keywordRestoreFilename(const std::string& pattern, 
                            const std::vector<fs::path>& filePaths,
                            std::set<int>& removedFiles)
{
    try
    {
        const int index{ stoi( pattern.substr(3) ) };

        if ( index < 0 || index >= ( filePaths.size() ) )
        {
            std::cout << "\nIndex is out of bounds.\n";
            printPause();
            return;
        }

        if ( !removedFiles.count(index) )
        {
            std::cout << "\nIndex " << index << " does not need to be restored. Use rm # to remove.\n";
            printPause();
            return;
        }

        removedFiles.erase(index);
        std::cout << "\nFile restored: " << filePaths[index].filename().string() << '\n';
        printPause();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << "\nIncorrect usage of rm+ #. Use keyword ? for more info.\n";
        printPause();
    }

}


void keywordRemoveFilename(const std::string& pattern, 
                           const std::vector<fs::path>& filePaths,
                           std::set<int>& removedFiles)
{
    try
    {
        const int index{ stoi( pattern.substr(2) ) };

        if ( index < 0 || index > (filePaths.size() - 1) )
        {
            std::cout << "\nIndex is out of bounds.\n";
            printPause();
            return;
        }

        if ( removedFiles.count(index) )
        {
            std::cout << "\nIndex " << index << " is already removed. Use rm+ # to restore.\n";
            printPause();
            return;
        }

        removedFiles.insert(index);
        std::cout << "\nFile removed: " << filePaths[index].filename().string() << '\n';
        printPause();
    }

    catch(const std::exception& e) //...
    {
        std::cerr << e.what() << '\n';
        std::cout << "\nIncorrect usage of rm #. Use keyword ? for more info.\n";
        printPause();
    }
}



void keywordRemoveDots(std::string& pattern, 
                       const std::vector<fs::path>& filePaths,
                       const std::set<int>& removedFiles)
{
    std::cout << "\nAre you sure you want to remove all dots? "
                 "(Good dots once lost are lost forever.)\n"
                 "Enter q to quit or ENTER to continue.\n";
    getline(std::cin, pattern);

    if (pattern == "q")
        return;
    
    int idx{};
    int matchCount{};
    pattern = ".";
    for (const auto& file: filePaths)
    {
        bool dotAtStart{};
        
        // Remove suffix, and extension if file (not folder)
        std::string filename{ removeDotEnds(file, dotAtStart) };

        // Check for matches
        if ( removedFiles.count(idx) || 
             (filename.find(pattern) == std::string::npos) )
        {
            ++idx;
            continue;
        }

        // Edit filename with new pattern
        std::string new_filename{strReplaceAll(filename, ".", "")};

        // Restore extension or suffix
        new_filename = restoreDotEnds(file, new_filename, dotAtStart);

        // Rename the actual files
        fs::path fullPath{file.parent_path() /= new_filename};
        if ( renameErrorCheck (file, fullPath) )
            printFileChange(file, new_filename);

        ++idx;
        ++matchCount;
    }

    if (matchCount == 0)
        std::cout << "\nNo filenames with dots found.\n";

    printPause();
}



void keywordBetween(const std::vector<fs::path>& filePaths,
                    const std::set<int>& removedFiles)
{
    std::string filename;
    std::string lpat{};
    std::string rpat{};
    
    std::cout << "Enter left pattern: ";
    getline(std::cin, lpat);
    std::cout << "Enter right pattern: ";
    getline(std::cin, rpat);
    std::cout << '\n';
    
    // Get matched filenames
    std::vector<fs::path> matchedPaths{};
    std::int16_t idx{};
    for (const auto& file: filePaths)
    {
        filename = file.filename().string();

        if ( !removedFiles.count(idx) && 
           (filename.find(lpat) != std::string::npos) && 
           (filename.find(rpat) != std::string::npos) )
        {
            matchedPaths.push_back(file);
        }

        ++idx;
    }

    if ( !betweenQuitQuery(matchedPaths) )
        return;

    //Rename and print
    for (const auto& file: matchedPaths)
    {
        filename = deleteBetween(file, lpat, rpat);
        printFileChange(file, filename);
    }

    printPause();
}