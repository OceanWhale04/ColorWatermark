// 表 6-2 单元测试：UT-DWT-01 / UT-SVD-01 / UT-CLAHE-01 / UT-PSF-01（无第三方框架，仅 assert）

#include "DWT_utils.h"
#include "Preprocessor.h"
#include "Utils.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// PSNR for single-channel float in [0,1] (Utils::computePSNR assumes 8-bit BGR).
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

static void ut_dwt_perfect_reconstruction() {
    Mat src(512, 512, CV_32F);
    randu(src, Scalar(0), Scalar(1));
    Mat LL, LH, HL, HH;
    dwt2(src, LL, LH, HL, HH);
    Mat rec;
    idwt2(rec, LL, LH, HL, HH);
    double psnr = psnrFloat01(src, rec);
    assert(psnr > 80.0);
    cout << "UT-DWT-01 PASS  PSNR(recon)=" << psnr << " dB\n";
}

static void ut_svd_reconstruction() {
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
    assert(maxErr < 1e-10);
    cout << "UT-SVD-01 PASS  maxAbsErr=" << maxErr << "\n";
}

static void ut_clahe_range() {
    Mat color(128, 128, CV_8UC3);
    randu(color, Scalar(0, 0, 0), Scalar(80, 80, 80));
    Mat low = simulateLowLight(color, 0.3, 3.0);
    Mat out = Preprocessor::clahe(low, 2.0, 8);
    double mn, mx;
    minMaxLoc(out, &mn, &mx);
    assert(mn >= 0 && mx <= 255);
    cout << "UT-CLAHE-01 PASS  pixel range [" << mn << ", " << mx << "]\n";
}

static void ut_psf_sum() {
    Mat psf = Preprocessor::generateMotionPSF(31, 30.0, 15);
    double s = sum(psf)[0];
    assert(abs(s - 1.0) < 1e-5);
    cout << "UT-PSF-01 PASS  sum(psf)=" << s << "\n";
}

int main() {
    try {
        ut_dwt_perfect_reconstruction();
        ut_svd_reconstruction();
        ut_clahe_range();
        ut_psf_sum();
    } catch (const cv::Exception& e) {
        cerr << "OpenCV exception: " << e.what() << endl;
        return 1;
    }
    cout << "All unit tests passed.\n";
    return 0;
}
