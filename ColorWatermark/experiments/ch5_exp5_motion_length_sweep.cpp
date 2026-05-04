// 5.4.2 实验五：L ∈ {10, 15, 20}，每组 10 幅平均 — 表 5-7

#include "ch5_dataset.h"
#include "ch5_run_helpers.h"
#include "exp_common.h"
#include "Preprocessor.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

static constexpr double kAlpha = 0.05;
static constexpr double kAngle = 30.0;
static constexpr double kMotNoise = 2.0;
static constexpr double kWienerK = 0.01;

int main(int argc, char* argv[]) {
    string listCsv = (argc > 1) ? argv[1] : "experiments/ch5_images.csv";
    string wmPath = (argc > 2) ? argv[2] : "test_images/XM_32x32.bmp";
    string outCsv = (argc > 3) ? argv[3] : "experiments/output/ch5_exp5_motion_lengths.csv";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) return -1;

    vector<int> lengths{ 10, 15, 20 };

    expEnsureDir("output/experiments");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "blur_length_px,mean_accuracy_no_preprocess,mean_accuracy_wiener,delta_pp\n";

    for (int L : lengths) {
        int psfSize = L * 2 + 1;
        Mat psf = Preprocessor::generateMotionPSF(psfSize, kAngle, L);
        double sum0 = 0, sum1 = 0;
        int n = 0;
        for (const auto& it : images) {
            Mat cover = ch5LoadCover512(it.second);
            if (cover.empty()) continue;
            Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
            Mat blurred = simulateMotionBlur(marked, kAngle, L, kMotNoise);
            Mat ext0 = WatermarkCodec::extract(blurred, cover, kAlpha);
            Mat restored = Preprocessor::wiener(blurred, psf, kWienerK);
            Mat ext1 = WatermarkCodec::extract(restored, cover, kAlpha);
            sum0 += computeAccuracy(ext0, wm);
            sum1 += computeAccuracy(ext1, wm);
            n++;
        }
        if (n == 0) continue;
        double m0 = sum0 / n, m1 = sum1 / n;
        csv << L << "," << m0 << "," << m1 << "," << (m1 - m0) << "\n";
        cout << "L=" << L << " mean_no=" << m0 << "% mean_wiener=" << m1 << "%\n";
    }
    cout << "Wrote " << outCsv << endl;
    return 0;
}
