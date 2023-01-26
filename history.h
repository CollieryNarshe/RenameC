#ifndef HISTORY_H
#define HISTORY_H

#include "colors.h"
// #include <algorithm>
#include <cstddef>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using Filenames = std::map<int16_t, fs::path>;



class HistoryData
{
    using OldNewFiles = std::map<fs::path, fs::path>;

public:
    std::vector<OldNewFiles> historyData{};
    fs::path currentPath{};
    bool saveHistory{true};

    HistoryData(fs::path dir)
        {
            currentPath = dir.replace_filename("RenameHistory.txt");
            if (!fs::exists(currentPath))
            {
                std::ofstream fileData{currentPath};
                std::cout << "History file created: " << currentPath << '\n';
                fileData << "on\n";
                fileData.close();
            }
            loadFromFile();
        }

    void update(Filenames& newFiles, Filenames& oldFiles)
    {
        OldNewFiles historyUpdate{};
        for (auto& pair: newFiles)
        {
            historyUpdate[oldFiles[pair.first]] = pair.second;
        }
        if (historyData.size() >= 10 && !historyUpdate.empty())
            historyData.pop_back();
            
        historyData.insert(historyData.begin(), historyUpdate);
    }
    
    void saveToFile()
    {
        std::ofstream fileData{currentPath};
        if (saveHistory)
            fileData << "on\n";
        else
            fileData << "off\n";

        for (auto& history : historyData)
        {
            for (auto& pair : history)
            {
                fileData << pair.first.string() << '\n';
                fileData << pair.second.string() << '\n';
            }
            fileData << '\n';
        }
        fileData.close();
    }

    void loadFromFile()
    {
        std::ifstream fileData{};
        fileData.open(currentPath, std::ios::in);
        if ( fileData.is_open() )
        {
            std::string line{};
            std::string temp_string{};
            OldNewFiles historyPoint{};
            getline(fileData, line);
            if (line == "off")
                saveHistory = false;
            else if (line == "on")
                saveHistory = true;
            std::cout << line << '\n';

            while(getline(fileData, line))
            {
                if (line == "")
                {
                    if (historyPoint.empty())
                        break;
                    historyData.push_back(historyPoint);
                    historyPoint.clear();
                    continue;
                }
                if (temp_string == "")
                {
                    fs::path l{line};
                    temp_string = l.generic_string();
                }
                else
                {
                    fs::path l{line};
                    historyPoint[temp_string] = l.generic_string();
                    temp_string = "";
                }
            }
        fileData.close();
        }
        else
        {
            std::cout << "Error opening file: " << currentPath << "\n";
        }
    }

    void clear()
    {
        std::cout << "Are you sure you want to erase all your history?\n"
                     "This action is permanent. (q to quit)\n> "; 
        std::string replacement{};
        std::getline(std::cin, replacement);
        std::cout << '\n';
        if (replacement == "q")
            return;

        historyData.clear();
    }

    void toggle()
    {
        saveHistory = !saveHistory;
    }

    void removeEntry(std::int16_t index)
    {
        historyData.erase(historyData.begin() + index);
    }

    void print()
    {
        setColor(Color::blue);
        std::cout << "\nHistory:\n";
        resetColor();
        std::int32_t idx{};
        std::int16_t color{};
        bool exists{};
        for (auto& history : historyData)
        {
            if ( fs::exists(history.begin()->second) )
                color = Color::green;
            else
                color = Color::red;

            // setColor(Color::yellow);
            std::cout << idx;
            std::cout << ". Directory: ";
            std::cout << history.begin()->first.parent_path().generic_string() << '\n';
            setColor(color);
            std::cout << history.begin()->first.filename().string();
            resetColor();
            std::cout << " --> ";
            setColor(color);
            std::cout << history.begin()->second.filename().string() << '\n';
            resetColor();
            ++idx;
        }
    }

    std::pair<Filenames, Filenames> getFilenames(std::int32_t index)
    {
        std::int16_t idx{};
        std::pair<Filenames, Filenames> filePaths{};
        Filenames old_filenames{};
        Filenames new_filenames{};
        for (auto& pair : historyData[index])
        {
            old_filenames[idx] = pair.first;
            new_filenames[idx] = pair.second;
            ++idx;
        }
        filePaths = make_pair(old_filenames, new_filenames);
        return filePaths;
    }
};

#endif