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
    string outDir = (argc > 4) ? argv[4] : "demos/paper_figures/output/fig4_1";

    Mat cover = imread(coverPath, IMREAD_COLOR);
    Mat wm = loadWatermark32(wmPath);
    if (cover.empty() || wm.empty()) {
        cerr << "Failed to load cover or watermark image." << endl;
        return -1;
    }

    Mat watermarked = WatermarkCodec::embed(cover, wm, alpha);

    Mat wmVis;
    resize(wm, wmVis, cover.size(), 0, 0, INTER_NEAREST);
    cvtColor(wmVis, wmVis, COLOR_GRAY2BGR);

    Mat comparison;
    hconcat(vector<Mat>{cover, wmVis, watermarked}, comparison);

    ensureOutputDir(outDir);
    imwrite(outDir + "/cover.png", cover);
    imwrite(outDir + "/watermark_binary_32x32.png", wm);
    imwrite(outDir + "/watermarked.png", watermarked);
    imwrite(outDir + "/fig4_1_embed_visual.png", comparison);

    cout << "PSNR = " << computePSNR(cover, watermarked) << " dB" << endl;
    cout << "SSIM = " << computeSSIM(cover, watermarked) << endl;
    cout << "Saved to: " << outDir << endl;
    return 0;
}
