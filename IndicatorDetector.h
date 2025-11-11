#ifndef INDICATORDETECTOR_H
#define INDICATORDETECTOR_H

#include <QString>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

struct IndicatorResult {
    bool isOn;          // 指示灯是否亮起
    double confidence;  // 置信度 (0-1)
    cv::Mat colorMask;  // 颜色掩码图像
    QString color;      // 检测的颜色
    double regionRatio; // 颜色区域比例
};

class IndicatorDetector
{
public:
    IndicatorDetector();

    // 检测指示灯状态
    IndicatorResult detect(const cv::Mat& img, const QString& color);

    // 设置检测阈值
    void setDetectionThreshold(double threshold) { m_threshold = threshold; }
    double getDetectionThreshold() const { return m_threshold; }

    // 设置置信度缩放因子
    void setConfidenceScale(double scale) { m_confidenceScale = scale; }
    double getConfidenceScale() const { return m_confidenceScale; }

private:
    // HSV颜色范围配置
    struct HSVRange {
        double hMin1, hMax1;  // 第一个色调范围
        double sMin1, sMax1;  // 第一个饱和度范围
        double vMin1, vMax1;  // 第一个亮度范围
        double hMin2, hMax2;  // 第二个色调范围（用于红色等跨色调边界的颜色）
        double sMin2, sMax2;  // 第二个饱和度范围
        double vMin2, vMax2;  // 第二个亮度范围
    };

    double m_threshold;       // 检测阈值
    double m_confidenceScale; // 置信度缩放因子

    // 获取对应颜色的HSV范围
    HSVRange getColorRange(const QString& color);

    // 创建颜色掩码
    cv::Mat createColorMask(const cv::Mat& hsvImg, const HSVRange& range);

    // 形态学处理
    cv::Mat morphologicalProcess(const cv::Mat& mask);
};

#endif // INDICATORDETECTOR_H
