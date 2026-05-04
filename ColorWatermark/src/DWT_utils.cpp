#include "DWT_utils.h"

using namespace cv;
using namespace std;

void dwt2(const Mat& src, Mat& LL, Mat& LH, Mat& HL, Mat& HH)
{
    CV_Assert(src.type() == CV_32F);
    CV_Assert((src.rows % 2) == 0 && (src.cols % 2) == 0);

    const int rows2 = src.rows / 2;
    const int cols2 = src.cols / 2;
    const float invSqrt2 = 0.70710678118f;

    Mat lowRows(src.rows, cols2, CV_32F);
    Mat highRows(src.rows, cols2, CV_32F);

    // 1D Haar along columns.
    for (int r = 0; r < src.rows; ++r) {
        const float* sp = src.ptr<float>(r);
        float* lp = lowRows.ptr<float>(r);
        float* hp = highRows.ptr<float>(r);
        for (int c = 0; c < cols2; ++c) {
            const float a = sp[2 * c];
            const float b = sp[2 * c + 1];
            lp[c] = (a + b) * invSqrt2;
            hp[c] = (a - b) * invSqrt2;
        }
    }

    LL.create(rows2, cols2, CV_32F);
    LH.create(rows2, cols2, CV_32F);
    HL.create(rows2, cols2, CV_32F);
    HH.create(rows2, cols2, CV_32F);

    // 1D Haar along rows.
    for (int r = 0; r < rows2; ++r) {
        const float* l0 = lowRows.ptr<float>(2 * r);
        const float* l1 = lowRows.ptr<float>(2 * r + 1);
        const float* h0 = highRows.ptr<float>(2 * r);
        const float* h1 = highRows.ptr<float>(2 * r + 1);

        float* llp = LL.ptr<float>(r);
        float* lhp = LH.ptr<float>(r);
        float* hlp = HL.ptr<float>(r);
        float* hhp = HH.ptr<float>(r);

        for (int c = 0; c < cols2; ++c) {
            llp[c] = (l0[c] + l1[c]) * invSqrt2;
            lhp[c] = (l0[c] - l1[c]) * invSqrt2;
            hlp[c] = (h0[c] + h1[c]) * invSqrt2;
            hhp[c] = (h0[c] - h1[c]) * invSqrt2;
        }
    }
}

void idwt2(Mat& dst, const Mat& LL, const Mat& LH, const Mat& HL, const Mat& HH)
{
    CV_Assert(LL.type() == CV_32F);
    CV_Assert(LH.size() == LL.size() && HL.size() == LL.size() && HH.size() == LL.size());

    const int rows = LL.rows * 2;
    const int cols = LL.cols * 2;
    const float invSqrt2 = 0.70710678118f;

    Mat lowRows(rows, LL.cols, CV_32F);
    Mat highRows(rows, LL.cols, CV_32F);

    // Inverse 1D Haar along rows.
    for (int r = 0; r < LL.rows; ++r) {
        const float* llp = LL.ptr<float>(r);
        const float* lhp = LH.ptr<float>(r);
        const float* hlp = HL.ptr<float>(r);
        const float* hhp = HH.ptr<float>(r);

        float* l0 = lowRows.ptr<float>(2 * r);
        float* l1 = lowRows.ptr<float>(2 * r + 1);
        float* h0 = highRows.ptr<float>(2 * r);
        float* h1 = highRows.ptr<float>(2 * r + 1);

        for (int c = 0; c < LL.cols; ++c) {
            const float lowA = llp[c];
            const float lowD = lhp[c];
            const float highA = hlp[c];
            const float highD = hhp[c];

            l0[c] = (lowA + lowD) * invSqrt2;
            l1[c] = (lowA - lowD) * invSqrt2;
            h0[c] = (highA + highD) * invSqrt2;
            h1[c] = (highA - highD) * invSqrt2;
        }
    }

    dst.create(rows, cols, CV_32F);

    // Inverse 1D Haar along columns.
    for (int r = 0; r < rows; ++r) {
        const float* lp = lowRows.ptr<float>(r);
        const float* hp = highRows.ptr<float>(r);
        float* dp = dst.ptr<float>(r);
        for (int c = 0; c < LL.cols; ++c) {
            const float a = lp[c];
            const float d = hp[c];
            dp[2 * c] = (a + d) * invSqrt2;
            dp[2 * c + 1] = (a - d) * invSqrt2;
        }
    }
}