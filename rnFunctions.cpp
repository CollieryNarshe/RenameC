#include "colors.h"
#include <algorithm>  // For transform
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
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

void lowerCase(std::string& s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
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

// Used with checkPatternWithRegex and convertPatternWithRegex
// Makes entire string pattern regex literal (except ? which is any digit)
std::string makeRegex(const std::string& filename, const std::string& pattern)
{
    std::string newPattern{};
    std::string character{};
    for (const char& c : pattern)
    {
        character = c;
        if (c == '\\' || c == '/' || c == '^' || c == ']')
            newPattern += ("[\\" + character + "]");
        else if (c == '?')
            newPattern += ("[0-9]");
        else
            newPattern += ("[" + character + "]");
    }
    return newPattern;
}

// bool check for pattern, converting ? into any number
bool checkPatternWithRegex(const std::string& filename, const std::string& pattern)
{
    std::regex patternRegEx{ makeRegex(filename, pattern) };
    std::smatch sm;
    std::regex_search(filename, sm, patternRegEx);
    if (sm[0] == "")
        return false;
    return true;
}

// Convert pattern, converting ? into number
std::string convertPatternWithRegex(std::string filename, std::string pattern,
                                    bool lower = true)
{
    if (lower)
    {
        lowerCase(filename);
        lowerCase(pattern);
    }
    std::regex patternRegEx{ makeRegex(filename, pattern) };
    std::smatch sm;
    std::regex_search(filename, sm, patternRegEx);
    if (sm[0] == "")
        return pattern;
    return sm[0];
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
    if (!index)
    {
        setColor(Color::red);
        std::cout << "\nNo filenames to change.\n";
        resetColor();
        printPause();
        return true;
    }
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
    int32_t idx{};
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



bool checkBetweenMatches(const fs::path& path, 
                         std::string lpat, std::string rpat)
{
    std::string filename {path.filename().string()};

    // Convert any ? into numerical digit
    lpat = convertPatternWithRegex(filename, lpat);
    rpat = convertPatternWithRegex(filename, rpat);
    
    // Get index of patterns
    std::size_t leftIndex{lowercase(filename).find(lowercase(lpat))};
    std::size_t rightIndex{lowercase(filename).rfind(lowercase(rpat))};

    // Check if matched
    bool lmatch{leftIndex != std::string::npos};
    bool rmatch{rightIndex != std::string::npos};
    if (
        (lmatch && rmatch) || 
        (lpat == "#begin" && rmatch) ||
        (lmatch && ((rpat == "#end") || (rpat == "#ext"))) ||
        (lpat == "#begin" && ((rpat == "#end") || (rpat == "#ext")))
       )
        return true; 
    return false;
}


fs::path getBetweenFilename(const fs::path& path, 
                          std::string lpat, std::string rpat,
                          const std::string& replacement, bool plus)
{
    std::string filename {path.filename().string()};

    // Convert any ? into numerical digit
    lpat = convertPatternWithRegex(filename, lpat);
    rpat = convertPatternWithRegex(filename, rpat);

    // Get index of patterns
    std::size_t leftIndex{lowercase(filename).find(lowercase(lpat))};
    std::size_t rightIndex{lowercase(filename).rfind(lowercase(rpat))};

    // Check if matched
    bool lmatch{leftIndex != std::string::npos};
    bool rmatch{rightIndex != std::string::npos};
    if (
        !(lpat == "#begin" && rmatch) &&
        !(lmatch && ((rpat == "#end") || (rpat == "#ext"))) &&
        !(lmatch && rmatch) &&
        !(lpat == "#begin" && ((rpat == "#end") || (rpat == "#ext")))
       )
        return "";

    // Adjust for pattern keywords
    if (lpat == "#begin")
        leftIndex = 0;
    if (rpat == "#end" || rpat == "#ext" )
        rightIndex = path.stem().string().length();

    // Switch the pattern indexes if rightmost one is entered first
    // (plus) check to include patterns to replace
    if ( leftIndex > rightIndex )
    {
        std::swap(leftIndex, rightIndex);
        if (plus)
            rightIndex += lpat.length();
        else
            leftIndex += rpat.length();
    }
    // else if (lpat != "#begin")
    else if (plus)
        rightIndex += rpat.length();
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
    int32_t idx{};
    int32_t prev{};
    
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


void printToFile(Filenames& filePaths, std::set<fs::path> directories, 
                 std::string separator)
{
    std::ofstream newFile;
    newFile.open("RenameFileList.txt");
    if (!newFile.is_open())
    {
        setColor(Color::red);
        std::cout << "Error opening file.\n";
        resetColor();
        printPause();
        return;
    }

    // Write directory paths
    newFile << "Directories:\n";
    for (auto& path: directories)
    {
        newFile << path.generic_string() << '\n';
    }

    newFile << '\n';

    // Write filenames
    for (auto& pair: filePaths)
    {
        newFile << pair.second.filename().string() << separator;
    }
}



Filenames replaceSubtitleFilenames(Filenames& filePaths, Filenames subtitlePaths)
{
    size_t sSize{ ( std::min(filePaths.size(), subtitlePaths.size()) )};
    std::vector<fs::path> newSubPaths{};
    fs::path newFilename{};

    size_t idx{};
    for (auto& pair : filePaths)
    {
        if (fs::is_directory(pair.second))
            continue;
        newFilename = pair.second.stem().string() + subtitlePaths[idx].extension().string();
        subtitlePaths[idx].replace_filename(newFilename);
        ++idx;
        if (idx >= sSize)
            break;
    }
    return subtitlePaths;
}



void defaultPrintFilenameWithColor(const fs::path& filePath, std::string pat)
{
    if ( pat.empty() )
        return;

    // Convert any ? into digit
    pat = convertPatternWithRegex(filePath.filename().string(), lowercase(pat));
    std::cerr << pat << " TEST: pat after regex\n";

    const std::string filename{filePath.filename().string()};
    fs::path newFile{filePath};

    // Rename a string of filename
    if (pat == "#begin")
    {
        std::cout << filename;
        return;
    }
    else if (pat == "#end")
    {
        std::cout << filename;
        return;
    }
    else if (pat == "#ext")
    {
        std::cout << filePath.stem();
        setColor(Color::blue);
        std::cout << filePath.extension();
        resetColor();
        return;
    }

    // Print filename, with each pattern in blue
    std::string filename_lower{ lowercase(filename) };
    std::size_t patPos{};
    std::size_t prevEnd{};
    std::size_t whiteLength{};

    while( (patPos = filename_lower.find(pat, patPos) ) != std::string::npos )
    {
        whiteLength = patPos - prevEnd;
        std::cout << filename.substr(prevEnd, whiteLength);
        setColor(Color::blue);
        std::cout << filename.substr(patPos, pat.length());
        resetColor();
        prevEnd = patPos + pat.length();
        patPos = prevEnd;
    }
    whiteLength = filename.length() - prevEnd;
    std::cout << filename.substr(prevEnd, whiteLength) << '\n';
}


// Used with betweenPrintFilenameWithColor
void adjustForPatternKeywords(const fs::path& filePath, const std::string& pattern, 
                              size_t& index, size_t& pLength)
{
    if (pattern == "#begin")
    {
        index = 0;
        pLength = 0;
    }
    else if (pattern == "#end")
    {
        index = filePath.stem().string().length();
        pLength = 0;
    }
    else if (pattern == "#ext")
    {
        index = filePath.stem().string().length();
        pLength = filePath.extension().string().length();
    }
}



void betweenPrintFilenameWithColor(const fs::path& filePath, std::string pattern1,
                                std::string pattern2)
{   
    // Convert any ? into digit
    pattern1 = convertPatternWithRegex(filePath.filename().string(), pattern1);

    std::string filename{filePath.filename().string()};
    size_t index{lowercase(filename).find(lowercase(pattern1))};
    size_t pLength{pattern1.length()};
    adjustForPatternKeywords(filePath, pattern1, index, pLength);
    size_t pat1End{index + pLength};

    // If pattern1 not in filename:
    if (index == std::string::npos)
    {
        std::cout << filename << "\n";
        return;
    }

    // Print the filename
    std::cout << filename.substr(0, index);            //first part before pat
    setColor(Color::blue);
    std::cout << filename.substr(index, pLength);      //pattern1
    resetColor();
    if (pattern2 == "")
    {
        std::cout << filename.substr(pat1End) << '\n'; //Rest of filename after pattern1
        return;
    }

    // Second pattern
    pattern2 = convertPatternWithRegex(filePath.filename().string(), pattern2);
    index = lowercase(filename).find(lowercase(pattern2));
    pLength = pattern2.length();
    adjustForPatternKeywords(filePath, pattern2, index, pLength);
    size_t pat2End{index + pLength};


    // If pattern2 not in filename:
    if (index == std::string::npos)
    {
        std::cout << filename.substr(pat1End) << '\n';
        return;
    }

    // Print second part
    setColor(Color::red);
    std::cout << filename.substr(pat1End, index - pLength);           //center of pat
    setColor(Color::blue);
    std::cout << filename.substr(index, pLength);           //pattern2
    resetColor();
    std::cout << filename.substr(index + pLength) << '\n';  //end of filename
}