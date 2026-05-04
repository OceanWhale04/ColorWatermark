#include <opencv2/opencv.hpp>
#include <iostream>
#include <direct.h>
#include "WatermarkCodec.h"
#include "Preprocessor.h"
#include "Utils.h"

using namespace cv;
using namespace std;

static Mat loadWatermark32(const string& path) {
    Mat wm = imread(path, IMREAD_GRAYSCALE);
    if (wm.empty()) return wm;
    resize(wm, wm, Size(32, 32), 0, 0, INTER_NEAREST);
    threshold(wm, wm, 128, 255, THRESH_BINARY);
    return wm;
}

static Mat scaleToSquareGray(const Mat& src, int size) {
    Mat out;
    resize(src, out, Size(size, size), 0, 0, INTER_NEAREST);
    cvtColor(out, out, COLOR_GRAY2BGR);
    return out;
}

static void ensureOutputDir(const string& dir) {
    string normalized = dir;
    for (size_t i = 0; i < normalized.size(); ++i) {
        if (normalized[i] == '/') normalized[i] = '\\';
    }
    string current;
    for (size_t i = 0; i < normalized.size(); ++i) {
        current.push_back(normalized[i]);
        if (normalized[i] == '\\' || i + 1 == normalized.size()) {
            if (!current.empty() && current.back() == '\\') continue;
            _mkdir(current.c_str());
        }
    }
}

int main(int argc, char* argv[]) {
    string coverPath = (argc > 1) ? argv[1] : "test_images/color/512/4.2.07.tiff";
    string wmPath = (argc > 2) ? argv[2] : "test_images/XM_32x32.bmp";
    double alpha = (argc > 3) ? stod(argv[3]) : 0.05;
    string outDir = (argc > 4) ? argv[4] : "demos/paper_figures/output/fig4_4";

    Mat cover = imread(coverPath, IMREAD_COLOR);
    Mat wm = loadWatermark32(wmPath);
    if (cover.empty() || wm.empty()) {
        cerr << "Failed to load cover or watermark image." << endl;
        return -1;
    }

    Mat watermarked = WatermarkCodec::embed(cover, wm, alpha);
    Mat blurred = simulateMotionBlur(watermarked, 30.0, 15, 2.0);

    Mat extNoPre = WatermarkCodec::extract(blurred, cover, alpha);
    double accNoPre = computeAccuracy(extNoPre, wm);
    Mat psf = Preprocessor::generateMotionPSF(31, 30.0, 15);

    vector<double> kCandidates{ 0.001, 0.003, 0.005, 0.01, 0.02, 0.03, 0.05 };
    Mat restoredBest, extWienerBest;
    double bestAcc = -1.0;
    double bestK = 0.01;

    for (double k : kCandidates) {
        Mat restored = Preprocessor::wiener(blurred, psf, k);
        Mat coverWiener = Preprocessor::wiener(cover, psf, k);
        Mat extracted = WatermarkCodec::extract(restored, coverWiener, alpha);
        double acc = computeAccuracy(extracted, wm);
        if (acc > bestAcc) {
            bestAcc = acc;
            bestK = k;
            restoredBest = restored;
            extWienerBest = extracted;
        }
    }

    Mat panel1 = blurred.clone();
    Mat panel2 = scaleToSquareGray(extNoPre, panel1.rows);
    Mat panel3 = restoredBest.clone();
    Mat panel4 = scaleToSquareGray(extWienerBest, panel1.rows);

    Mat comparison;
    hconcat(vector<Mat>{panel1, panel2, panel3, panel4}, comparison);

    ensureOutputDir(outDir);
    imwrite(outDir + "/degraded_motion_blur.png", blurred);
    imwrite(outDir + "/restored_wiener_best.png", restoredBest);
    imwrite(outDir + "/extracted_without_preprocess.png", extNoPre);
    imwrite(outDir + "/extracted_with_wiener_best.png", extWienerBest);
    imwrite(outDir + "/fig4_4_motion_wiener_ablation.png", comparison);

    cout << "Accuracy (no preprocess) = " << accNoPre << " %" << endl;
    cout << "Accuracy (Wiener best) = " << bestAcc << " %" << endl;
    cout << "Best Wiener param: K=" << bestK << endl;
    cout << "Saved to: " << outDir << endl;
    return 0;
}
