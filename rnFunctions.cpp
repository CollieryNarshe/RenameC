#include <algorithm>  // For transform
#include <iostream>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector> 

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;



std::string lowercase(std::string s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}



std::string strReplaceAll(std::string origin, const std::string& pat, 
                          const std::string& newPat, const size_t start = 0)
{
    // Search is not case sensitive, but replacement pattern is
    std::string newString_lower{ lowercase(origin) };
    std::string pat_lower{ lowercase(pat) };

    if ( pat.empty() )
        return origin;
    
    size_t startPos{ start };
    while( (startPos = newString_lower.find(pat_lower, startPos) ) != std::string::npos )
    {
        origin.replace(startPos, pat.length(), newPat);
        newString_lower.replace(startPos, pat.length(), newPat);
        startPos += newPat.length();
    }
    
    return origin;
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
    std::string filename{file.filename().string()};
    fs::path newFile{file};

    // Rename a string of filename
    if (pat == "#begin")
        newFile.replace_filename(newPat + filename);
    if (pat == "#ext")
        newFile.replace_filename(file.stem().string() + newPat);
    else if (pat == "#end")
    {
        filename = file.stem().string();
        newFile.replace_filename(filename + newPat + file.extension().string());
    }
    else
        newFile.replace_filename(strReplaceAll(filename, pat, newPat));

    // Rename the actual files
    renameErrorCheck(file, newFile);

    return newFile.filename().string();
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

    int leftIndex{static_cast<int>(lowercase(filename).find(lowercase(lpat)))};
    int rightIndex{static_cast<int>(lowercase(filename).rfind(lowercase(rpat)))};

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


void capitalize(std::string& s, bool allLower=false)
{
    if (allLower)
    {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        return;
    }

    for (size_t idx{}; idx < s.length(); ++idx)
    {
        if ( (idx == 0) && (std::islower(s[idx])) )
            s[0] = std::toupper(s[0]);

        else if ( (s[idx - 1] == ' ') && (std::islower(s[idx])) )
            s[idx] = std::toupper(s[idx]);
    }
}


