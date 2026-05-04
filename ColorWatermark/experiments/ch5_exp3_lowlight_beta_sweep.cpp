// 5.3.2 实验三：β ∈ {0.3, 0.2, 0.1}，每组 10 幅平均准确率 — 表 5-5

#include "ch5_dataset.h"
#include "ch5_run_helpers.h"
#include "exp_common.h"
#include "Preprocessor.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <iostream>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

static constexpr double kAlpha = 0.05;
static constexpr double kNoiseStd = 3.0;
static constexpr double kClip = 2.0;
static constexpr int kTile = 8;

int main(int argc, char* argv[]) {
    bool wordProtocol = false;
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--word-protocol") wordProtocol = true;
    }
    vector<string> pos;
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--word-protocol") continue;
        pos.push_back(argv[i]);
    }
    string listCsv = pos.size() >= 1 ? pos[0] : "experiments/ch5_images.csv";
    string wmPath = pos.size() >= 2 ? pos[1] : "test_images/XM_32x32.bmp";
    string outCsv = pos.size() >= 3 ? pos[2] : "output/experiments/ch5_exp3_lowlight_beta.csv";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) return -1;

    if (wordProtocol)
        cout << "Mode: Word — extract(CLAHE(I'), I_orig)\n";
    else
        cout << "Mode: Domain-aligned — extract(CLAHE(I'), CLAHE(I_orig))\n";

    vector<double> betas{ 0.3, 0.2, 0.1 };

    expEnsureDir("output/experiments");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "beta,mean_accuracy_no_preprocess,mean_accuracy_clahe,delta_pp\n";

    for (double beta : betas) {
        double sum0 = 0, sum1 = 0;
        int n = 0;
        for (const auto& it : images) {
            Mat cover = ch5LoadCover512(it.second);
            if (cover.empty()) continue;
            Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
            Mat low = simulateLowLight(marked, beta, kNoiseStd);
            Mat ext0 = WatermarkCodec::extract(low, cover, kAlpha);
            Mat enhanced = Preprocessor::clahe(low, kClip, kTile);
            Mat refClahe = wordProtocol ? cover : Preprocessor::clahe(cover, kClip, kTile);
            Mat ext1 = WatermarkCodec::extract(enhanced, refClahe, kAlpha);
            sum0 += computeAccuracy(ext0, wm);
            sum1 += computeAccuracy(ext1, wm);
            n++;
        }
        if (n == 0) continue;
        double m0 = sum0 / n, m1 = sum1 / n;
        csv << beta << "," << m0 << "," << m1 << "," << (m1 - m0) << "\n";
        cout << "beta=" << beta << " mean_no=" << m0 << "% mean_clahe=" << m1 << "%\n";
    }
    cout << "Wrote " << outCsv << endl;
    return 0;
}
