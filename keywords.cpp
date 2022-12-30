#include "rnFunctions.h"
#include <iostream>
#include <string>
#include <set>
#include <vector>

using Filenames = std::map<int16_t, fs::path>;


void keywordDefaultReplace(std::string& pattern, 
                           Filenames& filePaths)
{
    Filenames matchedPaths{};

    //Check for matches
    for (const auto& pair: filePaths)
    {
        if (!(pair.second.filename().string().find(pattern) == std::string::npos) )
            // matchedPaths.push_back(file);
            matchedPaths[pair.first] = pair.second;

    }
    
    if ( !matchedPaths.size() )
    {
        std::cout << "\nNo filenames contain this pattern.\n";
        printPause();
        return;
    }

    // Get second input (replacement pattern)
    std::string replacement{ getReplacementInput(matchedPaths.size()) };

    if ( replacement == "q" )
        return;

    for ( const auto& pair: matchedPaths )
    {
        //Rename and print the file
        std::string newFilename{ renameFiles(pair.second, pattern, replacement) };
        printFileChange(pair.second, newFilename);

        // Update menu file list
        filePaths[pair.first] = newFilename;
    }

    printPause();
}



void keywordHelpMenu()
{
    std::cout << 
            "\nchdir:       Change directory."
            "\ndots:        Delete periods from all filenames and replace with spaces. Ignores prefixes and extensions."
            "\nbetween:     Remove text between (and not including) two patterns."
            "\nshownums:    Show index numbers for filenames."
            "\nrm #(,#):    Remove or restore the file(s) at index #s (to omit rename)."
            "\n!refresh:    Reload filenames from directory and restore removed files."
            "\nq, exit, '': Quit.\n";
            printPause();
}



void keywordChangeDir(fs::path& path, Filenames& filePaths_copy)
{
    std::string newDir{};
    std::cout << "Enter new directory: ";
    getline(std::cin, newDir);

    if (fs::exists(newDir))
    {
        path = newDir;
        filePaths_copy = getFilenames(path);
        std::cout << "\nDirectory changed.\n";
        printPause();
    }
    else
    {
        std::cout << "\nIncorrect directory path.\n";
        printPause();
    }
}



void keywordRemoveFilename(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy)
{
    try
    {
        // const int idx{ stoi( pattern.substr(2) ) };
        std::string nums{ pattern.substr(2) };
        std::vector<std::string> indexes{ splitString(nums, ",") };

        for (std::string index_string: indexes)
        {
            int32_t index{ stoi(index_string) };

            if ( index < 0 || index > (filePaths_copy.size() - 1) )
            {
                std::cout << "\nIndex " << index << " is out of bounds.\n";
                continue;
            }

            if (filePaths.contains(index))
            {
                std::cout << "\nFile removed: " << filePaths[index].filename().string() << '\n';
                filePaths.erase(index);
            }
            else
            {
                filePaths[index] = filePaths_copy[index];
                std::cout << "\nFile restored: " << filePaths[index].filename().string() << '\n';
            }
        }
    }
    catch(const std::exception& e) //...
    {
        std::cerr << e.what() << '\n';
        std::cout << "\nIncorrect usage of rm #. Use keyword ? for more info.\n";
    }
    printPause();
}



void keywordRemoveDots(std::string& pattern, 
                       Filenames& filePaths)
{
    std::cout << "\nAre you sure you want to remove all dots? "
                 "(Good dots once lost are lost forever.)\n"
                 "Enter q to quit or ENTER to continue.\n";
    getline(std::cin, pattern);

    if (pattern == "q")
        return;
    
    int matchCount{};
    pattern = ".";
    for (auto& pair: filePaths)
    {
        bool dotAtStart{};
        fs::path path{pair.second};
        
        // Remove suffix, and extension if file (not folder)
        std::string filename{ removeDotEnds(path, dotAtStart) };

        // Check for matches
        if ( filename.find(pattern) == std::string::npos )
            continue;

        // Edit filename with new pattern
        std::string new_filename{strReplaceAll(filename, pattern, " ")};

        // Restore extension or suffix
        new_filename = restoreDotEnds(path, new_filename, dotAtStart);

        // Rename the actual files
        fs::path fullPath{path.parent_path() /= new_filename};
        if ( renameErrorCheck (path, fullPath) )
            printFileChange(path, new_filename);
        
        // Update file list for menu:
        pair.second = new_filename;

        ++matchCount;
    }

    if (matchCount == 0)
        std::cout << "\nNo filenames with dots found.\n";

    printPause();
}



void keywordBetween(Filenames& filePaths)
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
    Filenames matchedPaths{};
    for (auto& pair: filePaths)
    {
        fs::path path{pair.second};
        int16_t idx{pair.first};

        filename = path.filename().string();

        if ( (filename.find(lpat) != std::string::npos) && 
             (filename.find(rpat) != std::string::npos) )
        {
            matchedPaths[idx] = path;
        }
    }

    // Ask to quit or continue
    if ( !betweenQuitQuery(matchedPaths) )
        return;

    //Rename and print
    for (const auto& path: matchedPaths)
    {
        filename = deleteBetween(path.second, lpat, rpat);
        printFileChange(path.second, filename);

        // Update menu filenames
        filePaths[path.first] = filename;
    }

    printPause();
}