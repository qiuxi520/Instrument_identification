#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <opencv2/opencv.hpp>

class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(QObject *parent = nullptr);

    // 图像加载和处理
    bool loadImage(const QString &fileName);
    void processAll();

    // 透视变换参数设置
    void setPerspectivePoints(const std::vector<cv::Point2f> &points);
    void setOutputSize(int width, int height);

    // 高斯模糊参数设置
    void setGaussianSigma(double sigmaX, double sigmaY);

    // Canny边缘检测参数设置
    void setCannyThresholds(int threshold1, int threshold2);

    // 霍夫圆检测参数设置
    void setHoughCirclesParams(int minRadius, int maxRadius);

    // 霍夫直线检测参数设置
    void setHoughLinesParams(int rho,double theta,int threshold,int minLineLength, int maxLineGap);

    // 仪表分析参数设置
    void setGaugeRange(double minValue, double maxValue);


    // 获取处理结果
    cv::Mat getOriginalImage() const { return m_originalImage; }
    cv::Mat getPerspectiveTransformResult() const { return m_perspectiveTransformResult; }
    cv::Mat getGrayImage() const { return m_grayImage; }
    cv::Mat getBlurredImage() const { return m_blurredImage; }
    cv::Mat getEdgesImage() const { return m_edgesImage; }

    cv::Mat getCirclesImage() const { return m_circleImage; }
    cv::Mat getLineImage() const { return m_lineImage; }

    cv::Vec3f getDetectedCircles() const { return m_detectedCircle; }
    cv::Vec4i getDetectedLines() const { return m_detectedLine; }

    double getReading() const { return reading; }

    // 获取图像尺寸
    int getImageWidth() const { return m_originalImage.cols; }
    int getImageHeight() const { return m_originalImage.rows; }


    // 仪表分析
    void analyzeGauge();
    double calculateReading(double angle, double minValue = 0.0, double maxValue = 1.0);

signals:
    void processingCompleted();
    void errorOccurred(const QString &errorMessage);

private:
    void applyPerspectiveTransform();
    void convertToGray();
    void applyGaussianBlur();
    void detectEdges();
    void detectCircles();
    void detectLines();

    // 图像数据
    cv::Mat m_originalImage;
    cv::Mat m_perspectiveTransformResult;
    cv::Mat m_grayImage;
    cv::Mat m_blurredImage;
    cv::Mat m_edgesImage;
    cv::Mat m_circleImage;
    cv::Mat m_lineImage;

    // 处理参数
    std::vector<cv::Point2f> m_sourcePoints;
    int m_outputWidth;
    int m_outputHeight;

    double m_sigmaX;
    double m_sigmaY;

    int m_cannyThreshold1;
    int m_cannyThreshold2;

    int m_minRadius;
    int m_maxRadius;


    int m_rho;
    double m_theta;
    int m_threshold;
    int m_minLineLength;
    int m_maxLineGap;

    // 检测结果
    std::vector<cv::Vec4i> circles;
    cv::Vec3f m_detectedCircle;

    int x,y,radius;
    std::vector<cv::Vec4i> lines;
    cv::Vec4i m_detectedLine;
    double maxLength = 0;

    // 仪表量程参数
    double m_gaugeMinValue;
    double m_gaugeMaxValue;
    double reading;
};

#endif // IMAGEPROCESSOR_H
