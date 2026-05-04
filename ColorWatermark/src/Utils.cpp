#include "Utils.h"

using namespace cv;
using namespace std;

Mat simulateLowLight(const Mat& src, double beta, double noiseStd) {
    Mat degraded;
    src.convertTo(degraded, CV_32F);
    degraded = degraded * beta;
    Mat noise(degraded.size(), degraded.type());
    randn(noise, 0, noiseStd);
    degraded += noise;
    degraded = cv::max(degraded, 0);
    degraded = cv::min(degraded, 255);
    degraded.convertTo(degraded, CV_8U);
    return degraded;
}

Mat simulateMotionBlur(const Mat& src, double angle, int length, double noiseStd) {
    int psfSize = length * 2 + 1;
    Mat psf = Mat::zeros(psfSize, psfSize, CV_32F);
    Point center(psfSize / 2, psfSize / 2);
    double rad = angle * CV_PI / 180.0;
    for (int i = -length / 2; i <= length / 2; i++) {
        int x = cvRound(center.x + i * cos(rad));
        int y = cvRound(center.y + i * sin(rad));
        if (x >= 0 && x < psfSize && y >= 0 && y < psfSize)
            psf.at<float>(y, x) = 1.0f;
    }
    psf /= sum(psf)[0];
    Mat blurred;
    filter2D(src, blurred, -1, psf, Point(-1, -1), 0, BORDER_REFLECT);
    Mat noise(blurred.size(), CV_32FC3);
    randn(noise, Scalar::all(0), Scalar::all(noiseStd));
    Mat blurredFloat;
    blurred.convertTo(blurredFloat, CV_32F);
    blurredFloat += noise;
    blurredFloat = cv::max(blurredFloat, 0);
    blurredFloat = cv::min(blurredFloat, 255);
    blurredFloat.convertTo(blurredFloat, CV_8U);
    return blurredFloat;
}

Mat horizontalConcat(const Mat& img1, const Mat& img2, const Mat& img3) {
    int maxHeight = max(max(img1.rows, img2.rows), img3.rows);
    auto resizeToHeight = [&](const Mat& img, int h) {
        if (img.rows == h) return img;
        Mat resized;
        resize(img, resized, Size(img.cols * h / img.rows, h));
        return resized;
        };
    Mat left = resizeToHeight(img1, maxHeight);
    Mat mid = resizeToHeight(img2, maxHeight);
    Mat right = resizeToHeight(img3, maxHeight);
    Mat result;
    hconcat(left, mid, result);
    hconcat(result, right, result);
    return result;
}

double computePSNR(const Mat& img1, const Mat& img2) {
    Mat diff;
    absdiff(img1, img2, diff);
    diff.convertTo(diff, CV_32F);
    diff = diff.mul(diff);
    Scalar mse = mean(diff);
    double mse_val = (mse[0] + mse[1] + mse[2]) / 3.0;
    if (mse_val < 1e-10) return 100.0;
    return 10.0 * log10(255.0 * 255.0 / mse_val);
}

double computeSSIM(const Mat& img1, const Mat& img2) {
    Mat gray1, gray2;
    if (img1.channels() == 3) {
        cvtColor(img1, gray1, COLOR_BGR2GRAY);
        cvtColor(img2, gray2, COLOR_BGR2GRAY);
    }
    else {
        gray1 = img1.clone();
        gray2 = img2.clone();
    }
    const double C1 = 6.5025, C2 = 58.5225;
    Mat I1, I2;
    gray1.convertTo(I1, CV_64F);
    gray2.convertTo(I2, CV_64F);
    Mat mu1, mu2;
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);
    Mat mu1_sq = mu1.mul(mu1);
    Mat mu2_sq = mu2.mul(mu2);
    Mat mu1_mu2 = mu1.mul(mu2);
    Mat sigma1_sq, sigma2_sq, sigma12;
    GaussianBlur(I1.mul(I1), sigma1_sq, Size(11, 11), 1.5);
    sigma1_sq -= mu1_sq;
    GaussianBlur(I2.mul(I2), sigma2_sq, Size(11, 11), 1.5);
    sigma2_sq -= mu2_sq;
    GaussianBlur(I1.mul(I2), sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;
    Mat t1 = 2.0 * mu1_mu2 + C1;
    Mat t2 = 2.0 * sigma12 + C2;
    Mat t3 = t1.mul(t2);
    t1 = mu1_sq + mu2_sq + C1;
    t2 = sigma1_sq + sigma2_sq + C2;
    t1 = t1.mul(t2);
    Mat ssim_map;
    divide(t3, t1, ssim_map);
    Scalar mssim = mean(ssim_map);
    return mssim[0];
}

double computeAccuracy(const Mat& extracted, const Mat& originalWM) {
    CV_Assert(extracted.size() == originalWM.size() && extracted.type() == CV_8U);
    int total = extracted.total();
    int correct = 0;
    for (int i = 0; i < extracted.rows; i++) {
        for (int j = 0; j < extracted.cols; j++) {
            uchar e = extracted.at<uchar>(i, j) > 128 ? 255 : 0;
            uchar o = originalWM.at<uchar>(i, j) > 128 ? 255 : 0;
            if (e == o) correct++;
        }
    }
    return 100.0 * correct / total;
}