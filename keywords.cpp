#include "rnFunctions.h"
#include "colors.h"
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
        setColor(Color::red);
        std::cout << "\nNo filenames contain this pattern.\n";
        resetColor();
        printPause();
        return;
    }

    // Get second input (replacement pattern)
    std::string replacement{ getReplacementInput(matchedPaths.size()) };

    if ( replacement == "q" )
        return;

    for ( auto& pair: matchedPaths )
    {
        //Rename and print the file
        std::string newFilename{ renameFile(pair.second, pattern, replacement) };
        if (newFilename == "")
            continue;
        printFileChange(pair.second, newFilename);

        // Update menu file list
        filePaths[pair.first].replace_filename(newFilename);
    }

    printPause();
}



void keywordHelpMenu()
{
    setColor(Color::green);

    std::cout << 
        "\n========================================================="
        "\npatterns:    #begin, #end, #ext\n"
        "\nchdir:       Change directory."
        "\n!dots:       Delete periods from all filenames and replace with spaces. Ignores prefixes and extensions."
        "\nbetween:     Remove text between (and not including) two patterns."
        "\n!index:      Show index numbers for filenames."
        "\nrm #(,#):    Remove or restore the file(s) at index #s (to omit rename)."
        "\n!cap:        Capitalize each word of all files."
        "\n!reload:     Reload filenames from directory and restore removed files."
        "\nq, exit, '': Quit.\n"
        "=========================================================\n";
    resetColor();
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
        setColor(Color::green);
        std::cout << "\nDirectory changed.\n";
        resetColor();
    }
    else
    {
        setColor(Color::red);
        std::cout << "\nIncorrect directory path.\n";
        resetColor();
        printPause();
    }
}



void keywordRemoveFilename(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy)
{
    try
    {
        std::string nums{ pattern.substr(2) };
        std::vector<std::string> indexes{ splitString(nums, ",") };

        for (std::string index_string: indexes)
        {
            int32_t index{ stoi(index_string) };

            if ( index < 0 || index > (filePaths_copy.size() - 1) )
            {
                setColor(Color::red);
                std::cout << "\nIndex " << index << " is out of bounds.\n";
                resetColor();
                continue;
            }

            if (filePaths.contains(index))
            {
                setColor(Color::green);
                std::cout << "File removed: " << filePaths[index].filename().string() << '\n';
                filePaths.erase(index);
                resetColor();
            }
            else
            {
                filePaths[index] = filePaths_copy[index];
                setColor(Color::green);
                std::cout << "File restored: " << filePaths[index].filename().string() << '\n';
                resetColor();
            }
        }
    }
    catch(const std::exception& e) //...
    {
        std::cerr << e.what() << '\n';
        setColor(Color::red);
        std::cout << "\nIncorrect usage of rm #. Use keyword ? for more info.\n";
        resetColor();
    }
}


bool checkMapItemUnique(const Filenames& filePaths, fs::path path)
{
    for (const auto& pair: filePaths)
    {
        if (path.generic_string() == pair.second.generic_string())
        {
            return false;
        }
    }
    return true;
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

    std::vector<std::string> renamed{};
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

        fs::path fullPath{path.parent_path() /= new_filename};

        // Make sure multiple files are not named the same name:
        if (!checkMapItemUnique(filePaths, fullPath))
        {
            setColor(Color::red); std::cout << "File naming error for " << path.filename() << 
                            ": Two files cannot have the same name."; resetColor();
            continue;
        }
        renamed.push_back(fullPath.generic_string());

        // Rename the actual files
        if ( renameErrorCheck (path, fullPath) )
        {
            printFileChange(path, new_filename);
            
            // Update file list for menu:
            pair.second.replace_filename(new_filename);
            
            ++matchCount;
        }

    }

    if (matchCount == 0)
    {
        setColor(Color::red);
        std::cout << "\nNo filenames with dots found.\n";
        resetColor();
    }

    printPause();
}



void keywordBetween(Filenames& filePaths)
{
    std::string filename;
    std::string lpat{};
    std::string rpat{};
    std::string replacement{};
    
    std::cout << "Enter left pattern: ";
    getline(std::cin, lpat);
    std::cout << "Enter right pattern: ";
    getline(std::cin, rpat);
    std::cout << "Enter replacement pattern: ";
    getline(std::cin, replacement);
    
    // Get matched filenames
    Filenames matchedPaths{};

    for (auto& pair: filePaths)
    {
        fs::path path{pair.second};
        int16_t idx{pair.first};

        filename = path.filename().string();

        // Get matches
        bool lmatch{lowercase(filename).find(lowercase(lpat)) != std::string::npos};
        bool rmatch{lowercase(filename).find(lowercase(rpat)) != std::string::npos};

        if ((lpat == "#begin" && rmatch) ||
            (lmatch && (rpat == "#end")) ||
            (lmatch && rmatch) )
        {
            matchedPaths[idx] = path;
        }
    }

    // Ask to quit or continue
    if ( !betweenQuitQuery(matchedPaths) )
        return;

    //Rename and print
    std::vector<std::string> renamed{};
    for (const auto& path: matchedPaths)
    {
        filename = deleteBetween(path.second, lpat, rpat, replacement, renamed);
        if (filename == "")
            continue;  // When there's an error in renaming

        printFileChange(path.second, filename);

        // Update menu filenames
        filePaths[path.first].replace_filename(filename);
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