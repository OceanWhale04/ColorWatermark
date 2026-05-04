#pragma once
#include <opencv2/opencv.hpp>

class WatermarkCodec {
public:
    // 嵌入水印，返回含水印图像
    // cover: 8-bit 3通道彩色图
    // watermark: 8-bit 单通道二值图（0/255）
    // alpha: 嵌入强度
    static cv::Mat embed(const cv::Mat& cover, const cv::Mat& watermark, double alpha);

    // 提取水印，返回二值水印图像
    // watermarked: 待检测图像（可能已退化）
    // original: 原始载体图像（非盲提取所需）
    // alpha: 嵌入强度（需与嵌入时一致）
    static cv::Mat extract(const cv::Mat& watermarked, const cv::Mat& original, double alpha);
};