#include "CLI.h"
#include <iostream>

using namespace std;
using namespace cv;

AppConfig CLI::parse(int argc, char* argv[]) {
    AppConfig cfg;
    // 简洁的命令行解析（无需第三方库）
    for (int i = 1; i < argc; i++) {
        string arg(argv[i]);
        if (arg == "--mode" && i + 1 < argc) cfg.mode = argv[++i];
        else if (arg == "--input" && i + 1 < argc) cfg.inputPath = argv[++i];
        else if (arg == "--output" && i + 1 < argc) cfg.outputPath = argv[++i];
        else if (arg == "--watermark" && i + 1 < argc) cfg.watermarkPath = argv[++i];
        else if (arg == "--original" && i + 1 < argc) cfg.originalPath = argv[++i];
        else if (arg == "--alpha" && i + 1 < argc) cfg.alpha = stod(argv[++i]);
        else if (arg == "--preprocess" && i + 1 < argc) cfg.preprocess = argv[++i];
        else if (arg == "--beta" && i + 1 < argc) cfg.beta = stod(argv[++i]);
        else if (arg == "--noiseStd" && i + 1 < argc) cfg.noiseStd = stod(argv[++i]);
        else if (arg == "--motionAngle" && i + 1 < argc) cfg.motionAngle = stod(argv[++i]);
        else if (arg == "--motionLength" && i + 1 < argc) cfg.motionLength = stoi(argv[++i]);
        else if (arg == "--motionNoiseStd" && i + 1 < argc) cfg.motionNoiseStd = stod(argv[++i]);
        else if (arg == "--wienerK" && i + 1 < argc) cfg.wienerK = stod(argv[++i]);
        else if (arg == "--clipLimit" && i + 1 < argc) cfg.clipLimit = stod(argv[++i]);
        else if (arg == "--tileSize" && i + 1 < argc) cfg.tileSize = stoi(argv[++i]);
        else if (arg == "--psfSize" && i + 1 < argc) cfg.psfSize = stoi(argv[++i]);
    }
    return cfg;
}