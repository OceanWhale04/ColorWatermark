// 表 6-2 集成测试 IT-EM-01：10 幅图无退化 embed->extract，准确率应为 100%

#include "ch6_dataset.h"
#include "ch6_run_helpers.h"
#include "exp_common.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
    string listCsv = (argc > 1) ? argv[1] : "experiments/ch6_images.csv";
    string wmPath = (argc > 2) ? argv[2] : "test_images/XM_32x32.bmp";

    vector<pair<string, string>> images;
    string err;
    if (!ch5LoadImageList(listCsv, images, err)) {
        cerr << err << endl;
        return 1;
    }
    Mat wm = expLoadWatermark32(wmPath);
    if (wm.empty()) return 1;

    const double alpha = 0.05;
    for (const auto& it : images) {
        Mat cover = ch5LoadCover512(it.second);
        if (cover.empty()) continue;
        Mat marked = WatermarkCodec::embed(cover, wm, alpha);
        Mat ext = WatermarkCodec::extract(marked, cover, alpha);
        double acc = computeAccuracy(ext, wm);
        if (acc < 100.0 - 1e-6) {
            cerr << "FAIL " << it.first << " accuracy=" << acc << "%\n";
            return 2;
        }
        cout << "IT-EM-01 " << it.first << " " << acc << "%\n";
    }
    cout << "Integration test IT-EM-01 PASS.\n";
    return 0;
}
