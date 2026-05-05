// 实验二：低光照 β=0.3，无预处理 vs CLAHE（仅处理待检测图；载体为原始图）

#include "ch6_dataset.h"
#include "ch6_run_helpers.h"
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
static constexpr double kBeta = 0.3;
static constexpr double kNoiseStd = 3.0;
static constexpr double kClip = 2.0;
static constexpr int kTile = 8;

int main(int argc, char* argv[]) {
    bool wordProtocol = false;
    for (int i = 1; i < argc; ++i) {
        string a(argv[i]);
        if (a == "--word-protocol") wordProtocol = true;
    }

    string listCsv = "experiments/ch6_images.csv";
    string wmPath = "test_images/XM_32x32.bmp";
    string outCsv = "experiments/output/ch6_exp2_lowlight_clahe.csv";
    // 参数顺序（跳过开关）：[listCsv] [wmPath] [outCsv]
    vector<string> pos;
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--word-protocol") continue;
        pos.push_back(argv[i]);
    }
    if (pos.size() >= 1) listCsv = pos[0];
    if (pos.size() >= 2) wmPath = pos[1];
    if (pos.size() >= 3) outCsv = pos[2];

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) {
        cerr << "Bad watermark\n";
        return -1;
    }

    expEnsureDir("experiments/output");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "image_name,accuracy_no_preprocess,accuracy_clahe,delta_pp,mean_abs_clahe_minus_low\n";

    double sum0 = 0, sum1 = 0;
    int n = 0;

    if (wordProtocol)
        cout << "Mode: Word manuscript — extract(CLAHE(I'), I_orig) [reference NOT enhanced]\n";
    else
        cout << "Mode: Domain-aligned (default) — extract(CLAHE(I'), CLAHE(I_orig))\n";

    for (const auto& it : images) {
        Mat cover = ch5LoadCover512(it.second);
        if (cover.empty()) continue;

        Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
        Mat low = simulateLowLight(marked, kBeta, kNoiseStd);

        Mat ext0 = WatermarkCodec::extract(low, cover, kAlpha);
        Mat enhanced = Preprocessor::clahe(low, kClip, kTile);
        Mat refClahe = wordProtocol ? cover : Preprocessor::clahe(cover, kClip, kTile);
        Mat ext1 = WatermarkCodec::extract(enhanced, refClahe, kAlpha);

        Mat diff;
        absdiff(enhanced, low, diff);
        Scalar m = mean(diff);

        double a0 = computeAccuracy(ext0, wm);
        double a1 = computeAccuracy(ext1, wm);
        csv << it.first << "," << a0 << "," << a1 << "," << (a1 - a0) << "," << m[0] << "\n";
        sum0 += a0;
        sum1 += a1;
        n++;
        cout << it.first << " no=" << a0 << "% clahe=" << a1 << "% meanAbs(clahe-low)=" << m[0] << "\n";
    }

    if (n > 0) {
        double m0 = sum0 / n, m1 = sum1 / n;
        csv << "MEAN," << m0 << "," << m1 << "," << (m1 - m0) << ",\n";
        cout << "MEAN no=" << m0 << "% clahe=" << m1 << "% delta=" << (m1 - m0) << " pp\n";
    }
    cout << "Wrote " << outCsv << endl;
    return 0;
}
