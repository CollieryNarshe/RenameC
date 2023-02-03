#include "rnFunctions.h"
#include "colors.h"
#include "history.h"
#include "textCount.cpp"
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
        "\n\nRename keywords:"
        "\nbetween              Replace text between (not including) two patterns."
        "\nbetween+             Replace text between (including) two patterns."
        "\n!dots                Replace periods with spaces, ignoring pref and ext."
        "\n!series              Default parameters for filenames with s01e01 pattern."
        "\n!rnsubs              Match a folder's filenames (subs) to menu stems."
        "\n!lower               Lowercase every letter."
        "\n!cap                 Capitalize every word."

        "\n\nMenu keywords:"
        "\n!index               Show index numbers for filenames."
        "\nrm #(,#-#) [name]    Remove/restore filenames with index #s or name."
        "\nrm- (#(,#-#)) [name] Remove all filenames, except index/name."
        "\n!find !rfind [pat]   Remove all filenames containing/without pattern."
        "\nrmfolders, rmfiles   Remove all folders or files."
        "\nchdir, adir, rmdir   Change, add, or remove a working directory."
        "\nadir+                Add all menu folders to working directories."
        "\n!pwd                 Print work directories."

        "\n\nPattern matches:"
        "\n#begin               The start of filename."
        "\n#end                 The end of filename."
        "\n#ext                 Selects the file extension."
        "\n?                    Any digit 0-9. Can use match in replacement."
        "\n*                    Zero or one of any character or digit."
        "\n#^                   A number sequence for replacement. (01, 02...)"
        "\n#index #             Points to the filename index."

        "\n\nOther keywords:"
        "\n!reload              Reload the menu from source files."
        "\n!wordcount           Get count of words and lines in menu text files."
        "\n!print               Creates a text file with menu list."
        "\n!history             Show a list of rename history. Undo past renames."
        "\n!undo                Undo the last rename."
        "\n!togglehistory       Pause/unpause saving history."
        "\nq, exit, ''          Quit.\n\n";

    resetColor();
    std::cout << "Press ENTER to return to filename menu.\n"
                 "Or type a keyword or pattern to search:\n> ";
}





void keywordDefaultReplace(std::string& pattern, Filenames& filePaths, 
                           HistoryData& history)
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
        redErrorMessage("No filenames to change.");
        return;
    }
    // Print a preview with pattern highlighted
    else
    {
        std::cout << '\n';
        for (auto& pair : matchedPaths)
        {
            defaultPrintFilenameWithColor(pair.second, pattern);
        }
    }
    // Get second input for replacement
    std::cout << "\nEnter replacement pattern (or q to quit):\n> "; 
    std::string replacement{};
    std::getline(std::cin, replacement);
    std::cout << '\n';
    if (replacement == "q")
        return;

    // Get new filenames
    std::string temp_pattern{};
    std::string temp_replace{};
    fs::path temp_filename{};
    std::vector<std::string> digits{};
    bool patHasQ{pattern.find("?") != std::string::npos};
    bool replaceHasQ{replacement.find("?") != std::string::npos};
    int16_t sequencePattern_idx{1};
    
    for ( auto pair = matchedPaths.cbegin(); pair != matchedPaths.cend(); )
    {
        std::string originalFilename{pair->second.filename().string()};
        temp_replace = replacement;

        // extract digits into vector to use with ? in replacement pattern
        if (patHasQ && replaceHasQ)
        {
            digits = extractDigits( lowercase(originalFilename), lowercase(pattern) );
            temp_replace = replaceDigits(digits, temp_replace);
        }
        temp_replace = convertSequenceNumber(temp_replace, sequencePattern_idx, matchedPaths.size());
        temp_pattern = convertPatternWithRegex(originalFilename, pattern);
        temp_filename = renameFile(pair->second, temp_pattern, temp_replace);

        // Check for repeat names, but not if case is different
        if (
            (fs::exists(temp_filename) || !checkMapItemUnique(matchedPaths, temp_filename)) &&
            !(lowercase(temp_filename.filename().string()) == lowercase(originalFilename) &&
                temp_filename.filename() != pair->second.filename())
           )
        {
            redErrorMessage("Cannot rename " + originalFilename + " (Filename \"" +
                temp_filename.filename().string() + "\" already exists.)", false);
            matchedPaths.erase(pair++);
            ++sequencePattern_idx;
            continue;
        }

        matchedPaths[pair->first] = temp_filename;
        // Print filename and changes
        printFileChange(filePaths[pair->first], matchedPaths[pair->first]);
        ++pair;
        ++sequencePattern_idx;
    }

    // Chance to quit
    if ( checkIfQuit(matchedPaths.size()) )
        return;

    if (history.saveHistory)
        history.update(matchedPaths, filePaths);

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
        redErrorMessage("No new directories found.");
        return;
    }

    // Chance to quit
    std::string query{};
    std::cout << "Press ENTER to add these directories. (q to quit.)\n> ";
    getline(std::cin, query);
    if (query != "")
        return;

    // Add directories and reset menu files
    directories = directories_temp;
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

        std::cout << "Enter the directory path you wish to remove (or q to quit):\n> ";
        getline(std::cin, query);

        if (query == "q" || query == "")
            return;
    }

    fs::path fullDirPath{fs::canonical(query)};

    // If dir input is not in the list
    if (directories.find(fullDirPath) == directories.end())
    {
        redErrorMessage("That directory has not been added.");
        return;
    }

    directories.erase(fullDirPath);
    setColor(Color::green);
    std::cout << "\nDirectory removed: " << fullDirPath << '\n';
    resetColor();
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
    std::cout << "Enter new directory (or q to quit):\n> ";
    getline(std::cin, newDir);
    }

    if (newDir == "q" || newDir == "")
        return;

    fs::path newDirPath{fs::canonical(newDir)};

    if (!fs::exists(newDirPath))
    {
        redErrorMessage("Incorrect directory path.");
        return;
    }

    setColor(Color::green);
    if (!add){
        directories.clear();
        std::cout << "\nDirectory changed to: " << newDirPath << '\n'; }
    else
        std::cout << "\nDirectory added: " << newDirPath << '\n';
    resetColor();

    if (!add)
        std::filesystem::current_path(newDirPath);
    directories.insert(newDirPath);
}


bool removeByFilename(std::string filename, 
                      Filenames& filePaths,
                      Filenames& filePaths_copy)
{
    bool fileFound{};
    filename = removeSpace(filename);
    setColor(Color::green);
    for (auto& pair : filePaths_copy)
    {
        if (filename == pair.second.filename().string())
        {
            if (filePaths.contains(pair.first))
            {
                std::cout << "File removed: " << pair.second.filename().string() << '\n';
                filePaths.erase(pair.first);
                fileFound = true;
            }
            else
            {
                filePaths[pair.first] = filePaths_copy[pair.first];
                std::cout << "File restored: " << pair.second.filename().string() << '\n';
                fileFound = true;
            }
        }
    }
    resetColor();
    return fileFound;
}


void keywordRemoveFilename(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy)
{
    try
    {
        std::string subPat{ pattern.substr(2) };
        if (removeByFilename(subPat, filePaths, filePaths_copy))
            return;
        std::vector<std::string> indexes{ splitString(subPat, ",") };
        convertRangeDashes(indexes);

        for (std::string index_string: indexes)
        {
            int32_t index{ stoi(index_string) };

            if ( index < 0 || index > (filePaths_copy.size() - 1) )
            {
                redErrorMessage("Index " + std::to_string(index) + " is out of bounds.", false);
                continue;
            }

            if (filePaths.contains(index))
            {
                setColor(Color::green);
                std::cout << "File removed: " << filePaths_copy[index].filename().string() << '\n';
                filePaths.erase(index);
                resetColor();
            }
            else
            {
                filePaths[index] = filePaths_copy[index];
                setColor(Color::green);
                std::cout << "File restored: " << filePaths_copy[index].filename().string() << '\n';
                resetColor();
            }
        }
    }
    catch(const std::exception& e) //...
    {
        redErrorMessage("File not found.");
    }
}


bool KeepByFilename(std::string filename, 
                      Filenames& filePaths,
                      Filenames& filePaths_copy)
{
    bool fileFound{};
    filename = removeSpace(filename);
    for (auto& pair : filePaths_copy)
    {
        if (filename == pair.second.filename().string())
        {
            filePaths[pair.first] = pair.second;
            fileFound = true;
        }
    }
    return fileFound;
}



void keywordRemoveAllFilenames(const std::string& pattern, 
                           Filenames& filePaths,
                           Filenames& filePaths_copy)
{
    filePaths.clear();
    try
    {
        std::string subPat{ pattern.substr(3) };
        if (subPat == "")
            return;
        if (KeepByFilename(subPat, filePaths, filePaths_copy))
            return;
        
        std::vector<std::string> index_strs{ splitString(subPat, ",") };
        convertRangeDashes(index_strs);

        std::vector<std::int16_t> indexes{};
        for (std::string index_string: index_strs)
        {
            int32_t idx{ stoi(index_string) };

            if ( idx < 0 || idx > (filePaths_copy.size() - 1) )
            {
                redErrorMessage("Index " + std::to_string(idx) + " is out of bounds.", false);
                continue;
            }
            if (!std::count(indexes.begin(), indexes.end(), idx))
                indexes.push_back(idx);
        }


        for (std::int16_t index: indexes)
        {
            if (filePaths_copy.contains(index))
            {
                setColor(Color::green);
                std::cout << "File kept: " << filePaths_copy[index].filename().string() << '\n';
                filePaths[index] = filePaths_copy[index];
                resetColor();
            }
        }
    }
    catch(const std::exception& e) //...
    {
        redErrorMessage("File not found. All files removed.");
        return;
    }
}


void keywordRemoveDots(Filenames& filePaths, HistoryData& history)
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

        new_filename = new_path.filename().string();
        old_filename = pair.second.filename().string();

        // Check for naming conflicts
        if (fs::exists(new_path) || !checkMapItemUnique(matchedPaths, new_path))
            {
                redErrorMessage("Cannot rename \"" + old_filename + "\" (Filename \"" + 
                                new_filename + "\" already exists.)\n", false);
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

    if (history.saveHistory)
        history.update(matchedPaths, filePaths);

    // Rename the actual files and update menu
    renameAndMenuUpdate(matchedPaths, filePaths);
}



void keywordBetween(Filenames& filePaths, HistoryData& history, bool plus)
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
    if ((lpat.rfind("#index", 0) == 0 && (lIndexCheck == 1000)) || 
         rpat.rfind("#index", 0) == 0 && (rIndexCheck == 1000))
    {
        printPause();
        return;
    }

    if (lpat == "")
        lpat = "#begin";
    if (rpat == "")
        rpat = "#end";

    std::cout << '\n';
    int32_t matchNum{};
    for (auto& pair : filePaths)
    {
        if ( checkBetweenMatches(pair.second, lpat, rpat) )
        {
            betweenPrintFilenameWithColor(pair.second, lpat, rpat, plus);
            ++matchNum;
        }
    }

    // Check if matches
    if (!matchNum)
    {
        redErrorMessage("Error 1: No filenames contain these patterns.");
        return;
    }

    std::cout << "\nEnter replacement pattern (or q to quit):\n> ";
    getline(std::cin, replacement);

    if (replacement == "q")
        return;
    
    std::cout << '\n';
    // Get matched filenames
    int16_t sequencePattern_idx{1};
    Filenames matchedPaths{};
    std::string temp_replacement{};
    for (auto& pair: filePaths)
    {
        fs::path path{pair.second};
        int16_t idx{pair.first};

        temp_replacement = convertSequenceNumber(replacement, sequencePattern_idx, matchNum);
        fullPath = getBetweenFilename(path, lpat, rpat, temp_replacement, plus);

        if (fullPath == "")
            continue;  // Skip if no match

        // Make sure new filename is different
        if (fullPath == path)
        {
            ++sequencePattern_idx;
            continue;
        }

        // Make sure multiple files are not named the same name:
        if (fs::exists(fullPath) || !checkMapItemUnique(matchedPaths, fullPath))
        {
            redErrorMessage("Cannot rename " + path.filename().string() + " (Filename " + 
                            fullPath.filename().string() + " already exists.)", false);
            ++sequencePattern_idx;
            continue;
        }

        matchedPaths[idx] = fullPath;
        printFileChange(path, fullPath);
        ++sequencePattern_idx;
    }

    // Print number of matches then ask to quit or continue
    if (checkIfQuit(matchedPaths.size()) )
        return;

    if (history.saveHistory)
        history.update(matchedPaths, filePaths);

    //Rename and print
    renameAndMenuUpdate(matchedPaths, filePaths);
}



void keywordCapOrLower(Filenames& filePaths, std::string_view pattern,
                       HistoryData& history)
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

    if (history.saveHistory)
        history.update(matchedPaths, filePaths);

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



void keywordSeries(Filenames& filePaths, HistoryData& history)
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
                redErrorMessage("Cannot rename \"" + old_filename + "\" (Filename \"" + new_filename + "\" already exists.)\n", false);
                continue;
            }

        matchedPaths[pair.first] = new_path;
    }
// End of dots code
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

            // make s01e01 lowercase
            new_filename.replace(sm.position(0), 1, "s");
            new_filename.replace(sm.position(0) + 3, 1, "e");
        }
        else
        {
            matchedPaths.erase(pair++);
            continue;
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
        replacement = "";
        // Skip if no match and make sure new filename is different
        if (fullPath == "" || fullPath == pair->second)
        {
            ++pair;
            continue;
        }

        // Make sure multiple files are not named the same name:
        if (fs::exists(fullPath) || !checkMapItemUnique(renamed_temp, fullPath))
        {
            redErrorMessage("Cannot rename " + path.filename().string() + " (Filename " + fullPath.filename().string() + " already exists.)", false);
            matchedPaths.erase(pair++);
            continue;
        }

        renamed_temp[idx] = fullPath;
        matchedPaths[idx] = fullPath;
        ++pair;
    }
// End between code
    // Print
    std::cout << '\n';
    int16_t idx{};
    for (auto pair = matchedPaths.cbegin(); pair != matchedPaths.cend(); )
    {
        idx = pair->first;
        if (filePaths[idx] == matchedPaths[idx])
        {
            redErrorMessage(filePaths[idx].filename().string() + " is already named properly.", false);
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
        redErrorMessage("No files to rename.");
        return;
    }

    // Print number of matches then ask to quit or continue
    if (checkIfQuit(matchedPaths.size()) )
        return;

    if (history.saveHistory)
        history.update(matchedPaths, filePaths);

    // Rename files and update menu
    renameAndMenuUpdate(matchedPaths, filePaths);
}


void keywordPrintToFile(Filenames& filePaths, bool& showNums, std::set<fs::path> directories)
{
    std::cout << "Enter separator for filenames (default: newline):\n> "; 
    std::string separator{};
    std::getline(std::cin, separator);
    if (separator == "")
        separator = "\n";

    printToFile(filePaths, directories, separator);
}



// Used with keywordRenameSubs
fs::path getFirstFolder()
{
    for (const auto& dir: fs::directory_iterator(".\\"))
    {
        if (dir.is_directory())
            return dir;
    }

    fs::path empty{};
    return empty;
}


void keywordRenameSubs(Filenames& filePaths, HistoryData& history)
{
    fs::path sub_directory{getFirstFolder()};

    setColor(Color::green);
    std::cout << "Default: " << sub_directory << '\n';
    resetColor();
    std::cout << "Enter path for directory containing subtitles (Blank for default):\n> "; 
    std::string query{};
    std::getline(std::cin, query);
    if (query == "")
        query = sub_directory.string();

    if (!fs::exists(query))
    {
        redErrorMessage("Directory doesn't exist.");
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
    std::cout << '\n';
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
        redErrorMessage("No filenames to change.");
        return;
    }

    // Print number of matches then ask to quit or continue
    if (checkIfQuit(size))
        return;

    if (history.saveHistory)
        history.update(newSubPaths, subtitlePaths);

    // Rename files and update menu
    renameAndMenuUpdate(newSubPaths, subtitlePaths);

}



void keywordRemoveDirectories(Filenames& filePaths, bool remove)
{
    setColor(Color::green);
    bool itemRemoved{};
    for (auto pair = filePaths.cbegin(); pair != filePaths.cend(); )
    {
        if ( fs::is_directory(pair->second) == remove)
        {
            std::cout << "Removed: " << pair->second << '\n';
            filePaths.erase(pair++);
            itemRemoved = true;
        }
        else
            ++pair;
    }
    if (!itemRemoved && remove)
        redErrorMessage("No directories to remove.");
    else if (!itemRemoved && !remove)
        redErrorMessage("No files to remove.");
}



void keywordWordCount(Filenames& filePaths)
{
    std::vector<fs::path> vectorPaths{};
    for (auto& p: filePaths)
        vectorPaths.push_back(p.second);

    TextCount wordCount(vectorPaths);
    wordCount.printInfo();
    // wordCount.printWords();
    // wordCount.printWords(0, "if");
    printPause();
}



void keywordHistory(HistoryData& history, Filenames& filePaths)
{
    if (history.historyData.empty())
    {
        redErrorMessage("There is no history.");
        return;
    }

    history.print();
    std::cout << "\nEnter an index number to undo rename. (Green examples can revert, red don't currently exist.)\n" 
                 "Type 'clear' to erase history.\n" 
                 "Or press ENTER to return to menu:\n> ";
    std::string query{};
    std::getline(std::cin, query);

    if (query == "q" || query == "")
        return;
    if (query == "clear")
    {
        history.clear();
        return;
    }

    std::int32_t index{};
    try
    {
        index = stoi(query);
        if (index >= history.historyData.size() || index < 0)
        {
            const std::exception e{};
            throw e;
        }
    }
    catch(const std::exception& e)
    {
        redErrorMessage("ERROR: Index number 0-" + std::to_string(history.historyData.size() - 1) + " required.");
        return;
    }
    fs::path renamedFile{history.historyData[index].begin()->second};
    if (!fs::exists(renamedFile))
    {
        redErrorMessage("Cannot undo because \"" + renamedFile.filename().string() + "\" has since been changed.");
        return;
    }
    
    undoRename(history, index, filePaths);
}



void keywordToggleHistory(HistoryData& history)
{
    history.toggle();

    if (history.saveHistory)
        redErrorMessage("History saving turned on.");
    else
        redErrorMessage("History saving paused.");
}



void keywordFind(std::string& pat, Filenames& filePaths, bool remove)
{
    int16_t sub_num{};
    if (remove) sub_num = 6;
    else sub_num = 5;
    std::string pattern{removeSpace(pat.substr(sub_num))};
    std::string pattern_temp{};

    if (pattern == "")
    {
        std::cout << "\nEnter a pattern to search (or q to quit):\n> "; 
        std::getline(std::cin, pattern);
        if (pattern == "q")
            return;
    }

    std::string name{};
    Filenames filePaths_temp{filePaths};
    std::string messageFilesRemoved{};
    bool matchFound{};
    for (auto pair = filePaths_temp.cbegin(); pair != filePaths_temp.cend(); )
    {
        name = lowercase(pair->second.filename().string());
        pattern_temp = convertPatternWithRegex(name, pattern);
        if (name.find(lowercase(pattern_temp)) == std::string::npos)
        {
            if (!remove)
            {
                messageFilesRemoved += "File removed: " + name + '\n';
                filePaths_temp.erase(pair++);
                matchFound = true;
                continue;
            }
        }
        else
        {
            if (remove)
            {
                messageFilesRemoved += "File removed: " + name + '\n';
                filePaths_temp.erase(pair++);
                matchFound = true;
                continue;
            }
        }
        ++pair;
    }

    if (filePaths_temp.empty())
    {
        if (!remove)
            redErrorMessage("No menu filenames match that pattern.");
        else
            redErrorMessage("All menu filenames match that pattern. Use a more specific pattern.");
        return;
    }

    if (!matchFound)
    {
        if (!remove)
            redErrorMessage("All menu filenames match that pattern. Use a more specific pattern.");
        else
            redErrorMessage("No menu filenames match that pattern.");
        return;
    }

    setColor(Color::green);
    std::cout << messageFilesRemoved;
    resetColor();
    filePaths = filePaths_temp;
}