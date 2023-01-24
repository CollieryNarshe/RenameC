#include "Colors.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class TextCount
{
public:
    std::vector<fs::path> pathNames{};
    std::int64_t lineCount{}; // total lines including blank lines
    std::int64_t blankLineCount{};
    std::int64_t wordCount{};
    std::int64_t charCount{};
    std::map<fs::path, std::int64_t> pathCharCounts{};
    std::map<fs::path, std::int64_t> pathWordCounts{};
    std::map<fs::path, std::int64_t> pathLineCounts{};
    std::map<fs::path, std::int64_t> pathBlankLineCounts{};

private:
    std::map<std::string, std::int64_t> wordListWithCounts{};
    std::vector<std::pair<std::string, std::int64_t>> wordListWithCounts_Pairs{};
    // std::string comepleteText{};

public:
    TextCount(std::vector<fs::path>& paths)
    {
        pathNames = paths;
        std::string word{};
        std::string line{};
        char character{};
        int32_t idx{};
        for (const auto& path : paths)
        {
            if (fs::is_directory(path))
            {
                pathNames.erase(pathNames.begin() + idx);
                continue;
            }
            std::ifstream fileData{};
            fileData.open(path, std::ios::in);

            if ( fileData.is_open() )
            {
                while(getline(fileData, line))
                {
                    if (line == "")
                    {
                        ++blankLineCount;
                        ++pathBlankLineCounts[path];
                        // continue;
                    }
                    ++lineCount;
                    ++pathLineCounts[path];
                    // if (comepleteText != "")
                    //     comepleteText += '\n';
                    // comepleteText += line;
                    std::stringstream stream(line);
                    while (stream >> word)
                    {
                        ++wordCount;
                        ++pathWordCounts[path];
                        ++wordListWithCounts[word];
                        for (char ch : word)
                        {
                            if (std::isalpha(ch) || std::ispunct(ch))
                            {
                                ++charCount;
                                ++pathCharCounts[path];
                            }
                        }
                    }
                }
                fileData.close();
            }
            else
            {
                std::cout << "Error opening file: " << path << '\n';
            }
        ++idx;
        }
        // Duplicate map list of words into vector to sort by value
        for (auto itr = wordListWithCounts.begin(); itr != wordListWithCounts.end(); ++itr)
            wordListWithCounts_Pairs.push_back(*itr);
        std::sort(wordListWithCounts_Pairs.begin(), 
                  wordListWithCounts_Pairs.end(), 
                  [=](std::pair<std::string, std::int64_t>& a, std::pair<std::string, std::int64_t>& b)
                     {return a.second > b.second;});
    }



    void printInfo()
    {
        for (const auto path: pathNames)
        {
            setColor(Color::green);
            std::cout << "Filename: " << path.filename().string() << '\n';
            resetColor();
            std::cout << "Lines:          " << pathLineCounts[path] << '\n';
            std::cout << "Blank lines:    " << pathBlankLineCounts[path] << '\n';
            std::cout << "Words:          " << pathWordCounts[path] << '\n';
            std::cout << "Characters:     " << pathCharCounts[path] << '\n';
            std::cout << '\n';
        }
        if (pathNames.size() > 1)
        {
            std::cout << "Total lines:       " << lineCount << '\n';
            std::cout << "Total blank lines: " << blankLineCount << '\n';
            std::cout << "Total words:       " << wordCount << '\n';
            std::cout << "Total characters:  " << charCount << '\n';
        }

    }



    void printWords(std::int64_t num = 20, std::string word = "")
    {
        if (word == "")
        {
            if (num > size(wordListWithCounts_Pairs) || num == 0)
                num = size(wordListWithCounts_Pairs);
            for (size_t n{}; n < num; ++n)
                std::cout << wordListWithCounts_Pairs[n].first << "  -  " << wordListWithCounts_Pairs[n].second << '\n';
        }
        else
        {
            if (wordListWithCounts.find("word") != wordListWithCounts.end())
                std::cout << word << "  -  " << wordListWithCounts[word] << '\n';
            else
                std::cout << "Could not find word: " << word << '\n';
        }
    }
};
