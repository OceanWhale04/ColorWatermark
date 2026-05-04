#pragma once

#include <chrono>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <vector>

inline void expEnsureDir(const std::string& dir) {
    std::string normalized = dir;
    for (size_t i = 0; i < normalized.size(); ++i) {
        if (normalized[i] == '/') normalized[i] = '\\';
    }
    std::string current;
    for (size_t i = 0; i < normalized.size(); ++i) {
        current.push_back(normalized[i]);
        if (normalized[i] == '\\' || i + 1 == normalized.size()) {
            if (!current.empty() && current.back() == '\\') continue;
            _mkdir(current.c_str());
        }
    }
}

inline cv::Mat expLoadWatermark32(const std::string& path) {
    cv::Mat wm = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (wm.empty()) return wm;
    cv::resize(wm, wm, cv::Size(32, 32), 0, 0, cv::INTER_NEAREST);
    cv::threshold(wm, wm, 128, 255, cv::THRESH_BINARY);
    return wm;
}

inline void expCsvOpen(std::ofstream& out, const std::string& path) {
    expEnsureDir(path.substr(0, path.find_last_of("\\/")));
    out.open(path, std::ios::out | std::ios::trunc);
    out << std::fixed << std::setprecision(6);
}

inline double expElapsedMs(const std::chrono::steady_clock::time_point& t0) {
    using namespace std::chrono;
    return duration_cast<duration<double, std::milli>>(steady_clock::now() - t0).count();
}
