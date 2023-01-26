#include "colors.h"
#include "history.h"
#include <algorithm>  // For transform
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

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


// Used with checkPatternWithRegex and convertPatternWithRegex and extractDigits
// Makes entire string pattern regex literal (except ? which is any digit)
std::string makeRegex(const std::string& pattern)
{
    std::string newPattern{};
    std::string character{};
    for (const char& c : pattern)
    {
        character = c;
        if (c == '\\' || c == '/' || c == '^' || c == ']')
            newPattern += ("[\\" + character + "]");
        else if (c == '?')
            newPattern += ("([0-9])");
        else
            newPattern += ("[" + character + "]");
    }
    return newPattern;
}



std::vector<std::string> extractDigits(const std::string& filename, const std::string& pattern)
{
    std::vector<std::string> digits{};
    std::regex regexPat{makeRegex(pattern)};
    std::smatch sm{};
    std::regex_search(filename, sm, regexPat);

    for (size_t x{1}; x < sm.size(); ++x )
    {
        digits.push_back(sm[x]);
    }
    return digits;
}


std::string replaceDigits(const std::vector<std::string>& digits, std::string pat)
{
    if ( pat.empty() )
        return pat;

    auto newDigit{digits.begin()};
    std::size_t startPos{};
    while( (startPos = pat.find("?", startPos) ) != std::string::npos )
    {
        if (newDigit == digits.end())
            newDigit = digits.begin();

        pat.replace(startPos, 1, *newDigit);
        ++startPos;
        ++newDigit;
    }
    return pat;
}



void printFileChange(const fs::path& oldPath, const fs::path& newPath)
{
    setColor(Color::pink);
    std::cout << oldPath.filename().string();
    setColor(Color::green);
    std::cout << "\n   ---->" << newPath.filename().string() << '\n';
    resetColor();
}



// bool check for pattern, converting ? into any number
bool checkPatternWithRegex(const std::string& filename, const std::string& pattern)
{
    std::regex patternRegEx{ makeRegex(pattern) };
    std::smatch sm;
    std::regex_search(filename, sm, patternRegEx);
    if (sm[0] == "")
        return false;
    return true;
}



// Convert pattern, converting ? into number
std::string convertPatternWithRegex(std::string filename, std::string pattern,
                                    bool lower = true, bool right = false)
{
    if (lower)
    {
        toLowercase(filename);
        toLowercase(pattern);
    }
    std::regex patternRegEx{ makeRegex(pattern) };

    std::smatch sm;

    // Get first occurance of pattern
    if (!right)
    {
        std::regex_search(filename, sm, patternRegEx);
        if (sm[0] == "")
            return pattern;
        return sm[0];
    }
    
    // Get last occurance of pattern (simulates rfind)
    std::string match{};
    while (std::regex_search(filename, sm, patternRegEx))
    {
        for (auto m : sm)
        {
            // match = m.str();
            match = sm[0];
        }
        filename = sm.suffix().str();
    }
    if (match == "")
        return pattern;
    return match;

}


bool checkForMatches(const Filenames& matchedPaths)
{
    if ( !matchedPaths.size() )
    {
        setColor(Color::red);
        std::cout << "\nError 2: No filenames contain this pattern.\n";
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
    setColor(Color::blue);
    std::cout << "\n" << index << " filenames will be renamed.\n";
    resetColor();
    std::cout << "Press ENTER to continue (or q to quit):\n> ";
    std::string query{};
    std::getline(std::cin, query);
    std::cout << '\n';

    if (query != "")
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


std::int16_t getIndex(const std::string& pattern)
{
    std::int16_t index{1000};

    try
    {
        if (pattern.rfind("#index", 0) == 0)
            index = stoi(pattern.substr(6));
    }
    catch(const std::exception& e)
    {
        setColor(Color::red);
        std::cerr << "Error with index conversion: " << e.what() << '\n';
        resetColor();
    }
    return index;
}


fs::path renameFile(fs::path filePath, const std::string& pat, 
                    std::string newPat)
{
    std::string filename{filePath.filename().string()};
    std::string filenameStem{filePath.stem().string()};
    std::int16_t set_index{getIndex(pat)};

    // Replace NewPat keywords
    if (newPat == "#begin" || newPat == "#end")
        newPat = "";
    else if (newPat == "#ext")
        newPat = filePath.extension().string();

    // Rename a string of filename
    if (pat == "#begin")
        filePath.replace_filename(newPat + filename);
    else if (pat == "#ext")
        filePath.replace_filename(filenameStem + newPat);
    else if (pat == "#end")
        filePath.replace_filename(filenameStem + newPat + filePath.extension().string());
    else if (pat.rfind("#index", 0) == 0)
        filePath.replace_filename(filename.substr(0, set_index) + newPat + filename.substr(set_index));
    else
        filePath.replace_filename(strReplaceAll(filename, pat, newPat));

    return filePath;
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
    rpat = convertPatternWithRegex(filename, rpat, true, true);
    // Get index of patterns
    std::size_t leftIndex{lowercase(filename).find(lowercase(lpat))};
    std::size_t rightIndex{lowercase(filename).rfind(lowercase(rpat))};

    // Check if matched
    bool lmatch{leftIndex != std::string::npos};
    bool rmatch{rightIndex != std::string::npos};

    std::int16_t set_indexL{getIndex(lpat)};
    std::int16_t set_indexR{getIndex(rpat)};

    bool lKeywordIndex{lpat.rfind("#index", 0) == 0};
    bool rKeywordIndex{rpat.rfind("#index", 0) == 0};

    // Check if #index is inside of the other pattern
    std::size_t lLen{lpat.length()};
    std::size_t rLen{rpat.length()};
    if (!lKeywordIndex && rKeywordIndex && (lpat != "#end") && (lpat != "#ext") && (lpat != "#begin") &&
         !((set_indexR < leftIndex) || (set_indexR > leftIndex + lLen)))
        return false; 

    if (lKeywordIndex && !rKeywordIndex && (rpat != "#end") && (rpat != "#ext") && (rpat != "#begin") &&
         !((set_indexL < rightIndex) || (set_indexL > rightIndex + lLen)))
        return false; 

    // switch indexes if both set with #index but in wrong order:
    if ((set_indexL > set_indexR) && (lKeywordIndex && rKeywordIndex))
        std::swap(set_indexL, set_indexR);
    if (lKeywordIndex && (set_indexL <= filename.length()))
        lmatch = true;
    if (rKeywordIndex && set_indexR <= filename.length())
        rmatch = true;
    if (lpat == "#begin" || lpat == "#end" || lpat == "#ext")
        lmatch = true;
    if (rpat == "#begin" || rpat == "#end" || rpat == "#ext")
        rmatch = true;

    // Make sure both patterns aren't #index with same number
    if ( lKeywordIndex && rKeywordIndex && (lpat == rpat) )
        lmatch = false;

    // indexes not the same AND no keyword #index
    if ((lmatch && rmatch) && ((leftIndex != rightIndex) && (!lKeywordIndex && !rKeywordIndex)))
        return true; 
    else if ((lmatch && rmatch) && (lKeywordIndex || rKeywordIndex))
        return true;
    return false;
}


fs::path getBetweenFilename(const fs::path& path, 
                            std::string lpat, std::string rpat,
                            std::string replacement, bool plus)
{
    std::string filename {path.filename().string()};

    // extract digits into vector to use with ? in replacement pattern
    std::vector<std::string> digits{};
    std::vector<std::string> rdigits{};
    bool lpatHasQ{lpat.find("?") != std::string::npos};
    bool rpatHasQ{rpat.find("?") != std::string::npos};
    bool replaceHasQ{replacement.find("?") != std::string::npos};
    if ( (lpatHasQ || rpatHasQ) && replaceHasQ )
    {
        digits = extractDigits(lowercase(filename), lowercase(lpat));
        rdigits = extractDigits(lowercase(filename), lowercase(rpat));
        digits.insert(digits.end(), rdigits.begin(), rdigits.end());
    }

    // Convert any ? into numerical digit
    lpat = convertPatternWithRegex(filename, lpat);
    rpat = convertPatternWithRegex(filename, rpat, true, true);

    // Get index of patterns
    std::size_t leftIndex{lowercase(filename).find(lowercase(lpat))};
    std::size_t rightIndex{lowercase(filename).rfind(lowercase(rpat))};

    // Check if matched
    bool lKeywordIndex{ lpat.rfind("#index", 0) == 0 };
    bool rKeywordIndex{ rpat.rfind("#index", 0) == 0 };
    bool lmatch{leftIndex != std::string::npos};
    bool rmatch{rightIndex != std::string::npos};
    std::int16_t set_indexL{getIndex(lpat)};
    std::int16_t set_indexR{getIndex(rpat)};
    if (lKeywordIndex && set_indexL <= filename.length())
        lmatch = true;
    if (rKeywordIndex && set_indexR <= filename.length())
        rmatch = true;
    if (lpat == "#begin" || lpat == "#end" || lpat == "#ext")
        lmatch = true;
    if (rpat == "#begin" || rpat == "#end" || rpat == "#ext")
        rmatch = true;
    if (!(lmatch && rmatch))
        return "";

    // Adjust for pattern keywords
    if (lpat == "#begin")
        leftIndex = 0;
    if (rpat == "#begin")
        rightIndex = 0;
    if (rpat == "#end" || rpat == "#ext" )
        rightIndex = path.stem().string().length();
    if (lpat == "#end" || lpat == "#ext" )
        leftIndex = path.stem().string().length();
    if (lKeywordIndex)
        leftIndex = getIndex(lpat);
    if (rKeywordIndex)
        rightIndex = getIndex(rpat);

    if (leftIndex == rightIndex)
        return "";

    // Switch the pattern indexes if rightmost one is entered first
    // (plus) check to include patterns to replace
    if ( leftIndex > rightIndex)
    {
        std::swap(leftIndex, rightIndex);
        if (!lKeywordIndex && !rKeywordIndex)
        {
            if (plus && lpat != "#begin" && lpat != "#end" && lpat != "#ext")
                rightIndex += lpat.length();
            else if (rpat != "#begin" && rpat != "#end" && rpat != "#ext")
                leftIndex += rpat.length();
        }
        else if (lKeywordIndex && !rKeywordIndex)
            leftIndex += rpat.length();
    }
    else if (plus && !rKeywordIndex)
        rightIndex += rpat.length();
    else if (lpat != "#begin" && lpat != "#end" && lpat != "#ext" && !lKeywordIndex)
        leftIndex += lpat.length();

    // Handle replacement keyword patterns
    if (replacement == "#begin" || replacement == "#end")
        replacement = "";
    else if (replacement == "#ext")
        replacement = path.extension().string();

    // Replace ? with digits in replacement pattern
    if ( (lpatHasQ || rpatHasQ) && replaceHasQ )
        replacement = replaceDigits(digits, replacement);

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

    const std::string filename{filePath.filename().string()};
    fs::path newFile{filePath};
    std::int16_t set_index{getIndex(pat)};

    if (pat == "#begin")
    {
        setColor(Color::blue);
        std::cout << "*";
        resetColor();
        std::cout << filename << '\n';
        return;
    }
    else if (pat == "#end")
    {
        std::cout << filePath.stem().string();
        setColor(Color::blue);
        std::cout << "*";
        resetColor();
        std::cout << filePath.extension().string() << '\n';
        return;
    }
    else if (pat == "#ext")
    {
        std::cout << filePath.stem().string();
        setColor(Color::blue);
        std::cout << filePath.extension().string() << '\n';
        resetColor();
        return;
    }
    else if (pat.rfind("#index", 0) == 0)
    {
        std::cout << filename.substr(0, set_index);
        setColor(Color::blue);
        std::cout << "*";
        resetColor();
        std::cout << filename.substr(set_index) << '\n';
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
    // std::int16_t set_index1{getIndex(pattern)};

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
    else if (pattern.rfind("#index", 0) == 0)
    {
        index = getIndex(pattern);
        pLength = 0;
    }
}



void betweenPrintFilenameWithColor(const fs::path& filePath, std::string pattern1,
                                std::string pattern2)
{   
    // Convert any ? into digit
    pattern1 = convertPatternWithRegex(filePath.filename().string(), pattern1);
    pattern2 = convertPatternWithRegex(filePath.filename().string(), pattern2, true, true);

    std::string filename{filePath.filename().string()};
    size_t index{lowercase(filename).find(lowercase(pattern1))};
    size_t index2{lowercase(filename).rfind(lowercase(pattern2))};
    size_t pLength{pattern1.length()};
    size_t pLength2{pattern2.length()};
    adjustForPatternKeywords(filePath, pattern1, index, pLength);
    adjustForPatternKeywords(filePath, pattern2, index2, pLength2);
    if (index > index2)
    {
        pattern1.swap(pattern2);
        std::swap(index, index2);
        std::swap(pLength, pLength2);
    }

    size_t patEnd{index + pLength};
    size_t pat2End{index2 + pLength2};
    size_t midLength{index2 - patEnd};

    // If pattern1 not in filename:
    if (index == std::string::npos)
    {
        std::cout << filename << "\n";
        return;
    }

    // Print the filename
    std::cout << filename.substr(0, index);            //first part before pat
    setColor(Color::blue);
    if (pLength == 0)
        std::cout << "*";
    else
        std::cout << filename.substr(index, pLength);      //pattern1
    resetColor();

    // If pattern2 not in filename:
    if (index2 == std::string::npos)
    {
        std::cout << filename.substr(patEnd) << '\n';
        return;
    }

    // Print second part
    setColor(Color::red);
    std::cout << filename.substr(patEnd, midLength);    //center of pat
    setColor(Color::blue);
    if (pLength2 == 0)
        std::cout << "*";
    else
        std::cout << filename.substr(index2, pLength2); //pattern2
    resetColor();
    std::cout << filename.substr(pat2End) << '\n';      //end of filename
}


void undoRename(HistoryData& history, std::int16_t index)
{
    std::pair<Filenames, Filenames> oldNewFilenames{};
    oldNewFilenames = history.getFilenames(index);
    Filenames oldFiles{oldNewFilenames.first};
    Filenames newFiles{oldNewFilenames.second};
    
    std::cout << '\n';
    for (auto& pair : oldFiles)
    {
        printFileChange(newFiles[pair.first], pair.second);
    }

    // Chance to quit.
    if ( checkIfQuit(oldFiles.size()) )
        return;

    // Rename files.
    for (auto& pair: oldFiles)
    {
        renameErrorCheck(newFiles[pair.first], pair.second);
    }

    // Remove from history.
    history.removeEntry(index);
}