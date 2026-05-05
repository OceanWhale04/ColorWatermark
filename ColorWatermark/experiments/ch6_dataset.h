#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

inline bool ch5LoadImageList(const std::string& csvPath,
                             std::vector<std::pair<std::string, std::string>>& out,
                             std::string& err) {
    out.clear();
    std::ifstream f(csvPath.c_str());
    if (!f) {
        err = "Cannot open: " + csvPath;
        return false;
    }
    std::string line;
    if (!std::getline(f, line)) {
        err = "Empty CSV";
        return false;
    }
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string name, path;
        if (!std::getline(ss, name, ',')) continue;
        if (!std::getline(ss, path)) continue;
        while (!name.empty() && (name.front() == ' ' || name.front() == '\t')) name.erase(name.begin());
        while (!name.empty() && (name.back() == ' ' || name.back() == '\t')) name.pop_back();
        while (!path.empty() && (path.front() == ' ' || path.front() == '\t')) path.erase(path.begin());
        while (!path.empty() && (path.back() == ' ' || path.back() == '\t')) path.pop_back();
        if (!name.empty() && !path.empty())
            out.push_back({ name, path });
    }
    if (out.empty()) {
        err = "No rows in CSV";
        return false;
    }
    return true;
}
