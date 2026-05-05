// 实验一：常规场景 — 10 幅载体，α=0.05，PSNR / SSIM / 解码准确率

#include "ch6_dataset.h"
#include "ch6_run_helpers.h"
#include "exp_common.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <cmath>
#include <iostream>

using namespace cv;
using namespace std;

static constexpr double kAlpha = 0.05;

int main(int argc, char* argv[]) {
    string listCsv = (argc > 1) ? argv[1] : "experiments/ch6_images.csv";
    string wmPath = (argc > 2) ? argv[2] : "test_images/XM_32x32.bmp";
    string outCsv = (argc > 3) ? argv[3] : "experiments/output/ch6_exp1_baseline.csv";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return -1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) {
        cerr << "Bad watermark: " << wmPath << endl;
        return -1;
    }

    expEnsureDir("experiments/output");
    ofstream csv;
    expCsvOpen(csv, outCsv);
    csv << "image_name,psnr_db,ssim,accuracy_percent\n";

    double sumPsnr = 0, sumSsim = 0, sumAcc = 0;
    int n = 0;

    for (const auto& it : images) {
        Mat cover = ch5LoadCover512(it.second);
        if (cover.empty()) {
            cerr << "Skip (load fail): " << it.first << " -> " << it.second << endl;
            continue;
        }
        Mat marked = WatermarkCodec::embed(cover, wm, kAlpha);
        double psnr = computePSNR(cover, marked);
        double ssim = computeSSIM(cover, marked);
        Mat ext = WatermarkCodec::extract(marked, cover, kAlpha);
        double acc = computeAccuracy(ext, wm);

        csv << it.first << "," << psnr << "," << ssim << "," << acc << "\n";
        sumPsnr += psnr;
        sumSsim += ssim;
        sumAcc += acc;
        n++;
        cout << it.first << " PSNR=" << psnr << " SSIM=" << ssim << " Acc=" << acc << "%\n";
    }

    if (n > 0) {
        csv << "MEAN," << (sumPsnr / n) << "," << (sumSsim / n) << "," << (sumAcc / n) << "\n";
        cout << "MEAN PSNR=" << (sumPsnr / n) << " SSIM=" << (sumSsim / n) << " Acc=" << (sumAcc / n) << "%\n";
    }
    cout << "Wrote " << outCsv << endl;
    return 0;
}
