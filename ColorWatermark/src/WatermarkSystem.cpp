#include "WatermarkSystem.h"
#include "WatermarkCodec.h"
#include "Preprocessor.h"
#include "Utils.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int WatermarkSystem::run(const AppConfig& cfg) {
    if (cfg.mode == "embed") {
        Mat cover = imread(cfg.inputPath, IMREAD_COLOR);
        Mat wm = imread(cfg.watermarkPath, IMREAD_GRAYSCALE);
        if (cover.empty() || wm.empty()) {
            cerr << "Failed to load cover or watermark image." << endl;
            return -1;
        }
        resize(wm, wm, Size(32, 32));
        threshold(wm, wm, 128, 255, THRESH_BINARY);
        Mat watermarked = WatermarkCodec::embed(cover, wm, cfg.alpha);
        imwrite(cfg.outputPath, watermarked);
        cout << "PSNR = " << computePSNR(cover, watermarked)
            << " dB, SSIM = " << computeSSIM(cover, watermarked) << endl;
    }
    else if (cfg.mode == "extract") {
        Mat degraded = imread(cfg.inputPath, IMREAD_COLOR);
        Mat original = imread(cfg.originalPath, IMREAD_COLOR);
        Mat origWM = imread(cfg.watermarkPath, IMREAD_GRAYSCALE);
        if (degraded.empty() || original.empty() || origWM.empty()) {
            cerr << "Failed to load images." << endl;
            return -1;
        }
        resize(origWM, origWM, Size(32, 32));
        threshold(origWM, origWM, 128, 255, THRESH_BINARY);

        Mat processedDet = degraded;
        Mat processedOrig = original;
        if (cfg.preprocess == "clahe") {
            processedDet = Preprocessor::clahe(degraded, cfg.clipLimit, cfg.tileSize);
            processedOrig = Preprocessor::clahe(original, cfg.clipLimit, cfg.tileSize);
            cout << "Applied CLAHE preprocessing to both detected and reference images." << endl;
        }
        else if (cfg.preprocess == "wiener") {
            Mat psf = Preprocessor::generateMotionPSF(cfg.psfSize, cfg.motionAngle, cfg.motionLength);
            processedDet = Preprocessor::wiener(degraded, psf, cfg.wienerK);
            processedOrig = Preprocessor::wiener(original, psf, cfg.wienerK);
            cout << "Applied Wiener preprocessing to both detected and reference images." << endl;
        }
        Mat extracted = WatermarkCodec::extract(processedDet, processedOrig, cfg.alpha);
        imwrite(cfg.outputPath, extracted);
        cout << "Watermark accuracy = " << computeAccuracy(extracted, origWM) << " %" << endl;
    }
    else if (cfg.mode == "compare_clahe") {
        Mat original = imread(cfg.inputPath, IMREAD_COLOR);
        if (original.empty()) { cerr << "Cannot load input image." << endl; return -1; }
        Mat lowLight = simulateLowLight(original, cfg.beta, cfg.noiseStd);
        Mat enhanced = Preprocessor::clahe(lowLight, cfg.clipLimit, cfg.tileSize);
        Mat comparison = horizontalConcat(original, lowLight, enhanced);
        imwrite(cfg.outputPath, comparison);
        cout << "CLAHE comparison saved to " << cfg.outputPath << endl;
    }
    else if (cfg.mode == "compare_wiener") {
        Mat original = imread(cfg.inputPath, IMREAD_COLOR);
        if (original.empty()) { cerr << "Cannot load input image." << endl; return -1; }
        Mat blurred = simulateMotionBlur(original, cfg.motionAngle, cfg.motionLength, cfg.motionNoiseStd);
        int psfSize = cfg.motionLength * 2 + 1;
        Mat psf = Preprocessor::generateMotionPSF(psfSize, cfg.motionAngle, cfg.motionLength);
        Mat restored = Preprocessor::wiener(blurred, psf, cfg.wienerK);
        Mat comparison = horizontalConcat(original, blurred, restored);
        imwrite(cfg.outputPath, comparison);
        cout << "Wiener comparison saved to " << cfg.outputPath << endl;
    }
    else {
        cerr << "Unknown mode: " << cfg.mode << endl;
        return -1;
    }
    return 0;
}