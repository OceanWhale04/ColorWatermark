#pragma once
#include <opencv2/opencv.hpp>

// 低光照退化模拟
cv::Mat simulateLowLight(const cv::Mat& src, double beta, double noiseStd = 3.0);
// 运动模糊退化模拟
cv::Mat simulateMotionBlur(const cv::Mat& src, double angle, int length, double noiseStd = 2.0);
// 水平拼接三张图像
cv::Mat horizontalConcat(const cv::Mat& img1, const cv::Mat& img2, const cv::Mat& img3);
// 评价指标
double computePSNR(const cv::Mat& img1, const cv::Mat& img2);
double computeSSIM(const cv::Mat& img1, const cv::Mat& img2);
double computeAccuracy(const cv::Mat& extracted, const cv::Mat& originalWM); 
