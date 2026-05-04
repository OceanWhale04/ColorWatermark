#pragma once
#include <string>
#include <opencv2/core.hpp>

struct AppConfig {
    std::string mode;            // "embed", "extract", "compare_clahe", "compare_wiener"
    std::string inputPath;
    std::string outputPath;
    std::string watermarkPath;
    std::string originalPath;
    double alpha = 0.05;
    std::string preprocess = "none";   // none, clahe, wiener (extract mode)
    // 退化参数
    double beta = 0.3;
    double noiseStd = 3.0;
    double motionAngle = 30.0;
    int motionLength = 15;
    double motionNoiseStd = 2.0;
    double wienerK = 0.01;
    double clipLimit = 2.0;
    int tileSize = 8;
    int psfSize = 31;            // 用于维纳滤波时 PSF 尺寸
};

class CLI {
public:
    static AppConfig parse(int argc, char* argv[]);
};
