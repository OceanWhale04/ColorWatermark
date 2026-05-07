// 表 6-2 集成测试 IT-EM-01：10 幅图无退化 embed->extract，准确率应为 100%
// 运行结束将逐图结果写入 tests/output/integration_test_it_em01.csv

#include "ch6_dataset.h"
#include "ch6_run_helpers.h"
#include "exp_common.h"
#include "test_output_csv.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
    string listCsv = (argc > 1) ? argv[1] : "experiments/ch6_images.csv";
    string wmPath = (argc > 2) ? argv[2] : "test_images/XM_32x32.bmp";
    string outCsv = "tests/output/integration_test_it_em01.csv";
    if (argc > 3)
        outCsv = argv[3];

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return 1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty())
        return 1;

    const double alpha = 0.05;
    ofstream csv;
    testsCsvOpen(csv, outCsv);
    csv << "image_name,accuracy_percent,passed,note\n";

    double sumAcc = 0;
    int n = 0;
    bool allPass = true;

    for (const auto& it : images) {
        Mat cover = ch5LoadCover512(it.second);
        if (cover.empty())
            continue;
        Mat marked = WatermarkCodec::embed(cover, wm, alpha);
        Mat ext = WatermarkCodec::extract(marked, cover, alpha);
        double acc = computeAccuracy(ext, wm);
        int pass = (acc >= 100.0 - 1e-6) ? 1 : 0;
        if (!pass)
            allPass = false;
        csv << it.first << "," << acc << "," << pass << ",IT-EM-01 embed-extract\n";
        sumAcc += acc;
        n++;
        cout << "IT-EM-01 " << it.first << " " << acc << "% pass=" << pass << "\n";
    }

    if (n > 0) {
        double meanAcc = sumAcc / n;
        csv << "MEAN," << meanAcc << "," << (allPass ? 1 : 0) << ",over " << n << " images\n";
    }

    cout << "Wrote " << outCsv << endl;
    if (!allPass) {
        cerr << "Integration test IT-EM-01 FAILED.\n";
        return 2;
    }
    cout << "Integration test IT-EM-01 PASS.\n";
    return 0;
}
