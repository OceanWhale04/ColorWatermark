#include <opencv2/opencv.hpp>
#include <iostream>
#include "Preprocessor.h"
#include "Utils.h"

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
    // --- 1. 读取载体图像 ---
    String inputPath = "test_images/wiener.bmp";
    if (argc >= 2) inputPath = argv[1];
    Mat original = imread(inputPath, IMREAD_COLOR);
    if (original.empty()) {
        cerr << "无法加载图像: " << inputPath << endl;
        return -1;
    }

    // --- 2. 运动模糊退化（默认：角度30°，长度15像素）---
    double angle = 30.0;       // 可在代码中修改
    int length = 15;           // 模糊长度（像素）
    double noiseStd = 2.0;     // 噪声标准差
    Mat blurred = simulateMotionBlur(original, angle, length, noiseStd);

    // --- 3. 维纳滤波复原 ---
    // 使用与退化相同的 PSF 参数（论文5.4.1节：已知真实PSF的理想条件）
    int psfSize = length * 2 + 1;
    Mat psf = Preprocessor::generateMotionPSF(psfSize, angle, length);
    double K = 0.01;           // 维纳滤波信噪比参数
    Mat restored = Preprocessor::wiener(blurred, psf, K);

    // --- 4. 拼接并保存对比图 ---
    Mat comparison = horizontalConcat(original, blurred, restored);
    String outputFile = "demos/legacy/output_figures/wiener_contrast_angle30_len15.png";
    bool success = imwrite(outputFile, comparison);
    if (!success) {
        cerr << "图像保存失败！请检查路径是否存在且可写: " << outputFile << endl;
        return -1;
    }
    cout << "维纳滤波对比图已成功保存至 " << outputFile << endl;

    // 可选：显示窗口
    imshow("Wiener Filter Effect - Original | Blurred | Restored", comparison);
    waitKey(0);
    return 0;
}
