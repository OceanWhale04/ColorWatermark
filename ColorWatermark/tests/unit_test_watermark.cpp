// 表 6-2 单元测试：UT-DWT-01 / UT-SVD-01 / UT-CLAHE-01 / UT-PSF-01（无第三方框架）
// 运行结束将结果表写入 tests/output/unit_test_results.csv

#include "DWT_utils.h"
#include "Preprocessor.h"
#include "Utils.h"
#include "test_output_csv.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

static double psnrFloat01(const Mat& a, const Mat& b) {
    CV_Assert(a.size() == b.size() && a.type() == b.type() && a.channels() == 1);
    Mat d;
    absdiff(a, b, d);
    d = d.mul(d);
    double mse = mean(d)[0];
    if (mse < 1e-20)
        return 999.0;
    return 10.0 * log10(1.0 / mse);
}

static bool run_ut_dwt(vector<string>& lines) {
    Mat src(512, 512, CV_32F);
    randu(src, Scalar(0), Scalar(1));
    Mat LL, LH, HL, HH;
    dwt2(src, LL, LH, HL, HH);
    Mat rec;
    idwt2(rec, LL, LH, HL, HH);
    double psnr = psnrFloat01(src, rec);
    if (psnr <= 80.0) {
        lines.push_back("UT-DWT-01,psnr_db," + to_string(psnr) + ",0,threshold 80 dB");
        return false;
    }
    lines.push_back("UT-DWT-01,psnr_db," + to_string(psnr) + ",1,Haar round-trip");
    return true;
}

static bool run_ut_svd(vector<string>& lines) {
    Mat a(8, 8, CV_64F);
    randu(a, Scalar(-1), Scalar(1));
    Mat w, u, vt;
    SVD::compute(a, w, u, vt, SVD::FULL_UV);
    Mat sigma = Mat::diag(w);
    Mat recon = u * sigma * vt;
    double maxErr = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            maxErr = max(maxErr, abs(recon.at<double>(i, j) - a.at<double>(i, j)));
    if (maxErr >= 1e-10) {
        lines.push_back("UT-SVD-01,max_abs_error," + to_string(maxErr) + ",0,threshold 1e-10");
        return false;
    }
    lines.push_back("UT-SVD-01,max_abs_error," + to_string(maxErr) + ",1,8x8 reconstruction");
    return true;
}

static bool run_ut_clahe(vector<string>& lines) {
    Mat color(128, 128, CV_8UC3);
    randu(color, Scalar(0, 0, 0), Scalar(80, 80, 80));
    Mat low = simulateLowLight(color, 0.3, 3.0);
    Mat out = Preprocessor::clahe(low, 2.0, 8);
    double mn, mx;
    minMaxLoc(out, &mn, &mx);
    if (mn < 0 || mx > 255) {
        lines.push_back("UT-CLAHE-01,pixel_min," + to_string(mn) + ",0,");
        lines.push_back("UT-CLAHE-01,pixel_max," + to_string(mx) + ",0,out of 0..255");
        return false;
    }
    lines.push_back("UT-CLAHE-01,pixel_min," + to_string(mn) + ",1,");
    lines.push_back("UT-CLAHE-01,pixel_max," + to_string(mx) + ",1,output range");
    return true;
}

static bool run_ut_psf(vector<string>& lines) {
    Mat psf = Preprocessor::generateMotionPSF(31, 30.0, 15);
    double s = sum(psf)[0];
    if (abs(s - 1.0) >= 1e-5) {
        lines.push_back("UT-PSF-01,sum_psf," + to_string(s) + ",0,expect 1.0");
        return false;
    }
    lines.push_back("UT-PSF-01,sum_psf," + to_string(s) + ",1,normalized PSF");
    return true;
}

static void writeUnitCsv(const string& path, const vector<string>& lines) {
    ofstream csv;
    testsCsvOpen(csv, path);
    csv << "test_id,metric,value,passed,note\n";
    for (const string& ln : lines)
        csv << ln << "\n";
}

int main(int argc, char* argv[]) {
    string outCsv = "tests/output/unit_test_results.csv";
    if (argc > 1)
        outCsv = argv[1];

    vector<string> lines;
    bool all = true;
    all = run_ut_dwt(lines) && all;
    all = run_ut_svd(lines) && all;
    all = run_ut_clahe(lines) && all;
    all = run_ut_psf(lines) && all;

    writeUnitCsv(outCsv, lines);

    if (!all) {
        cerr << "One or more unit tests FAILED. See " << outCsv << endl;
        return 1;
    }
    for (const string& ln : lines)
        cout << ln << "\n";
    cout << "All unit tests passed. Wrote " << outCsv << endl;
    return 0;
}
