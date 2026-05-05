// 第六章表 6-9：CLAHE clipLimit 敏感度（固定 beta=0.3、tileSize=8，域对齐提取）

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
static constexpr double kBeta = 0.3;
static constexpr double kNoiseStd = 3.0;
static constexpr int kTile = 8;

int main(int argc, char* argv[]) {
    vector<string> pos;
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--word-protocol") continue;
        pos.push_back(argv[i]);
    }
    string listCsv = pos.size() >= 1 ? pos[0] : "experiments/ch6_images.csv";
    string wmPath = pos.size() >= 2 ? pos[1] : "test_images/XM_32x32.bmp";
    string outCsv = pos.size() >= 3 ? pos[2] : "output/experiments/ch6_exp4_cliplimit.csv";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) return -1;

    vector<double> clips{ 1.0, 2.0, 2.5, 3.0, 4.0 };

    expEnsureDir("experiments/output");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "clipLimit,tileSize,beta,mean_accuracy_percent\n";

    for (double clip : clips) {
        double sum = 0;
        int n = 0;
        for (const auto& it : images) {
            Mat cover = ch5LoadCover512(it.second);
            if (cover.empty()) continue;
            Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
            Mat low = simulateLowLight(marked, kBeta, kNoiseStd);
            Mat en = Preprocessor::clahe(low, clip, kTile);
            Mat cref = Preprocessor::clahe(cover, clip, kTile);
            Mat ext = WatermarkCodec::extract(en, cref, kAlpha);
            sum += computeAccuracy(ext, wm);
            n++;
        }
        double mean = n ? sum / n : 0;
        csv << clip << "," << kTile << "," << kBeta << "," << mean << "\n";
        cout << "clipLimit=" << clip << " mean accuracy=" << mean << "%\n";
    }
    cout << "Wrote " << outCsv << endl;
    return 0;
}
