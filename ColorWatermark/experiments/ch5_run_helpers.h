#pragma once

#include <opencv2/opencv.hpp>
#include <string>

inline cv::Mat ch5LoadCover512(const std::string& path) {
    cv::Mat m = cv::imread(path, cv::IMREAD_COLOR);
    if (m.empty()) return m;
    cv::resize(m, m, cv::Size(512, 512), 0, 0, cv::INTER_AREA);
    return m;
}
