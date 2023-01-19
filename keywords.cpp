#include "rnFunctions.h"
#include "colors.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <string_view>
#include <regex>

using Filenames = std::map<int16_t, fs::path>;



void keywordHelpMenu()
{
    setColor(Color::green);

    std::cout << 
        // "\n========================================================="
        "\nPattern matches:     #begin, #end, #ext, ? (a digit)"

        "\n\nDirectory keywords:"
        "\nchdir, adir, rmdir:  Change, add, or remove a directory."
        "\nadir+:               Add all directories from current path(s)."
        "\n!pwd:                Print work directory."
        "\n!rmdirs, !rmfiles:   Remove all folders or files from menu."
        "\n!index:              Show index numbers for filenames in menu."
        "\nrm #(,#):            Remove or restore the menu filename(s) at given index #(s)."

        "\n\nRename keywords:"
        "\nbetween:             Remove text between (and not including) two patterns."
        "\nbetween+:            Remove text between (and including) two patterns."
        "\n!dots:               Remove periods from all filenames and replace with spaces. Ignores prefixes and extensions."
        "\n!series:             Rename files with set parameters, especially filenames with s01e01 type pattern."
        "\n!rnsubs:             Change the filenames of a given directory to the filenames in the menu, except extension. Useful for subtitle matching"
        "\n!lower:              Lowercase every letter."
        "\n!cap:                Capitalize every word."

        "\n\nOther keywords:"
        "\n!reload:             Reload filenames from directory and restore removed files."
        "\n!print:              Creates a file with the list of file and folders currently in menu."
        "\nq, exit, '':         Quit.\n\n";

    resetColor();
    std::cout << "Press ENTER to return to filename menu.\n"
                 "Or type a keyword or pattern to search:\n> ";
}



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
    std::int16_t set_index{getIndex(pattern)}; // keyword #index

    //Check for matches
    if (pattern == "#begin" || pattern == "#end")
        matchedPaths = filePaths;

    else
    {
        for (const auto& pair: filePaths)
        {
            if (pattern == "#ext" && pair.second.has_extension() && !fs::is_directory(pair.second))
                matchedPaths[pair.first] = pair.second;

            else if (pattern.rfind("#index", 0) == 0 && set_index <= pair.second.filename().string().length())
                matchedPaths[pair.first] = pair.second;

            else
            {
            filename = lowercase( pair.second.filename().string() );
            if ( checkPatternWithRegex(filename, lowercase(pattern)) )
                matchedPaths[pair.first] = pair.second;
            }
        }
    }
    
    // Exit function if no matches found
    if ( !matchedPaths.size() )
    {
        setColor(Color::red);
        std::cout << "\nNo filenames to change.\n";
        resetColor();
        printPause();
        return;
    }
    // Print a preview with pattern highlighted
    else
    {
        for (auto& pair : matchedPaths)
        {
            defaultPrintFilenameWithColor(pair.second, pattern);
        }
    }
    // Get second input (replacement pattern)
    std::cout << "Enter replacement pattern or q to quit: "; 
    std::string replacement{};
    std::getline(std::cin, replacement);
    std::cout << '\n';
    if (replacement == "q")
        return;

    // Get new filenames
    std::string temp_pattern{pattern};
    fs::path temp_filename{};
    for ( auto pair = matchedPaths.cbegin(); pair != matchedPaths.cend(); )
    {
        temp_pattern = convertPatternWithRegex(pair->second.filename().string(), pattern);
        temp_filename = renameFile(pair->second, temp_pattern, replacement);

        // Check for repeat names, but not if case is different
        if (
            (fs::exists(temp_filename) || !checkMapItemUnique(matchedPaths, temp_filename)) &&
            !(lowercase(temp_filename.filename().string()) == lowercase(pair->second.filename().string()) &&
                temp_filename.filename() != pair->second.filename())
           )
        {
            setColor(Color::red);
            std::cout << "Cannot rename " << filePaths[pair->first].filename() << " (Filename " << temp_filename.filename() << " already exists.)\n"; 
            resetColor();
            matchedPaths.erase(pair++);
            continue;
        }

        matchedPaths[pair->first] = temp_filename;
        // Print filename and changes
        printFileChange(filePaths[pair->first], matchedPaths[pair->first]);
        ++pair;
    }

    // Chance to quit
    if ( checkIfQuit(matchedPaths.size()) )
        return;

    // Rename the actual files
    renameAndMenuUpdate(matchedPaths, filePaths);
}



void keywordAddAllDirs(std::set<fs::path>& directories,
                       Filenames& filePaths, Filenames& filePaths_copy)
{
    std::set<fs::path> directories_temp{directories};
    int32_t count{};
    for (auto& pair : filePaths)
    {
        // Check if file is a directory and not already added
        if (fs::is_directory(pair.second) && (directories.find(pair.second) == directories.end()))
        {
            directories_temp.insert(pair.second);
            ++count;
            setColor(Color::green);
            std::cout << pair.second.generic_string() << '\n';
            resetColor();
        }
    }

    // Message if nothing found.
    if (!count)
    {
        setColor(Color::red);
        std::cout << "No new directories found.\n"; 
        resetColor();
        printPause();
        return;
    }

    // Chance to quit
    std::string query{};
    std::cout << "Do you wish to add these directories? q to quit.";
    getline(std::cin, query);
    if (query == "q")
        return;

    // Add directories and reset menu files
    directories = directories_temp;
    filePaths = getFilenames(directories);
    filePaths_copy = filePaths;
}


void keywordRemoveDir(std::string pattern, std::set<fs::path>& directories,
                       Filenames& filePaths, Filenames& filePaths_copy)
{
    std::string query{};
    std::string pattern_end{removeSpace(pattern.substr(5))};

    // Check if address is already given
    if (fs::exists(pattern_end))
        query = pattern_end;
    else
    {
        setColor(Color::green);
        for (auto& dir: directories)
        {
            std::cout << dir.generic_string() << '\n';
        }
        resetColor();

        std::cout << "Enter the directory path you wish to remove or q to quit:\n> ";
        getline(std::cin, query);

        if (query == "q" || query == "")
            return;
    }

    // If dir input is not in the list
    if (directories.find(query) == directories.end())
    {
        setColor(Color::red);
        std::cout << "That directory has not been added.\n";
        resetColor();
        printPause();
        return;
    }

    directories.erase(query);
    filePaths = getFilenames(directories);
    filePaths_copy = filePaths;
}


void keywordChangeDir(const std::string& pattern, std::set<fs::path>& directories, 
                      Filenames& filePaths, Filenames& filePaths_copy, 
                      const bool add = false)
{
    std::string newDir{};
    std::string pattern_end{};
    if (add) // keyword adir
        pattern_end = removeSpace(pattern.substr(4));
    else     // keyword chdir
        pattern_end = removeSpace(pattern.substr(5));

    if (fs::exists(pattern_end))
        newDir = pattern_end;
    else
    {
    std::cout << "Enter new directory or q to quit:\n> ";
    getline(std::cin, newDir);
    }

    if (newDir == "q" || newDir == "")
        return;

    if (!fs::exists(newDir))
    {
        setColor(Color::red);
        std::cout << "\nIncorrect directory path.\n";
        resetColor();
        printPause();
    }

    setColor(Color::green);
    if (!add){
        directories.clear();
        std::cout << "\nDirectory changed.\n"; }
    else
        std::cout << "\nDirectory added.\n";
    resetColor();

    directories.insert(fs::canonical(newDir));
    filePaths = getFilenames(directories);
    filePaths_copy = filePaths;

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
                std::cout << "Index " << index << " is out of bounds.\n";
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
        std::cout << "\nIncorrect usage of rm #. Use keyword !help for more info.\n";
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

        // Check for naming conflicts
        if (fs::exists(new_path) || !checkMapItemUnique(matchedPaths, new_path))
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



void keywordBetween(Filenames& filePaths, bool plus)
{
    std::string lpat{};
    std::string rpat{};
    std::string replacement{};
    fs::path fullPath{};
    
    std::cout << "Enter left pattern: ";
    getline(std::cin, lpat);
    std::cout << "Enter right pattern: ";
    getline(std::cin, rpat);

    // Check if index number can be converted
    std::int16_t lIndexCheck{getIndex(lpat)};
    std::int16_t rIndexCheck{getIndex(rpat)};
    if (lIndexCheck == 1000 || rIndexCheck == 1000)
    {
        printPause();
        return;
    }

    int32_t matchNum{};
    for (auto& pair : filePaths)
    {
        if ( checkBetweenMatches(pair.second, lpat, rpat) )
        {
            betweenPrintFilenameWithColor(pair.second, lpat, rpat);
            ++matchNum;
        }
    }

    // Check if matches
    if (!matchNum)
    {
        setColor(Color::red);
        std::cout << "\nNo filenames contain these patterns.\n";
        resetColor();
        printPause();
        return;
    }

    std::cout << "Enter replacement pattern: ";
    getline(std::cin, replacement);
    
    // Get matched filenames
    Filenames matchedPaths{};
    for (auto& pair: filePaths)
    {
        fs::path path{pair.second};
        int16_t idx{pair.first};

        fullPath = getBetweenFilename(path, lpat, rpat, replacement, plus);

        if (fullPath == "")
            continue;  // Skip if no match

        // Make sure new filename is different
        if (fullPath == path)
            continue;

        // Make sure multiple files are not named the same name:
        if (fs::exists(fullPath) || !checkMapItemUnique(matchedPaths, fullPath))
        {
            setColor(Color::red);
            std::cout << "Cannot rename " << path.filename() << " (Filename " << fullPath.filename() << " already exists.)\n"; 
            resetColor();
            continue;
        }

        matchedPaths[idx] = fullPath;
        printFileChange(path, fullPath);
    }

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

void keywordPWD(const std::set<fs::path>& directories)
{
    for (auto& dir: directories)
    {
        setColor(Color::green);
        std::cout << dir.generic_string() << '\n';
        resetColor();
    }
    printPause();
}

void keywordSeries(Filenames& filePaths)
{
// Dots code start (edited):
    Filenames matchedPaths{filePaths};
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
        old_filename = pair.second.filename().string();
        new_filename = new_path.filename().string();

        // Check for naming conflict
        if (fs::exists(new_path) || !checkMapItemUnique(matchedPaths, new_path))
            {
                setColor(Color::red);
                std::cout << "Cannot rename \"" << old_filename << "\" (Filename \"" << new_filename << "\" already exists.)\n"; 
                resetColor();
                continue;
            }

        matchedPaths[pair.first] = new_path;
    }
//End of dots code
// Start cap code (edited)
    for (auto& pair: matchedPaths)
    {
        fs::path path = pair.second;
        std::string newFilename = path.filename().string();

        capitalize(newFilename);

        // Check if a match
        if(path.filename() != newFilename)
        {
            fs::path fullPath{path.parent_path() /= newFilename};
            matchedPaths[pair.first] = fullPath;
        }
    }
// Begin between code (edited and regex)
    std::regex pattern{"[Ss][0-9][0-9][Ee][0-9][0-9]"};
    std::smatch sm;
    std::string lpat{};
    std::string rpat{"#end"};
    std::string replacement{};
    fs::path fullPath{};
    
    // Get matched filenames
    Filenames renamed_temp{};      // To check for naming conflicts
    for (auto pair = matchedPaths.cbegin(); pair != matchedPaths.cend(); )
    {
        fs::path path{pair->second};
        int16_t idx{pair->first};
        new_filename = path.filename().string();
        std::regex_search(new_filename, sm, pattern);
        if (sm[0] != "")
        {
        lpat = sm[0];

        // make s01e01 lowercase //test maybe a better way to do this? transform?
        new_filename.replace(sm.position(0), 1, "s");
        new_filename.replace(sm.position(0) + 3, 1, "e");
        }

        // Add resolution size to end of filename (if found)
        fs::path newPath{new_filename};
        if (new_filename.find("360p") != std::string::npos)
            replacement = " [360p]";
        else if (new_filename.find("480p") != std::string::npos)
            replacement = " [480p]";
        else if (new_filename.find("720p") != std::string::npos)
            replacement = " [720p]";
        else if (new_filename.find("1080p") != std::string::npos)
            replacement = " [1080p]";

        path.replace_filename(newPath);
        matchedPaths[idx].replace_filename(newPath);

// ============================
        fullPath = getBetweenFilename(path, lpat, rpat, replacement, false);
        // Skip if no match and make sure new filename is different
        if (fullPath == "" || fullPath == pair->second)
        {
            ++pair;
            continue;
        }

        // Make sure multiple files are not named the same name:
        if (fs::exists(fullPath) || !checkMapItemUnique(renamed_temp, fullPath))
        {
            setColor(Color::red);
            std::cout << "Cannot rename " << path.filename() << " (Filename " << fullPath.filename() << " already exists.)\n"; 
            resetColor();
            matchedPaths.erase(pair++);
            continue;
        }

        renamed_temp[idx] = fullPath;
        matchedPaths[idx] = fullPath;
        ++pair;
    }
// End between code
    // Print
    int16_t idx{};
    for (auto pair = matchedPaths.cbegin(); pair != matchedPaths.cend(); )
    {
        idx = pair->first;
        if (filePaths[idx] == matchedPaths[idx])
        {
            setColor(Color::red);
            std::cout << filePaths[idx].filename() << " is already named properly.\n"; 
            resetColor();
            matchedPaths.erase(pair++);
            continue;
        }
        else
        {
            printFileChange(filePaths[idx], matchedPaths[idx]);
            ++pair;

        }
    }

    if (!matchedPaths.size())
    {
        setColor(Color::red);
        std::cout << "\nNo files to rename.\n";
        resetColor();
        printPause();
        return;
    }

    // Print number of matches then ask to quit or continue
    if (checkIfQuit(matchedPaths.size()) )
        return;
    // Rename files and update menu
    renameAndMenuUpdate(matchedPaths, filePaths);
}


void keywordPrintToFile(Filenames& filePaths, bool& showNums, std::set<fs::path> directories)
{
    std::cout << "Enter separator for filenames (\\n default):\n> "; 
    std::string separator{};
    std::getline(std::cin, separator);
    if (separator == "")
        separator = "\n";

    printToFile(filePaths, directories, separator);
}


void keywordRenameSubs(Filenames& filePaths)
{
    std::cout << "Enter the name of directory containing subtitles:\n> "; 
    std::string query{};
    std::getline(std::cin, query);
    if (!fs::exists(query))
    {
        setColor(Color::red);
        std::cout << "\nDirectory doesn't exist.\n";
        resetColor();
        printPause();
        return;
    }
    // Get filenames in subtitle directory
    std::set<fs::path> directories{fs::canonical(query)};
    Filenames subtitlePaths{getFilenames(directories)};

    // Exit if no files in directory
    if (subtitlePaths.size() == 0)
    {
        std::cout << "Directory is empty.\n";
        printPause();
        return;
    }
    // Change filenames before rename
    Filenames newSubPaths{ replaceSubtitleFilenames(filePaths, subtitlePaths) };

    // Print filenames using size of smallest file list
    size_t size{ std::min(newSubPaths.size(), filePaths.size() )};
    size_t size_copy{size};
    for (size_t idx{}; idx < size_copy; ++idx )
    {
        // remove filenames that were unchanged
        if (subtitlePaths[idx] == newSubPaths[idx])
        {
            subtitlePaths.erase(idx);
            newSubPaths.erase(idx);
            --size;
            continue;
        }

        printFileChange(subtitlePaths[idx], newSubPaths[idx]);
    }
    
    // Check if there are any filenames to change
    if (size <= 0)
    {
        setColor(Color::red);
        std::cout << "\nNo filenames to change.\n";
        resetColor();
        printPause();
        return;
    }

    // Print number of matches then ask to quit or continue
    if (checkIfQuit(size))
        return;

    // Rename files and update menu
    renameAndMenuUpdate(newSubPaths, subtitlePaths);

}

void keywordRemoveDirectories(Filenames& filePaths, bool remove)
{
    for (auto pair = filePaths.cbegin(); pair != filePaths.cend(); )
    {
        if ( fs::is_directory(pair->second) == remove)
            filePaths.erase(pair++);
        else
            ++pair;
    }
}