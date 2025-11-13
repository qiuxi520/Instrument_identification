#ifndef YOLODETECTOR_H
#define YOLODETECTOR_H

#include <QObject>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "yoloInference.h"

struct DetectionResult
{
    cv::Rect box;
    std::string className;
    float confidence;
    cv::Scalar color;

    DetectionResult(const Detection& detection)
        : box(detection.box)
        , className(detection.className)
        , confidence(detection.confidence)
        , color(detection.color)
    {}
};

class YoloDetector : public QObject
{
    Q_OBJECT

public:
    explicit YoloDetector(QObject *parent = nullptr);

    // 初始化检测器
    bool initialize(const std::string& modelPath,
                    const cv::Size& inputSize = cv::Size(320, 320),
                    const std::string& classesFile = "classes.txt",
                    bool useGPU = false);

    // 核心检测方法：输入Mat，返回检测结果
    std::vector<DetectionResult> detect(const cv::Mat& frame);

    // 带绘制的检测方法：在原图上绘制检测结果
    cv::Mat detectWithVisualization(const cv::Mat& frame);

    // 获取模型信息
    bool isInitialized() const { return m_initialized; }
    cv::Size getInputSize() const { return m_inputSize; }

private:
    Inference* m_inference;
    bool m_initialized;
    cv::Size m_inputSize;
};

#endif // YOLODETECTOR_H
