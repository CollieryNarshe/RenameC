#include "rnFunctions.h"
#include "colors.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <string_view>

using Filenames = std::map<int16_t, fs::path>;



void renameAndMenuUpdate(Filenames& newPaths, Filenames& oldPaths)
{
    for (auto& pair: newPaths)
    {
        fs::path newPath = pair.second;

        // Rename files. If successful update menu
        if ( renameErrorCheck(oldPaths[pair.first], newPath) )
            oldPaths[pair.first] = newPath;
    }
}


bool checkMapItemUnique(const Filenames& filePaths, fs::path path)
{
    for (const auto& pair: filePaths)
    {
        if (path == pair.second)
        {
            return false;
        }
    }
    return true;
}



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
            if (pattern == "#ext" && pair.second.has_extension() && !fs::is_directory(pair.second))
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
    std::cerr << "Enter replacement pattern: "; 
    std::string replacement{};
    std::getline(std::cin, replacement);
    std::cout << '\n';

    if (pattern == replacement)
    {
        setColor(Color::red);
        std::cout << "Pattern and replacement are the same.\n";
        resetColor();
        return;
    }


    // Get new filenames
    Filenames matchedPaths_copy{matchedPaths};
    for ( auto& pair: matchedPaths_copy )
    {
        matchedPaths[pair.first] = renameFile(pair.second, pattern, replacement);

        if (!checkMapItemUnique(filePaths, matchedPaths[pair.first]))
            {
                setColor(Color::red);
                std::cout << "Cannot rename " << filePaths[pair.first].filename() << " (Filename " << matchedPaths[pair.first].filename() << " already exists.)\n"; 
                resetColor();
                matchedPaths.erase(pair.first);
                continue;
            }

        // Print filename and changes
        printFileChange(filePaths[pair.first], matchedPaths[pair.first]);
    }

    // Chance to quit
    if ( checkIfQuit(matchedPaths.size()) )
        return;

    // Rename the actual files
    renameAndMenuUpdate(matchedPaths, filePaths);
}



void keywordHelpMenu()
{
    setColor(Color::green);

    std::cout << 
        "\n========================================================="
        "\nPatterns:    #begin, #end, #ext\n\nKeywords:"
        "\nchdir:       Change directory."
        "\n!dots:       Delete periods from all filenames and replace with spaces. Ignores prefixes and extensions."
        "\nbetween:     Remove text between (and not including) two patterns."
        "\n!index:      Show index numbers for filenames in menu."
        "\nrm #(,#):    Remove or restore the file(s) at index #s (to omit from rename)."
        "\n!lower:      Lowercase every letter."
        "\n!cap:        Capitalize every word."
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



void keywordRemoveDots(Filenames& filePaths)
{
    Filenames matchedPaths{};
    fs::path new_path{};
    std::string old_filename{};
    std::string new_filename{};
    bool dotAtStart{};
    std::cout << '\n';

    // Get matches and print
    for (auto& pair: filePaths)
    {
        new_path = pair.second;
        dotAtStart = false;
        
        // Remove suffix, and extension from path if not folder
        removeDotEnds(new_path, dotAtStart);

        // Check for matches
        if ( new_path.filename().string().find(".") == std::string::npos )
            continue;

        // Remove dots from path filename
        new_path.replace_filename(strReplaceAll(new_path.filename().string(), ".", " "));

        // Restore extension or suffix to path
        restoreDotEnds(new_path, pair.second, dotAtStart);

        // For code readability
        new_filename = new_path.filename().string();
        old_filename = pair.second.filename().string();

        // Check if a match and print
        if(old_filename == new_filename)
            continue;

        if (!checkMapItemUnique(filePaths, new_path))
            {
                setColor(Color::red);
                std::cout << "Cannot rename \"" << old_filename << "\" (Filename \"" << new_filename << "\" already exists.)\n"; 
                resetColor();
                continue;
            }

        matchedPaths[pair.first] = new_path;
        printFileChange(old_filename, new_filename);
    }

    // Exit if no matches found
    if ( !checkForMatches(matchedPaths) )
        return;

    // Ask to quit or continue
    if ( checkIfQuit(matchedPaths.size()) )
        return;

    // Rename the actual files and update menu
    renameAndMenuUpdate(matchedPaths, filePaths);
}



void keywordBetween(Filenames& filePaths)
{
    std::string filename;
    std::string lpat{};
    std::string rpat{};
    std::string replacement{};
    fs::path fullPath{};
    
    std::cout << "Enter left pattern: ";
    getline(std::cin, lpat);
    std::cout << "Enter right pattern: ";
    getline(std::cin, rpat);
    std::cout << "Enter replacement pattern: ";
    getline(std::cin, replacement);
    
    // Get matched filenames
    Filenames matchedPaths{};
    Filenames renamed{};      // To check for naming conflicts
    for (auto& pair: filePaths)
    {
        fs::path path{pair.second};
        int16_t idx{pair.first};

        fullPath = getBetweenFilename(path, lpat, rpat, replacement);
        if (fullPath == "")
            continue;  // Skip if no match

        // Make sure new filename is different
        if (fullPath == path)
            continue;

        // Make sure multiple files are not named the same name:
        if (!checkMapItemUnique(renamed, fullPath) || !checkMapItemUnique(filePaths, fullPath))
        {
            setColor(Color::red);
            std::cout << "Cannot rename " << path.filename() << " (Filename " << fullPath.filename() << " already exists.)\n"; 
            resetColor();
            continue;
        }

        renamed[idx] = fullPath;
        matchedPaths[idx] = fullPath;
        printFileChange(path, fullPath);
    }

    // Check if any matches
    if ( !checkForMatches(matchedPaths) )
        return;

    // Print number of matches then ask to quit or continue
    if (checkIfQuit(matchedPaths.size()) )
        return;

    //Rename and print
    renameAndMenuUpdate(matchedPaths, filePaths);
}



void keywordCapOrLower(Filenames& filePaths, std::string_view pattern)
{
    Filenames matchedPaths{};
    fs::path path{};
    std::string filename{};
    std::cout << '\n';

    // Get matches and print
    for (auto& pair: filePaths)
    {
        fs::path path = pair.second;
        std::string newFilename = path.filename().string();

        if (pattern == "!lower")
            toLowercase(newFilename);
        else if (pattern == "!cap")
            capitalize(newFilename);

        // Check if a match
        if(path.filename() != newFilename)
        {
            fs::path fullPath{path.parent_path() /= newFilename};
            matchedPaths[pair.first] = fullPath;
            // Print
            printFileChange(path, fullPath);
        }
    }

    if ( !checkForMatches(matchedPaths) )
        return;

    if ( checkIfQuit(matchedPaths.size()) )
        return;

    // Rename files and update menu
    renameAndMenuUpdate(matchedPaths, filePaths);
}


