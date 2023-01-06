#include "colors.h"
#include <algorithm>  // For transform
#include <iostream>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstddef>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;



void printPause()
{
    std::cout << "\nPress ENTER to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "==============================\n";
}



std::string lowercase(std::string s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}



std::string strReplaceAll(std::string origin, const std::string& pat, 
                          const std::string& newPat, const std::size_t start = 0)
{
    // Search is not case sensitive, but replacement pattern is
    std::string newString_lower{ lowercase(origin) };
    std::string pat_lower{ lowercase(pat) };

    if ( pat.empty() )
        return origin;
    
    std::size_t startPos{ start };
    while( (startPos = newString_lower.find(pat_lower, startPos) ) != std::string::npos )
    {
        origin.replace(startPos, pat.length(), newPat);
        newString_lower.replace(startPos, pat.length(), newPat);
        startPos += newPat.length();
    }
    
    return origin;
}



void printFileChange(const fs::path& oldPath, const fs::path& newPath)
{
    setColor(Color::pink);
    std::cout << oldPath.filename().string();
    setColor(Color::green);
    std::cout << "\n   ---->  " << newPath.filename().string() << '\n';
    resetColor();
}



bool checkForMatches(const Filenames& matchedPaths)
{
    if ( !matchedPaths.size() )
    {
        setColor(Color::red);
        std::cout << "\nNo filenames contain this pattern.\n";
        resetColor();
        printPause();
        return false;
    }
    return true;
}



bool checkIfQuit(std::size_t index)
{
    setColor(Color::red);
    std::cout << "\n" << index << " filenames will be renamed.\n";
    resetColor();
    std::cout << "Enter q to quit or ENTER to continue:\n> ";
    std::string query{};
    std::getline(std::cin, query);
    std::cout << '\n';

    if (query == "q")
        return true;
    return false;
}



void removeDotEnds(fs::path& file, bool& dotAtStart)
{
    std::string filename{};

    if ( fs::is_directory(file) )
        filename = file.filename().string();
    else
        filename = file.stem().string();

    // Remove dot prefix
    if ( filename.starts_with('.') )
    {
        filename.erase(0, 1);
        dotAtStart = true;
    }

    file.replace_filename(filename);
}



void restoreDotEnds(fs::path& newFile, const fs::path& file, 
                    bool dotAtStart)
{
    // Add file extension
    if ( !fs::is_directory(file) )
        newFile.replace_filename(newFile.filename() += file.extension());

    // Add dot prefix
    if (dotAtStart)
        newFile.replace_filename("." + newFile.filename().string());
}



bool renameErrorCheck(fs::path path, fs::path new_path)
{
    try
    {
        // std::cout << path << "  |  " << new_path << '\n'; // test
        fs::rename(path, new_path);
        return true;
    }
    catch(const std::exception& e)
    {
        setColor(Color::red);
        std::cout << e.what() << '\n';
        resetColor();
        return false;
    }
}



fs::path renameFile(const fs::path& file, const std::string& pat, 
                        const std::string& newPat)
{
    std::string filename{file.filename().string()};
    fs::path newFile{file};

    // Rename a string of filename
    if (pat == "#begin")
        newFile.replace_filename(newPat + filename);
    else if (pat == "#ext")
        newFile.replace_filename(file.stem().string() + newPat);
    else if (pat == "#end")
    {
        filename = file.stem().string();
        newFile.replace_filename(filename + newPat + file.extension().string());
    }
    else
        newFile.replace_filename(strReplaceAll(filename, pat, newPat));

    return newFile;
}



Filenames getFilenames(const std::set<fs::path>& dirs)
{
    Filenames filePaths{};
    int16_t idx{};
    for (auto& dir: dirs)
    {
        for ( const auto& path: fs::directory_iterator(dir) )
            {
                filePaths[idx] = path;
                ++idx;
            }
    }
    return filePaths;
}


void removeFileByName(Filenames& files, const fs::path path)
{
    for (auto& f : files)
    {
        if (f.second.filename() == path.filename())
        {
            files.erase(f.first);
            return;
        }
    }
}


void printFilenames(const Filenames& paths, 
                    const bool showNums=false)
{
    setColor(Color::cyan);
    std::cout << '\n';

    for (const auto& pair: paths)
    {
        // Print index number
        if (showNums)
            std::cout << pair.first << ". ";

        std::cout << pair.second.filename().string() << '\n';
    }
    resetColor();
}



fs::path getBetweenFilename(const fs::path& path, 
                          const std::string& lpat, const std::string& rpat,
                          const std::string& replacement)
{
    std::string filename {path.filename().string()};

    // Get index of patterns
    std::size_t leftIndex{lowercase(filename).find(lowercase(lpat))};
    std::size_t rightIndex{lowercase(filename).rfind(lowercase(rpat))};

    // Check if matched
    bool lmatch{leftIndex != std::string::npos};
    bool rmatch{rightIndex != std::string::npos};
    if (!(lpat == "#begin" && rmatch) &&
        !(lmatch && (rpat == "#end")) &&
        !(lmatch && rmatch) )
        return ""; 

    // Adjust for pattern keywords
    if (lpat == "#begin")
        leftIndex = 0;
    if (rpat == "#end")
        rightIndex = path.stem().string().length();

    // Switch the pattern indexes if rightmost one is entered first
    if ( leftIndex > rightIndex )
    {
        std::swap(leftIndex, rightIndex);
        leftIndex += rpat.length();
    }
    else if (lpat != "#begin")
        leftIndex += lpat.length();


    // Edit filename string
    filename.erase(filename.begin() + leftIndex, filename.begin() + rightIndex);
    filename.insert(leftIndex, replacement);

    fs::path fullPath{path.parent_path() /= filename};
    return fullPath;
}



// Used with splitString to remove spaces from ends of a string
std::string removeSpace(std::string s)
{
    s.erase(s.find_last_not_of(' ') + 1);
    s.erase( 0, s.find_first_not_of(' ') );
    return s;
}



// Returns a vector of a string split along a delimiter
std::vector<std::string> splitString(const std::string& str, 
                                     const std::string& delimiter, 
                                     bool removeSpaces = true)
{
    std::vector<std::string> splitLines{};
    std::string s{};
    int16_t idx{};
    int16_t prev{};
    
    while( ( idx = str.find(delimiter, prev) ) != std::string::npos )
    {
        s = str.substr(prev, idx - prev);
        if (removeSpaces)
            s = removeSpace(s);
        splitLines.push_back( s );
        prev = idx + delimiter.length();
    }

    // Add last substring
    s = str.substr(prev);
    if (removeSpaces)
            s = removeSpace(s);
    splitLines.push_back( s );

    return splitLines;
}


void toLowercase(std::string& s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
}


void capitalize(std::string& s)
{
    for (std::size_t idx{}; idx < s.length(); ++idx)
    {
        if ( (idx == 0) && (std::islower(s[idx])) )
            s[0] = std::toupper(s[0]);

        else if ( (s[idx - 1] == ' ') && (std::islower(s[idx])) )
            s[idx] = std::toupper(s[idx]);
    }
}


