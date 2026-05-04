#include "Preprocessor.h"
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

Mat Preprocessor::clahe(const Mat& src, double clipLimit, int tileSize)
{
    if (src.channels() == 1) {
        Mat result;
        Ptr<CLAHE> cl = createCLAHE(clipLimit, Size(tileSize, tileSize));
        cl->apply(src, result);
        return result;
    }
    else {
        // 转换为 LAB，仅增强 L 通道
        Mat lab;
        cvtColor(src, lab, COLOR_BGR2Lab);
        vector<Mat> labs;
        split(lab, labs);
        Ptr<CLAHE> cl = createCLAHE(clipLimit, Size(tileSize, tileSize));
        cl->apply(labs[0], labs[0]);  // L 通道
        merge(labs, lab);
        Mat result;
        cvtColor(lab, result, COLOR_Lab2BGR);
        return result;
    }
}

Mat Preprocessor::wiener(const Mat& blurred, const Mat& psf, double K)
{
    CV_Assert(blurred.channels() == 3);
    vector<Mat> bgr;
    split(blurred, bgr);
    vector<Mat> restoredChannels;

    for (int i = 0; i < 3; i++) {
        Mat img;
        bgr[i].convertTo(img, CV_32F);
        // 将图像和 PSF 扩展到最优 DFT 尺寸
        Mat paddedImg, paddedPSF;
        int optRows = getOptimalDFTSize(img.rows);
        int optCols = getOptimalDFTSize(img.cols);
        copyMakeBorder(img, paddedImg, 0, optRows - img.rows, 0, optCols - img.cols, BORDER_REFLECT);
        Mat psfFloat;
        psf.convertTo(psfFloat, CV_32F);
        copyMakeBorder(psfFloat, paddedPSF, 0, optRows - psf.rows, 0, optCols - psf.cols, BORDER_CONSTANT, 0);

        Mat G;
        dft(paddedImg, G, DFT_COMPLEX_OUTPUT);   // 模糊图像频谱
        Mat H;
        dft(paddedPSF, H, DFT_COMPLEX_OUTPUT);   // PSF 频谱

        // 维纳滤波：F = H* / (|H|^2 + K) * G
        Mat F(G.size(), G.type());
        for (int v = 0; v < G.rows; v++) {
            for (int u = 0; u < G.cols; u++) {
                Vec2f g = G.at<Vec2f>(v, u);
                Vec2f h = H.at<Vec2f>(v, u);
                float magH2 = h[0] * h[0] + h[1] * h[1];
                float denom = magH2 + K;
                // H 的复共轭
                float Hconj_re = h[0];
                float Hconj_im = -h[1];
                // 复数除法
                float factor_re = (Hconj_re * denom) / (denom * denom + 0.0);
                float factor_im = (Hconj_im * denom) / (denom * denom + 0.0);
                // 复数相乘
                F.at<Vec2f>(v, u)[0] = factor_re * g[0] - factor_im * g[1];
                F.at<Vec2f>(v, u)[1] = factor_re * g[1] + factor_im * g[0];
            }
        }

        Mat restored;
        idft(F, restored, DFT_REAL_OUTPUT | DFT_SCALE);
        restored = restored(Rect(0, 0, img.cols, img.rows)).clone();
        restored.convertTo(restored, CV_8U);
        restoredChannels.push_back(restored);
    }
    Mat result;
    merge(restoredChannels, result);
    return result;
}

Mat Preprocessor::generateMotionPSF(int size, double angle, int length)
{
    Mat psf = Mat::zeros(size, size, CV_32F);
    Point center(size / 2, size / 2);
    double rad = angle * CV_PI / 180.0;
    for (int i = -length / 2; i <= length / 2; i++) {
        int x = cvRound(center.x + i * cos(rad));
        int y = cvRound(center.y + i * sin(rad));
        if (x >= 0 && x < size && y >= 0 && y < size)
            psf.at<float>(y, x) = 1.0f;
    }
    // 归一化
    psf /= sum(psf)[0];
    return psf;
}