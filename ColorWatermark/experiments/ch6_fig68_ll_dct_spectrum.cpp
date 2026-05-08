// 图 6-8 数据与图像：与 WatermarkCodec 相同路径（一层 Haar DWT 的 LL 子带 → 8×8 分块 DCT），
// 统计各 8×8 DCT 系数位置上的平均绝对值（对 BGR 三通道、所有可整分块、ch6 图像表求平均）。
//
// 用法：ch6_fig68_ll_dct_spectrum.exe [ch6_images.csv] [输出目录]
// 默认：ch6_images.csv、experiments/output/
// 输出：ch6_fig68_dct_spectrum.csv、ch6_fig68_dct_heatmap.png、ch6_fig68_dct_zigzag.png

#include "ch6_dataset.h"
#include "exp_common.h"
#include "DWT_utils.h"
#include "Preprocessor.h"
#include "Utils.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

static const int kW = 512;
static const double kBeta = 0.3;
static const double kNoiseStd = 3.0;
static const double kClip = 2.0;
static const int kTile = 8;

// JPEG 之字形顺序，将 8×8 系数从低频到高频排成 64 点（用于折线图）
static const int kZigZag[64] = {
    0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

static void addSpectrumFromBgr8u(const Mat& bgr8u, Mat& sum8x8, int& blockCount)
{
    Mat img = bgr8u;
    if (img.rows != kW || img.cols != kW)
        resize(img, img, Size(kW, kW));

    vector<Mat> ch;
    split(img, ch);
    for (int c = 0; c < 3; ++c) {
        Mat f;
        ch[c].convertTo(f, CV_32F, 1.0 / 255.0);
        Mat LL, LH, HL, HH;
        dwt2(f, LL, LH, HL, HH);

        const int rb = LL.rows / 8;
        const int cb = LL.cols / 8;
        for (int i = 0; i < rb; ++i) {
            for (int j = 0; j < cb; ++j) {
                Rect roi(j * 8, i * 8, 8, 8);
                Mat block = LL(roi).clone();
                Mat d;
                dct(block, d);
                for (int u = 0; u < 8; ++u) {
                    for (int v = 0; v < 8; ++v)
                        sum8x8.at<double>(u, v) += std::abs(d.at<float>(u, v));
                }
                ++blockCount;
            }
        }
    }
}

static void saveCsv(const string& path, const Mat& O, const Mat& L, const Mat& C)
{
    ofstream out;
    expCsvOpen(out, path);
    out << "u,v,mean_abs_dct_original,mean_abs_dct_lowlight,mean_abs_dct_clahe\n";
    for (int u = 0; u < 8; ++u) {
        for (int v = 0; v < 8; ++v) {
            out << u << "," << v << ","
                << O.at<double>(u, v) << ","
                << L.at<double>(u, v) << ","
                << C.at<double>(u, v) << "\n";
        }
    }
}

static Mat heatmapU8(const Mat& norm01)
{
    Mat n;
    norm01.convertTo(n, CV_8U, 255.0);
    Mat color;
    applyColorMap(n, color, COLORMAP_JET);
    return color;
}

static void buildHeatmapFigure(const Mat& O, const Mat& L, const Mat& C, const string& path)
{
    double gmax = 0;
    minMaxLoc(O, nullptr, &gmax);
    double t;
    minMaxLoc(L, nullptr, &t);
    gmax = max(gmax, t);
    minMaxLoc(C, nullptr, &t);
    gmax = max(gmax, t);
    if (gmax < 1e-12)
        gmax = 1.0;

    auto normPanel = [&](const Mat& S) {
        Mat n = S / gmax;
        Mat up;
        resize(n, up, Size(192, 192), 0, 0, INTER_NEAREST);
        return heatmapU8(up);
    };

    Mat p0 = normPanel(O);
    Mat p1 = normPanel(L);
    Mat p2 = normPanel(C);

    const int titleH = 36;
    const int pad = 8;
    Mat row(max(p0.rows + titleH + pad, 1), p0.cols * 3 + pad * 4, CV_8UC3, Scalar(255, 255, 255));
    auto place = [&](const Mat& panel, int x0, const string& title) {
        putText(row, title, Point(x0 + 8, 22), FONT_HERSHEY_SIMPLEX, 0.55, Scalar(40, 40, 40), 1, LINE_AA);
        panel.copyTo(row(Rect(x0, titleH, panel.cols, panel.rows)));
    };
    place(p0, pad, "original");
    place(p1, pad * 2 + p0.cols, "low-light b=0.3");
    place(p2, pad * 3 + p0.cols * 2, "CLAHE on low");

    imwrite(path, row);
}

static void buildZigzagFigure(const Mat& O, const Mat& L, const Mat& C, const string& path)
{
    int W = 900;
    int H = 420;
    Mat canvas(H, W, CV_8UC3, Scalar(255, 255, 255));
    vector<Point> polyO, polyL, polyC;
    double ymax = 0;
    for (int k = 0; k < 64; ++k) {
        int idx = kZigZag[k];
        int u = idx / 8;
        int v = idx % 8;
        ymax = max(ymax, O.at<double>(u, v));
        ymax = max(ymax, L.at<double>(u, v));
        ymax = max(ymax, C.at<double>(u, v));
    }
    if (ymax < 1e-12)
        ymax = 1.0;

    int marginL = 60;
    int marginR = 40;
    int marginT = 40;
    int marginB = 50;
    int plotW = W - marginL - marginR;
    int plotH = H - marginT - marginB;

    for (int k = 0; k < 64; ++k) {
        int idx = kZigZag[k];
        int u = idx / 8;
        int v = idx % 8;
        double x = marginL + (plotW * k) / 63.0;
        auto ypix = [&](double val) {
            return marginT + static_cast<int>(plotH * (1.0 - val / ymax));
        };
        polyO.push_back(Point(cvRound(x), ypix(O.at<double>(u, v))));
        polyL.push_back(Point(cvRound(x), ypix(L.at<double>(u, v))));
        polyC.push_back(Point(cvRound(x), ypix(C.at<double>(u, v))));
    }

    rectangle(canvas, Rect(marginL, marginT, plotW, plotH), Scalar(220, 220, 220), 1);
    polylines(canvas, polyO, false, Scalar(60, 60, 200), 2, LINE_AA);
    polylines(canvas, polyL, false, Scalar(200, 120, 60), 2, LINE_AA);
    polylines(canvas, polyC, false, Scalar(60, 160, 80), 2, LINE_AA);

    putText(canvas, "Fig 6-8 LL 8x8 DCT mean |coeff| (zig-zag order)", Point(marginL, 28),
            FONT_HERSHEY_SIMPLEX, 0.65, Scalar(30, 30, 30), 1, LINE_AA);
    putText(canvas, "blue=original orange=low-light green=CLAHE(low)", Point(marginL, H - 18),
            FONT_HERSHEY_SIMPLEX, 0.45, Scalar(80, 80, 80), 1, LINE_AA);

    imwrite(path, canvas);
}

int main(int argc, char** argv)
{
    string listCsv = "experiments/ch6_images.csv";
    string outDir = "experiments/output";
    if (argc >= 2)
        listCsv = argv[1];
    if (argc >= 3)
        outDir = argv[2];

    vector<pair<string, string>> rows;
    string err;
    if (!ch5LoadImageList(listCsv, rows, err)) {
        cerr << err << endl;
        return 1;
    }

    expEnsureDir(outDir);

    Mat sumO = Mat::zeros(8, 8, CV_64F);
    Mat sumL = Mat::zeros(8, 8, CV_64F);
    Mat sumC = Mat::zeros(8, 8, CV_64F);
    long long totalBlocks = 0;
    int nImg = 0;

    for (const auto& pr : rows) {
        Mat cover = imread(pr.second, IMREAD_COLOR);
        if (cover.empty()) {
            cerr << "Skip (read fail): " << pr.second << endl;
            continue;
        }
        Mat low = simulateLowLight(cover, kBeta, kNoiseStd);
        Mat clahe = Preprocessor::clahe(low, kClip, kTile);

        int b0 = 0, b1 = 0, b2 = 0;
        addSpectrumFromBgr8u(cover, sumO, b0);
        addSpectrumFromBgr8u(low, sumL, b1);
        addSpectrumFromBgr8u(clahe, sumC, b2);
        if (b0 != b1 || b0 != b2) {
            cerr << "Block count mismatch for " << pr.first << ": " << b0 << " " << b1 << " " << b2 << endl;
            return 2;
        }
        totalBlocks += b0;
        ++nImg;
    }

    if (nImg == 0 || totalBlocks == 0) {
        cerr << "No images processed." << endl;
        return 1;
    }

    double denom = static_cast<double>(totalBlocks);
    Mat O = sumO / denom;
    Mat L = sumL / denom;
    Mat C = sumC / denom;

    string base = outDir + "/ch6_fig68";
#ifdef _WIN32
    for (size_t i = 0; i < base.size(); ++i)
        if (base[i] == '/')
            base[i] = '\\';
#endif

    saveCsv(base + "_dct_spectrum.csv", O, L, C);
    buildHeatmapFigure(O, L, C, base + "_dct_heatmap.png");
    buildZigzagFigure(O, L, C, base + "_dct_zigzag.png");

    double dcO = O.at<double>(0, 0);
    double dcL = L.at<double>(0, 0);
    double dcC = C.at<double>(0, 0);
    double acO = sum(O).val[0] - dcO;
    double acL = sum(L).val[0] - dcL;
    double acC = sum(C).val[0] - dcC;

    cout << "Images: " << nImg << "  total DCT blocks (sum over ch & imgs): " << totalBlocks << endl;
    cout << "Mean |DCT(0,0)| (DC):  orig=" << dcO << "  low=" << dcL << "  clahe=" << dcC << endl;
    cout << "Sum mean |AC| (rough): orig=" << acO << "  low=" << acL << "  clahe=" << acC << endl;
    cout << "Wrote:\n  " << base << "_dct_spectrum.csv\n  " << base << "_dct_heatmap.png\n  " << base
         << "_dct_zigzag.png\n";

    return 0;
}
