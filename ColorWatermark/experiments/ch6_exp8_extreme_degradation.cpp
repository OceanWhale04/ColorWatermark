// 第六章表 6-13：极端退化 — 低光照 beta=0.05；运动模糊 L=25（域对齐 CLAHE / 维纳）

#include "ch6_dataset.h"
#include "ch6_run_helpers.h"
#include "exp_common.h"
#include "Preprocessor.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

static constexpr double kAlpha = 0.05;
static constexpr double kLowNoise = 3.0;
static constexpr double kClip = 2.0;
static constexpr int kTile = 8;
static constexpr double kAngle = 30.0;
static constexpr int kBlurLen = 25;
static constexpr double kMotNoise = 2.0;
static constexpr double kWienerK = 0.01;

int main(int argc, char* argv[]) {
    vector<string> pos;
    for (int i = 1; i < argc; ++i) pos.push_back(argv[i]);
    string listCsv = pos.size() >= 1 ? pos[0] : "experiments/ch6_images.csv";
    string wmPath = pos.size() >= 2 ? pos[1] : "test_images/XM_32x32.bmp";
    string outCsv = pos.size() >= 3 ? pos[2] : "experiments/output/ch6_table6_13_extreme.csv";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) return -1;

    expEnsureDir("experiments/output");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "scenario,param,mean_accuracy_no_preprocess,mean_accuracy_with_preprocess\n";

    {
        const double beta = 0.05;
        double s0 = 0, s1 = 0;
        int n = 0;
        for (const auto& it : images) {
            Mat cover = ch5LoadCover512(it.second);
            if (cover.empty()) continue;
            Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
            Mat low = simulateLowLight(marked, beta, kLowNoise);
            Mat ext0 = WatermarkCodec::extract(low, cover, kAlpha);
            Mat en = Preprocessor::clahe(low, kClip, kTile);
            Mat cref = Preprocessor::clahe(cover, kClip, kTile);
            Mat ext1 = WatermarkCodec::extract(en, cref, kAlpha);
            s0 += computeAccuracy(ext0, wm);
            s1 += computeAccuracy(ext1, wm);
            n++;
        }
        double m0 = n ? s0 / n : 0, m1 = n ? s1 / n : 0;
        csv << "lowlight_extreme,beta=0.05," << m0 << "," << m1 << "\n";
        cout << "lowlight beta=0.05 no=" << m0 << "% clahe=" << m1 << "%\n";
    }

    {
        int psfSize = kBlurLen * 2 + 1;
        Mat psf = Preprocessor::generateMotionPSF(psfSize, kAngle, kBlurLen);
        double s0 = 0, s1 = 0;
        int n = 0;
        for (const auto& it : images) {
            Mat cover = ch5LoadCover512(it.second);
            if (cover.empty()) continue;
            Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
            Mat blurred = simulateMotionBlur(marked, kAngle, kBlurLen, kMotNoise);
            Mat ext0 = WatermarkCodec::extract(blurred, cover, kAlpha);
            Mat rest = Preprocessor::wiener(blurred, psf, kWienerK);
            Mat cref = Preprocessor::wiener(cover, psf, kWienerK);
            Mat ext1 = WatermarkCodec::extract(rest, cref, kAlpha);
            s0 += computeAccuracy(ext0, wm);
            s1 += computeAccuracy(ext1, wm);
            n++;
        }
        double m0 = n ? s0 / n : 0, m1 = n ? s1 / n : 0;
        csv << "motion_blur_extreme,L=25," << m0 << "," << m1 << "\n";
        cout << "motion L=25 no=" << m0 << "% wiener=" << m1 << "%\n";
    }

    cout << "Wrote " << outCsv << endl;
    return 0;
}
