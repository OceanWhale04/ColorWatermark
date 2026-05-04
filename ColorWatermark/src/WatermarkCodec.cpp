#include "WatermarkCodec.h"
#include "DWT_utils.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace cv;
using namespace std;

// 内部函数：对单通道图像执行完整的 DWT-DCT-SVD 流程，返回每个 8x8 块的第一个奇异值（存储为 E 矩阵）
static Mat computeSingularValues(const Mat& channel)
{
    CV_Assert(channel.type() == CV_32F);
    // 1. DWT
    Mat LL, LH, HL, HH;
    dwt2(channel, LL, LH, HL, HH);  // LL 尺寸减半

    // 2. 8x8 分块
    int blockSize = 8;
    int rowBlocks = LL.rows / blockSize;
    int colBlocks = LL.cols / blockSize;
    Mat sigmaMat(Size(colBlocks, rowBlocks), CV_32F);  // 存储每个块的最大奇异值

    // 确保 LL 尺寸能被8整除（实践中会先调整尺寸）
    for (int i = 0; i < rowBlocks; i++) {
        for (int j = 0; j < colBlocks; j++) {
            Rect roi(j * blockSize, i * blockSize, blockSize, blockSize);
            Mat block = LL(roi).clone();

            // DCT
            Mat dctBlock;
            dct(block, dctBlock);  // OpenCV dct 默认作用于 float 矩阵

            // SVD
            Mat U, Vt, w;
            SVD::compute(dctBlock, w, U, Vt); // w 是奇异值向量，降序排列

            sigmaMat.at<float>(i, j) = w.at<float>(0);  // 最大奇异值
        }
    }
    return sigmaMat;  // 返回与 LL 分块对应的奇异值矩阵
}

// 嵌入时的核心操作：修改奇异值矩阵
static void embedWatermarkToSigma(Mat& sigmaMat, const vector<int>& bits, double alpha)
{
    CV_Assert(sigmaMat.total() >= bits.size());
    int idx = 0;
    for (int i = 0; i < sigmaMat.rows; i++) {
        for (int j = 0; j < sigmaMat.cols; j++) {
            if (idx >= bits.size()) break;
            float S0 = sigmaMat.at<float>(i, j);
            float w = (float)bits[idx];
            sigmaMat.at<float>(i, j) = S0 + alpha * w;
            idx++;
        }
    }
}

// 嵌入后重建通道：利用修改后的 sigmaMat 替换对应块的奇异值，并逆 DCT、逆 DWT
static Mat reconstructChannelFromSigma(const Mat& channel, const Mat& sigmaMat, double /*alpha*/)
{
    CV_Assert(channel.type() == CV_32F);
    // 重新 DWT
    Mat LL, LH, HL, HH;
    dwt2(channel, LL, LH, HL, HH);

    int blockSize = 8;
    int rowBlocks = LL.rows / blockSize;
    int colBlocks = LL.cols / blockSize;

    // 创建新的 LL 副本（保留原始块数据，仅替换 DCT 系数中的奇异值）
    Mat newLL = LL.clone();

    int idx = 0;
    for (int i = 0; i < rowBlocks; i++) {
        for (int j = 0; j < colBlocks; j++) {
            float newSigma = sigmaMat.at<float>(i, j);

            Rect roi(j * blockSize, i * blockSize, blockSize, blockSize);
            Mat block = LL(roi).clone();

            // DCT
            Mat dctBlock;
            dct(block, dctBlock);

            // SVD
            Mat U, Vt, w;
            SVD::compute(dctBlock, w, U, Vt);

            // 用新奇异值替换第一个奇异值
            w.at<float>(0) = newSigma;

            // 重构对角矩阵
            Mat SigmaMatNew = Mat::diag(w);

            // 逆 SVD
            Mat dctNew = U * SigmaMatNew * Vt;

            // 逆 DCT
            Mat blockNew;
            idct(dctNew, blockNew);

            // 放回 LL
            blockNew.copyTo(newLL(roi));
        }
    }

    // 逆 DWT 得到重构通道
    Mat reconstructed;
    idwt2(reconstructed, newLL, LH, HL, HH);
    return reconstructed;
}

Mat WatermarkCodec::embed(const Mat& cover, const Mat& watermark, double alpha)
{
    CV_Assert(cover.channels() == 3);
    CV_Assert(watermark.channels() == 1);

    // 分离通道
    vector<Mat> bgr;
    split(cover, bgr);

    // 水印二值化并展平为双极性序列 {-1, +1}
    Mat wm;
    watermark.convertTo(wm, CV_32F, 1.0 / 255.0);
    wm = wm.reshape(1, 1);
    vector<int> bits;
    for (int i = 0; i < wm.cols; i++) {
        float b = round(wm.at<float>(0, i));
        bits.push_back((b > 0.5f) ? 1 : -1);
    }
    // 每个通道嵌入全量水印（三通道重复嵌入）
    vector<Mat> channelsOut;
    for (int ch = 0; ch < 3; ch++) {
        Mat chFloat;
        bgr[ch].convertTo(chFloat, CV_32F, 1.0 / 255.0);  // 归一化 [0,1]

        // 计算奇异值矩阵
        Mat sigmaMat = computeSingularValues(chFloat);

        // 修改奇异值
        embedWatermarkToSigma(sigmaMat, bits, alpha);

        // 重建该通道
        Mat chOut = reconstructChannelFromSigma(chFloat, sigmaMat, alpha);

        // 恢复 0-255 范围
        chOut = chOut * 255.0;
        chOut.convertTo(chOut, CV_8U);
        channelsOut.push_back(chOut);
    }

    Mat result;
    merge(channelsOut, result);
    return result;
}

Mat WatermarkCodec::extract(const Mat& watermarked, const Mat& original, double alpha)
{
    CV_Assert(watermarked.size() == original.size());
    vector<Mat> wmBGR, origBGR;
    split(watermarked, wmBGR);
    split(original, origBGR);

    // 假设水印长度为 32x32 = 1024（可动态决定）
    int wmSize = 32 * 32; // 可根据实际水印尺寸调整
    Mat voteAccum = Mat::zeros(1, wmSize, CV_32F);

    for (int ch = 0; ch < 3; ch++) {
        Mat wmFloat, origFloat;
        wmBGR[ch].convertTo(wmFloat, CV_32F, 1.0 / 255.0);
        origBGR[ch].convertTo(origFloat, CV_32F, 1.0 / 255.0);

        // 计算原始载体和待检测图像的奇异值矩阵
        Mat sigmaOrig = computeSingularValues(origFloat);
        Mat sigmaDet = computeSingularValues(wmFloat);

        int idx = 0;
        for (int i = 0; i < sigmaDet.rows; i++) {
            for (int j = 0; j < sigmaDet.cols; j++) {
                if (idx >= wmSize) break;
                float s_orig = sigmaOrig.at<float>(i, j);
                float s_det = sigmaDet.at<float>(i, j);
                float estimate = (s_det - s_orig) / alpha;
                voteAccum.at<float>(0, idx) += estimate;
                idx++;
            }
        }
    }

    // 投票：三通道平均后使用 0 阈值判定双极性比特
    voteAccum /= 3.0;
    Mat wmBin = Mat::zeros(1, wmSize, CV_8U);
    for (int i = 0; i < wmSize; i++) {
        float val = voteAccum.at<float>(0, i);
        wmBin.at<uchar>(0, i) = (val >= 0.0f) ? 255 : 0;
    }

    // 重塑为 32x32
    return wmBin.reshape(1, 32);
}