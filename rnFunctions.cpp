#include <iostream>
#include <filesystem>
#include <set>
#include <string>
#include <vector> 

namespace fs = std::filesystem;



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



std::string getReplacementInput(const std::vector<fs::path>& matchedPaths)
{
    std::cout << "\n" << matchedPaths.size() << " filenames with this pattern will be renamed.\n"
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



std::vector<fs::path> getFilenames(const std::string& dir)
{
    std::vector<fs::path> filePaths{};

    for ( const auto& path: fs::directory_iterator(dir) )
        {
            filePaths.push_back(path);
        }
    return filePaths;
}



void printFilenames(const std::vector<fs::path>& paths, 
                    const std::set<int>& removedFiles, 
                    const bool showNums=false)
{
    std::cout << '\n';

    int idx{};
    for (const auto& file: paths)
    {
        // Check if user removed file from list
        if ( !removedFiles.count(idx) )
        {
            // Print index number
            if (showNums)
                std::cout << idx << ". ";

            std::cout << file.filename().string() << '\n';
        }
        ++idx;
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



bool betweenQuitQuery(std::vector<fs::path> matchedPaths)
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