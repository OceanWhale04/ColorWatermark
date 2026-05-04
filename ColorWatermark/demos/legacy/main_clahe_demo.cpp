#include <opencv2/opencv.hpp>
#include <iostream>
#include "Preprocessor.h"
#include "Utils.h"

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
    // --- 1. 读取载体图像 ---
    String inputPath = "test_images/clahe.bmp";   // 根据实际路径修改
    if (argc >= 2) inputPath = argv[1];
    Mat original = imread(inputPath, IMREAD_COLOR);
    if (original.empty()) {
        cerr << "无法加载图像：" << inputPath << endl;
        return -1;
    }

    // --- 2. 低光照退化（β=0.3 示例）---
    double beta = 0.3;          // 轻度低光照
    double noiseStd = 3.0;
    Mat lowLight = simulateLowLight(original, beta, noiseStd);

    // --- 3. CLAHE 增强 ---
    Mat enhanced = Preprocessor::clahe(lowLight, 2.0, 8);

    // --- 4. 拼接对比图并保存 ---
    Mat comparison = horizontalConcat(original, lowLight, enhanced);
    String outputFile = "demos/legacy/output_figures/clahe_contrast_beta0.3.png";
    imwrite(outputFile, comparison);

    bool success = imwrite(outputFile, comparison);
    if (!success) {
        cerr << "图像保存失败！请检查路径是否存在且可写: " << outputFile << endl;
        return -1;
    }
    cout << "CLAHE 对比图已成功保存至 " << outputFile << endl;

    // 可选：显示在窗口中
    imshow("CLAHE Effect - Original | Low-Light | Enhanced", comparison);
    waitKey(0);
    return 0;
}
