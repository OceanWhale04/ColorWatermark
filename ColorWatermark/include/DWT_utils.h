#pragma once
#include <opencv2/opencv.hpp>

// 二维单层离散小波变换（一层分解）
// src: 输入单通道浮点图像
// LL, LH, HL, HH: 输出四个子带（尺寸减半）
void dwt2(const cv::Mat& src, cv::Mat& LL, cv::Mat& LH, cv::Mat& HL, cv::Mat& HH);

// 逆二维单层离散小波变换
// dst: 输出重建的单通道浮点图像
// LL, LH, HL, HH: 输入四个子带（尺寸应为输入图像的一半）
void idwt2(cv::Mat& dst, const cv::Mat& LL, const cv::Mat& LH, const cv::Mat& HL, const cv::Mat& HH);