// 实验五：运动模糊 L=15、30°，无预处理 vs 维纳（K=0.01，仅复原待检测图；载体为原始图）
// PSF 与 Utils::simulateMotionBlur 一致：长度 L → 核尺寸 2L+1

#include "ch6_dataset.h"
#include "ch6_run_helpers.h"
#include "exp_common.h"
#include "Preprocessor.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <iostream>

using namespace cv;
using namespace std;

static constexpr double kAlpha = 0.05;
static constexpr double kAngle = 30.0;
static constexpr int kLength = 15;
static constexpr double kMotNoise = 2.0;
static constexpr double kWienerK = 0.01;

int main(int argc, char* argv[]) {
    string listCsv = (argc > 1) ? argv[1] : "experiments/ch6_images.csv";
    string wmPath = (argc > 2) ? argv[2] : "test_images/XM_32x32.bmp";
    string outCsv = (argc > 3) ? argv[3] : "experiments/output/ch6_exp5_motion_wiener.csv";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) return -1;

    const int psfSize = kLength * 2 + 1;
    Mat psf = Preprocessor::generateMotionPSF(psfSize, kAngle, kLength);

    expEnsureDir("experiments/output");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "image_name,accuracy_no_preprocess,accuracy_wiener,delta_pp\n";

    double sum0 = 0, sum1 = 0;
    int n = 0;

    for (const auto& it : images) {
        Mat cover = ch5LoadCover512(it.second);
        if (cover.empty()) continue;

        Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
        Mat blurred = simulateMotionBlur(marked, kAngle, kLength, kMotNoise);

        Mat ext0 = WatermarkCodec::extract(blurred, cover, kAlpha);
        Mat restored = Preprocessor::wiener(blurred, psf, kWienerK);
        Mat ext1 = WatermarkCodec::extract(restored, cover, kAlpha);

        double a0 = computeAccuracy(ext0, wm);
        double a1 = computeAccuracy(ext1, wm);
        csv << it.first << "," << a0 << "," << a1 << "," << (a1 - a0) << "\n";
        sum0 += a0;
        sum1 += a1;
        n++;
        cout << it.first << " no=" << a0 << "% wiener=" << a1 << "%\n";
    }

    if (n > 0) {
        double m0 = sum0 / n, m1 = sum1 / n;
        csv << "MEAN," << m0 << "," << m1 << "," << (m1 - m0) << "\n";
        cout << "MEAN no=" << m0 << "% wiener=" << m1 << "% delta=" << (m1 - m0) << " pp\n";
    }
    cout << "Wrote " << outCsv << endl;
    return 0;
}
