// 第六章表 6-14 / 6.3.4：关键操作平均耗时（10 幅图，与论文 NFR-1 对照）
// 用法：在 VS 中排除 main.cpp、取消本文件 ExcludedFromBuild 后生成；工作目录为 ColorWatermark 工程目录。
// 可选参数：[ch6_images.csv] [watermark.bmp] [输出.csv] ；默认 CSV 为 tests/output/perf_test_results.csv
// 可选：首参数 --warmup 5 表示每类操作先热身 5 次（不计入统计）

#include "ch6_dataset.h"
#include "ch6_run_helpers.h"
#include "exp_common.h"
#include "test_output_csv.h"
#include "Preprocessor.h"
#include "Utils.h"
#include "WatermarkCodec.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

static constexpr double kAlpha = 0.05;
static constexpr double kClip = 2.0;
static constexpr int kTile = 8;
static constexpr double kMotAngle = 30.0;
static constexpr int kMotLen = 15;
static constexpr double kMotNoise = 2.0;
static constexpr double kWienerK = 0.01;

using Clock = chrono::steady_clock;

static double elapsedMs(Clock::time_point t0) {
    return chrono::duration<double, milli>(Clock::now() - t0).count();
}

static double meanOf(const vector<double>& v) {
    if (v.empty()) return 0;
    return accumulate(v.begin(), v.end(), 0.0) / (double)v.size();
}

static void warmupDiscard(const string& path, int n) {
    for (int i = 0; i < n; ++i) {
        Mat m = imread(path, IMREAD_COLOR);
        (void)m;
    }
}

int main(int argc, char* argv[]) {
    int warmup = 0;
    vector<string> pos;
    for (int i = 1; i < argc; ++i) {
        string a(argv[i]);
        if (a == "--warmup" && i + 1 < argc) {
            warmup = max(0, atoi(argv[++i]));
            continue;
        }
        pos.push_back(a);
    }

    string listCsv = pos.size() >= 1 ? pos[0] : "experiments/ch6_images.csv";
    string wmPath = pos.size() >= 2 ? pos[1] : "test_images/XM_32x32.bmp";
    string outCsv = pos.size() >= 3 ? pos[2] : (testsOutputDir() + "\\perf_test_results.csv");

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

    const int psfSize = kMotLen * 2 + 1;
    Mat psf = Preprocessor::generateMotionPSF(psfSize, kMotAngle, kMotLen);

    vector<double> t_imread, t_embed, t_extract, t_clahe, t_wiener_color;
    vector<double> t_nfr_extract_clahe, t_nfr_extract_wiener;

    for (const auto& it : images) {
        const string& path = it.second;
        if (warmup > 0)
            warmupDiscard(path, min(warmup, 3));

        // 1) 仅磁盘读取（与表 6-14「cv::imread」一致；不含后续缩放）
        {
            auto t0 = Clock::now();
            Mat raw = imread(path, IMREAD_COLOR);
            (void)raw;
            t_imread.push_back(elapsedMs(t0));
        }

        Mat cover = ch5LoadCover512(path);
        if (cover.empty()) continue;

        // 2) 嵌入
        Mat marked;
        {
            auto t0 = Clock::now();
            marked = WatermarkCodec::embed(cover, wm, kAlpha);
            t_embed.push_back(elapsedMs(t0));
        }

        // 3) 提取（不含 I/O：输入已在内存）
        {
            auto t0 = Clock::now();
            Mat ext = WatermarkCodec::extract(marked, cover, kAlpha);
            (void)ext;
            t_extract.push_back(elapsedMs(t0));
        }

        // 4) CLAHE 彩色整幅
        {
            auto t0 = Clock::now();
            Mat out = Preprocessor::clahe(cover, kClip, kTile);
            (void)out;
            t_clahe.push_back(elapsedMs(t0));
        }

        // 5) 维纳彩色（与退化实验一致：对含水印图做运动模糊再复原）
        Mat blurred = simulateMotionBlur(marked, kMotAngle, kMotLen, kMotNoise);
        {
            auto t0 = Clock::now();
            Mat rest = Preprocessor::wiener(blurred, psf, kWienerK);
            (void)rest;
            t_wiener_color.push_back(elapsedMs(t0));
        }

        // NFR-1：提取 + 双侧 CLAHE（与 WatermarkSystem 口径一致）
        {
            auto t0 = Clock::now();
            Mat d = Preprocessor::clahe(marked, kClip, kTile);
            Mat r = Preprocessor::clahe(cover, kClip, kTile);
            Mat ext = WatermarkCodec::extract(d, r, kAlpha);
            (void)ext;
            t_nfr_extract_clahe.push_back(elapsedMs(t0));
        }

        // NFR-1：提取 + 双侧维纳
        {
            auto t0 = Clock::now();
            Mat d = Preprocessor::wiener(blurred, psf, kWienerK);
            Mat r = Preprocessor::wiener(cover, psf, kWienerK);
            Mat ext = WatermarkCodec::extract(d, r, kAlpha);
            (void)ext;
            t_nfr_extract_wiener.push_back(elapsedMs(t0));
        }
    }

    const double m_imread = meanOf(t_imread);
    const double m_embed = meanOf(t_embed);
    const double m_extract = meanOf(t_extract);
    const double m_clahe = meanOf(t_clahe);
    const double m_w_color = meanOf(t_wiener_color);
    const double m_w_per_ch = m_w_color / 3.0;
    const double m_nfr_clahe = meanOf(t_nfr_extract_clahe);
    const double m_nfr_wien = meanOf(t_nfr_extract_wiener);

    ofstream csv;
    testsCsvOpen(csv, outCsv);
    csv << "operation,mean_ms,remark\n";
    csv << "imread," << m_imread << ",cv::imread per image path (raw file)\n";
    csv << "embed," << m_embed << ",alpha=" << kAlpha << " DWT-DCT-SVD\n";
    csv << "extract_no_preprocess," << m_extract << ",in-memory only\n";
    csv << "clahe_color," << m_clahe << ",clipLimit=" << kClip << " tile=" << kTile << "\n";
    csv << "wiener_est_per_channel," << m_w_per_ch << ",wiener_color_mean/3 (serial BGR)\n";
    csv << "wiener_color," << m_w_color << ",full BGR motion blur then wiener\n";
    csv << "nfr_extract_plus_clahe_both," << m_nfr_clahe << ",CLAHE(marked)+CLAHE(cover)+extract\n";
    csv << "nfr_extract_plus_wiener_both," << m_nfr_wien << ",wiener(blurred)+wiener(cover)+extract\n";

    cout << fixed << setprecision(2);
    cout << "=== Table 6-14 style (mean over " << t_embed.size() << " images) ===\n";
    cout << "imread (ms):                    " << m_imread << "\n";
    cout << "embed (ms):                     " << m_embed << "\n";
    cout << "extract no preprocess (ms):     " << m_extract << "\n";
    cout << "CLAHE color (ms):               " << m_clahe << "\n";
    cout << "Wiener est. per channel (ms):   " << m_w_per_ch << "  (= color/3)\n";
    cout << "Wiener color (ms):              " << m_w_color << "\n";
    cout << "--- NFR-1 composite ---\n";
    cout << "extract + bilateral CLAHE (ms): " << m_nfr_clahe << "  (limit 1500 ms)\n";
    cout << "extract + bilateral Wiener(ms):" << m_nfr_wien << "  (limit 3000 ms)\n";
    cout << "embed limit 2000 ms: " << (m_embed <= 2000 ? "PASS" : "FAIL") << "\n";
    cout << "Wrote " << outCsv << endl;
    return 0;
}
