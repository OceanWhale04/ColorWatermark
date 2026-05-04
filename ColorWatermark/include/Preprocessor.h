#pragma once
#include <opencv2/opencv.hpp>

class Preprocessor {
public:
    // CLAHE 低光照增强
    // src: 输入彩色图像（BGR）或灰度图
    // clipLimit: 对比度裁剪阈值，默认2.0
    // tileSize: 分块大小，默认8
    static cv::Mat clahe(const cv::Mat& src, double clipLimit = 2.0, int tileSize = 8);

    // 维纳滤波运动模糊复原
    // blurred: 退化图像（BGR 彩色）
    // psf: 点扩散函数（单通道 float，尺寸应与运动模糊核对应）
    // K: 信噪比参数，默认0.01
    static cv::Mat wiener(const cv::Mat& blurred, const cv::Mat& psf, double K = 0.01);

    // 生成匀速直线运动模糊 PSF 核
    // size: 输出核的大小（正方形），应大于等于模糊长度
    // angle: 运动方向（度，0为水平）
    // length: 模糊长度（像素）
    static cv::Mat generateMotionPSF(int size, double angle, int length);
};