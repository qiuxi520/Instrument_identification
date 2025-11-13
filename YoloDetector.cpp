#include "YoloDetector.h"


YoloDetector::YoloDetector(QObject *parent)
    : QObject(parent)
    , m_inference(nullptr)
    , m_initialized(false)
    , m_inputSize(320, 320)
{
}

bool YoloDetector::initialize(const std::string& modelPath,const cv::Size& inputSize,const std::string& classesFile,bool useGPU)
{
    try
    {
        m_inputSize = inputSize;
        m_inference = new Inference(modelPath, inputSize, classesFile, useGPU);
        m_initialized = true;

        // qDebug() << "YOLO detector initialized successfully";
        // qDebug() << "Model:" << QString::fromStdString(modelPath);
        // qDebug() << "Input size:" << inputSize.width << "x" << inputSize.height;
        // qDebug() << "GPU:" << (useGPU ? "Enabled" : "Disabled");

        return true;
    }
    catch (const std::exception& e)
    {
        qWarning() << "Failed to initialize YOLO detector:" << e.what();
        m_initialized = false;
        return false;
    }
}

std::vector<DetectionResult> YoloDetector::detect(const cv::Mat& frame)
{
    std::vector<DetectionResult> results;

    if (!m_initialized || !m_inference)
    {
        qWarning() << "YOLO detector not initialized";
        return results;
    }

    if (frame.empty())
    {
        qWarning() << "Input frame is empty";
        return results;
    }

    try
    {
        // 运行推理
        std::vector<Detection> detections = m_inference->runInference(frame);

        // 转换为结果格式
        for (const auto& detection : detections)
        {
            results.emplace_back(detection);
        }

        // qDebug() << "Detection completed. Found" << results.size() << "objects";

    }
    catch (const std::exception& e)
    {
        qWarning() << "Detection error:" << e.what();
    }

    return results;
}

cv::Mat YoloDetector::detectWithVisualization(const cv::Mat& frame)
{
    cv::Mat resultFrame = frame.clone();

    if (!m_initialized) {return resultFrame;}

    // 获取检测结果
    std::vector<DetectionResult> results = detect(frame);

    // 在图像上绘制检测结果
    for (const auto& detection : results)
    {
        // 绘制检测框
        cv::rectangle(resultFrame, detection.box, detection.color, 2);

        // 绘制类别和置信度文本
        std::string classString = detection.className + ' ' +std::to_string(detection.confidence).substr(0, 4);
        cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
        cv::Rect textBox(detection.box.x, detection.box.y - 40,textSize.width + 10, textSize.height + 20);

        cv::rectangle(resultFrame, textBox, detection.color, cv::FILLED);
        cv::putText(resultFrame, classString,cv::Point(detection.box.x + 5, detection.box.y - 10),cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
    }

    return resultFrame;
}
