// 第六章表 6-12：运动模糊 L=15、30° 下退化噪声 sigma 扫描；无预处理 vs 维纳 K=0.01（域对齐）
// simulateMotionBlur 第四参数为卷积后高斯噪声标准差

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
static constexpr double kAngle = 30.0;
static constexpr int kLength = 15;
static constexpr double kWienerK = 0.01;

int main(int argc, char* argv[]) {
    vector<string> pos;
    for (int i = 1; i < argc; ++i)
        pos.push_back(argv[i]);
    string listCsv = pos.size() >= 1 ? pos[0] : "experiments/ch6_images.csv";
    string wmPath = pos.size() >= 2 ? pos[1] : "test_images/XM_32x32.bmp";
    string outCsv = pos.size() >= 3 ? pos[2] : "experiments/output/ch6_exp7_noise.csv";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) return -1;

    int psfSize = kLength * 2 + 1;
    Mat psf = Preprocessor::generateMotionPSF(psfSize, kAngle, kLength);

    vector<double> sigmas{ 1.0, 2.0, 3.0, 4.0, 5.0 };

    expEnsureDir("experiments/output");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "motion_noise_sigma,mean_accuracy_no_preprocess,mean_accuracy_wiener\n";

    for (double sigma : sigmas) {
        double sum0 = 0, sum1 = 0;
        int n = 0;
        for (const auto& it : images) {
            Mat cover = ch5LoadCover512(it.second);
            if (cover.empty()) continue;
            Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
            Mat blurred = simulateMotionBlur(marked, kAngle, kLength, sigma);
            Mat ext0 = WatermarkCodec::extract(blurred, cover, kAlpha);
            Mat rest = Preprocessor::wiener(blurred, psf, kWienerK);
            Mat cref = Preprocessor::wiener(cover, psf, kWienerK);
            Mat ext1 = WatermarkCodec::extract(rest, cref, kAlpha);
            sum0 += computeAccuracy(ext0, wm);
            sum1 += computeAccuracy(ext1, wm);
            n++;
        }
        double m0 = n ? sum0 / n : 0, m1 = n ? sum1 / n : 0;
        csv << sigma << "," << m0 << "," << m1 << "\n";
        cout << "sigma=" << sigma << " no_preprocess=" << m0 << "% wiener=" << m1 << "%\n";
    }
    cout << "Wrote " << outCsv << endl;
    return 0;
}
