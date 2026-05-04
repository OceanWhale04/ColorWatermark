#include <opencv2/opencv.hpp>
#include <iostream>
#include <direct.h>
#include "WatermarkCodec.h"
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
    string outDir = (argc > 4) ? argv[4] : "demos/paper_figures/output/fig4_2";

    Mat cover = imread(coverPath, IMREAD_COLOR);
    Mat wm = loadWatermark32(wmPath);
    if (cover.empty() || wm.empty()) {
        cerr << "Failed to load cover or watermark image." << endl;
        return -1;
    }

    Mat watermarked = WatermarkCodec::embed(cover, wm, alpha);
    Mat extracted = WatermarkCodec::extract(watermarked, cover, alpha);

    Mat wmVis, extVis;
    resize(wm, wmVis, Size(256, 256), 0, 0, INTER_NEAREST);
    resize(extracted, extVis, Size(256, 256), 0, 0, INTER_NEAREST);
    cvtColor(wmVis, wmVis, COLOR_GRAY2BGR);
    cvtColor(extVis, extVis, COLOR_GRAY2BGR);

    Mat comparison;
    hconcat(vector<Mat>{wmVis, extVis}, comparison);

    ensureOutputDir(outDir);
    imwrite(outDir + "/watermarked_clean.png", watermarked);
    imwrite(outDir + "/watermark_extracted_clean.png", extracted);
    imwrite(outDir + "/fig4_2_extract_clean.png", comparison);

    cout << "Accuracy = " << computeAccuracy(extracted, wm) << " %" << endl;
    cout << "Saved to: " << outDir << endl;
    return 0;
}
