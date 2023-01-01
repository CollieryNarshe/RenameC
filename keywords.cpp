#include "rnFunctions.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <set>
#include <vector>

using Filenames = std::map<int16_t, fs::path>;


void keywordDefaultReplace(std::string& pattern, 
                           Filenames& filePaths)
{
    Filenames matchedPaths{};
    std::string filename{};

    //Check for matches
    if (pattern == "#begin" || pattern == "#end")
        matchedPaths = filePaths;

    else
    {
        for (const auto& pair: filePaths)
        {
            if (pattern == "#ext" && pair.second.has_extension())
                matchedPaths[pair.first] = pair.second;

            else
            {
            filename = lowercase( pair.second.filename().string() );
            if (!(filename.find(lowercase(pattern)) == std::string::npos) )
                matchedPaths[pair.first] = pair.second;
            }
        }
    }
    
    // Exit function if no matches found
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
        "\n\n========================================================="
        "\npatterns:    #start, #end, #ext\n"
        "\nchdir:       Change directory."
        "\n!dots:       Delete periods from all filenames and replace with spaces. Ignores prefixes and extensions."
        "\nbetween:     Remove text between (and not including) two patterns."
        "\n!index:      Show index numbers for filenames."
        "\nrm #(,#):    Remove or restore the file(s) at index #s (to omit rename)."
        "\n!cap:        Capitalize each word of all files."
        "\n!reload:     Reload filenames from directory and restore removed files."
        "\nq, exit, '': Quit.\n"
        "=========================================================\n";
    printPause();
}



void keywordChangeDir(fs::path& path, Filenames& filePaths, Filenames& filePaths_copy)
{
    std::string newDir{};
    std::cout << "Enter new directory: ";
    getline(std::cin, newDir);

    if (fs::exists(newDir))
    {
        path = newDir;
        filePaths = getFilenames(path);
        filePaths_copy = filePaths;
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



void keywordRemoveDots(Filenames& filePaths)
{
    std::cout << "\nAre you sure you want to remove all dots? "
                 "(Good dots once lost are lost forever.)\n"
                 "Enter q to quit or ENTER to continue.\n";
    std::string pattern{};
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
        pair.second = pair.second.replace_filename(new_filename);

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

        // Get matches
        if ( (lowercase(filename).find(lowercase(lpat)) != std::string::npos) && 
             (lowercase(filename).find(lowercase(rpat)) != std::string::npos) )
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



void keywordCapitalize(Filenames& filePaths)
{
    std::cout << "\nAre you sure you want to capitalize every word of all files? "
                 "(Good lowercase once lost is lost forever.)\n"
                 "Enter q to quit or ENTER to continue.\n";
    std::string query{};
    getline(std::cin, query);

    if (query == "q")
        return;

    fs::path path{};
    std::string filename{};
    for (auto& pair: filePaths)
    {
        fs::path path = pair.second;
        std::string filename = path.filename().string();

        capitalize(filename);

        // Rename the actual files
        fs::path fullPath{path.parent_path() /= filename};
        if ( renameErrorCheck (path, fullPath) )
            printFileChange(path, filename);

        // Edit filename for menu
        pair.second.replace_filename(filename);
    }
    printPause();
}


void keywordLower(Filenames& filePaths)
{
    std::cout << "\nAre you sure you want to remove capitalization of every word of all files? "
                 "(Good uppercase once lost is lost forever.)\n"
                 "Enter q to quit or ENTER to continue.\n";
    std::string query{};
    getline(std::cin, query);

    if (query == "q")
        return;

    fs::path path{};
    std::string filename{};
    for (auto& pair: filePaths)
    {
        fs::path path = pair.second;
        std::string filename = path.filename().string();

        capitalize(filename, true);

        // Rename the actual files
        fs::path fullPath{path.parent_path() /= filename};
        if ( renameErrorCheck (path, fullPath) )
            printFileChange(path, filename);

        // Edit filename for menu
        pair.second.replace_filename(filename);
    }
    printPause();
}