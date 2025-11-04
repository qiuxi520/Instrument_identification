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

    // 分割ROI参数设置
    void setROIParams(int x=0,int y=0,int width=100,int height=100);
    // 透视变换参数设置
    void setPerspectivePoints(const std::vector<cv::Point2f> &points);
    void setOutputSize(int width, int height);
    // 高斯模糊参数设置
    void setGaussianSigma(double sigmaX, double sigmaY);
    // Canny边缘检测参数设置
    void setCannyThresholds(int threshold1, int threshold2);
    // 霍夫圆检测参数设置
    void setHoughCirclesParams(int minRadius, int maxRadius);
    void setmaskROIParams(int maskRadius);
    void setGlobalThresholdParams(double threshold, double maxValue);
    // 腐蚀参数设置
    void setErosionParams(int kernelSize, int iterations);
    // 形态学参数设置
    void setMorphologyOperationParams(int openKernelSize = 3, int closeKernelSize = 3, int kernelType = cv::MORPH_RECT, int iterations = 1);
    // 霍夫直线检测参数设置
    void setHoughLinesParams(double rho,double theta,int threshold,double minLineLength, double maxLineGap);
    // 仪表分析参数设置
    void setGaugeParams(double minValueAngle, double maxValueAngle,double minValue, double maxValue);


    // 获取处理结果
    cv::Mat getOriginalImage() const { return m_originalImage; }
    cv::Mat getOriginalWithROIImage() const { return m_originalWithROIImage; }
    cv::Mat getROIImage() const { return m_ROIsegmentationImage; }
    cv::Mat getPerspectiveTransformRange() const { return PTransformRangeImage; }
    cv::Mat getPerspectiveTransformResult() const { return m_perspectiveTransformResult; }
    cv::Mat getGrayImage() const { return m_grayImage; }
    cv::Mat getBlurredImage() const { return m_blurredImage; }
    cv::Mat getEdgesImage() const { return m_edgesImage; }
    cv::Mat getCirclesImage() const { return m_circleImage; }
    cv::Mat getMaskROIImage() const { return m_maskROIImage; }
    cv::Mat getThresholdImage() const { return m_thresholdImage; }
    cv::Mat getErosionImage() const { return m_erosionResultImge; }
    cv::Mat getmorphologyOperationImage() const { return m_morphologyResultImge; }
    cv::Mat getLineImage() const { return m_lineImage; }
    cv::Mat getResultImage() const { return m_resultImage; }


    cv::Vec3f getDetectedCircles() const { return m_detectedCircle; }
    cv::Vec4i getDetectedLines() const { return m_detectedLine; }

    double getReading() const { return m_reading; }
    double getAngle() const { return m_angle; }

    // 获取图像尺寸
    int getImageWidth() const { return m_originalImage.cols; }
    int getImageHeight() const { return m_originalImage.rows; }


signals:
    void processingCompleted();
    void errorOccurred(const QString &errorMessage);

private:
    cv::Mat m_originalImage;

    int m_ROIx,m_ROIy,m_ROIwidth,m_ROIheight;
    cv::Mat m_originalWithROIImage;
    cv::Mat m_ROIsegmentationImage;
    void ROISegmentation();

    cv::Mat PTransformRangeImage;
    cv::Mat m_perspectiveTransformResult;
    std::vector<cv::Point2f> m_sourcePoints;
    int m_outputWidth;
    int m_outputHeight;
    void applyPerspectiveTransform();

    cv::Mat m_grayImage;
    void convertToGray();

    double m_sigmaX,m_sigmaY;
    cv::Mat m_blurredImage;
    void applyGaussianBlur();

    int m_cannyThreshold1,m_cannyThreshold2;
    cv::Mat m_edgesImage;
    void detectEdges();

    int m_minRadius,m_maxRadius;
    cv::Mat m_circleImage;
    void detectCircles();
    std::vector<cv::Vec4i> circles;
    int m_circleCenterX,m_circleCenterY,m_circleRadius;
    cv::Point m_circleCenter;
    // 检测结果
    cv::Vec3f m_detectedCircle;


    int m_maskRadius;
    cv::Mat m_maskROIImage;
    void maskROI();


    double m_globalThreshold,m_maxValue;
    cv::Mat m_thresholdImage;
    void globalThreshold();

    int m_kernelSize,m_erosionIterations;
    cv::Mat m_erosionResultImge;
    cv::Mat m_kernel;
    void Erosion();

    int m_openKernelSize,m_closeKernelSize,m_kernelType = cv::MORPH_RECT,m_morphologyIterations;
    cv::Mat m_morphologyResultImge;
    void morphologyOperation();


    double m_rho,m_theta,m_minLineLength,m_maxLineGap;
    int m_threshold;
    cv::Mat m_lineImage;
    std::vector<cv::Vec4i> lines;
    cv::Vec4i m_detectedLine;
    double maxLength = 0;
    void detectLines();
    cv::Point2f pointerTip;

    // 仪表量程参数
    double m_gaugeMinValueAngle,m_gaugeMaxValueAngle,m_gaugeMinValue,m_gaugeMaxValue;
    double m_reading,m_angle;
    void analyzeGauge();

    cv::Mat m_resultImage;
    void Result();

};

#endif // IMAGEPROCESSOR_H
