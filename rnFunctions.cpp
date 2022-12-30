#include <iostream>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector> 

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;


std::string strReplaceAll(const std::string& origin, const std::string& pat, 
                          const std::string& newPat, const size_t start = 0)
{
    std::string newString{ origin };

    if ( pat.empty() )
        return newString;
        
    size_t startPos{ start };
    while( (startPos = newString.find(pat, startPos) ) != std::string::npos )
    {
        newString.replace(startPos, pat.length(), newPat);
        startPos += newPat.length();
    }
    
    return newString;
}



void printFileChange(const fs::path oldPath, const std::string& newFilename)
{
    std::cout << oldPath.filename().string() 
              << " -->  " << newFilename << '\n';
}



std::string getReplacementInput(int16_t index)
{
    std::cout << "\n" << index << " filenames with this pattern will be renamed.\n"
                    "Press q to cancel or enter new pattern.\n"
                    "Change to: ";
    std::string replacement{};
    std::getline(std::cin, replacement);
    std::cout << '\n';

    return replacement;
}



std::string removeDotEnds(const fs::path& file, bool& dotAtStart)
{
    std::string filename;

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

    return filename;
}



std::string restoreDotEnds(const fs::path& file, std::string newFilename, 
                           bool& dotAtStart)
{
    if ( !fs::is_directory(file) )
        newFilename += file.extension().string();

    // Add dot prefix
    if (dotAtStart)
    {
        newFilename = "." + newFilename;
        dotAtStart = false;
    }

    return newFilename;
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
        std::cout << e.what() << '\n';
        return false;
    }
}



std::string renameFiles(const fs::path& file, const std::string& pat, 
                        const std::string& newPat)
{
    // Rename a string of filename
    std::string newFilename{ strReplaceAll(file.filename().string(), pat, newPat) };

    // Rename the actual files
    fs::path fullPath{ file.parent_path() /= newFilename };
    // fs::rename(file, fullPath);
    renameErrorCheck(file, fullPath);
    //test

    return newFilename;
}



std::map<int16_t, fs::path> getFilenames(const fs::path& dir)
{
    std::map<int16_t, fs::path> filePaths{};
    int16_t idx{};

    for ( const auto& path: fs::directory_iterator(dir) )
        {
            filePaths[idx] = path;
            ++idx;
        }
    return filePaths;
}



void printFilenames(const std::map<int16_t, fs::path>& paths, 
                    const bool showNums=false)
{
    std::cout << '\n';

    for (const auto& pair: paths)
    {
        // Print index number
        if (showNums)
            std::cout << pair.first << ". ";

        std::cout << pair.second.filename().string() << '\n';
    }
}



std::string deleteBetween(const fs::path& path, 
                          const std::string& lpat, const std::string& rpat)
{
    std::string filename {path.filename().string()};

    int leftIndex{static_cast<int>(filename.find(lpat))};
    int rightIndex{static_cast<int>(filename.rfind(rpat))};

    // Switch the pattern indexes if rightmost one is entered first
    if (leftIndex > rightIndex)
    {
        std::swap(leftIndex, rightIndex);
        leftIndex += rpat.length();
    }
    else
        leftIndex += lpat.length();
    
    // Edit filename text
    filename.erase(filename.begin() + leftIndex, filename.begin() + rightIndex);

    // Rename actual files
    fs::path fullPath{path.parent_path() /= filename};
    renameErrorCheck(path, fullPath);

    return filename;
}



void printPause()
{
    std::cout << "\nPress ENTER to continue...";
    std::cin;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\n\n==============================" << std::endl;
}



bool betweenQuitQuery(const Filenames& matchedPaths)
{
    if ( !matchedPaths.size() )
    {
        std::cout << "\nNo filenames contain this pattern.\n";
        printPause();
        return false;
    }

    std::cout << "\n" << matchedPaths.size() << " filenames with this pattern will be renamed.\n"
                 "Press q to cancel or ENTER to continue.\n";

    std::string query{};
    std::getline(std::cin, query);
    std::cout << '\n';

    if (query == "q")
        return false;
    
    return true;
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