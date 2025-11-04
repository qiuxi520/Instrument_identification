#include "imageprocessor.h"
#include <QDebug>

ImageProcessor::ImageProcessor(QObject *parent) : QObject(parent)
    , m_outputWidth(613)
    , m_outputHeight(580)
    , m_sigmaX(2.0)
    , m_sigmaY(2.0)
    , m_cannyThreshold1(50)
    , m_cannyThreshold2(150)
    , m_minRadius(281)
    , m_maxRadius(377)
    , m_rho(1)
    , m_theta(CV_PI/180)
    , m_threshold(30)
    , m_gaugeMinValue(0.0)        // 新增
    , m_gaugeMaxValue(15.0)        // 新增

{
    // 初始化默认的透视变换点
    m_sourcePoints = {
        cv::Point2f(60, 41),   // 左上
        cv::Point2f(620, 36),  // 右上
        cv::Point2f(585, 528), // 右下
        cv::Point2f(55, 582)   // 左下
    };
}

bool ImageProcessor::loadImage(const QString &fileName)
{
    m_originalImage = cv::imread(fileName.toStdString());
    if (m_originalImage.empty())
    {
        emit errorOccurred("无法加载图像文件: " + fileName);
        return false;
    }

    processAll();
    return true;
}

void ImageProcessor::processAll()
{
    if (m_originalImage.empty())
    {
        return;
    }

    ROISegmentation();
    applyPerspectiveTransform();
    convertToGray();
    applyGaussianBlur();
    detectEdges();
    detectCircles();
    maskROI();
    globalThreshold();
    Erosion();
    morphologyOperation();
    detectLines();
    analyzeGauge();
    Result();

    emit processingCompleted();
}

// --------------------提取ROI--------------------
void ImageProcessor::ROISegmentation()
{
    if (m_originalImage.empty()){return;}

    // 检查ROI参数是否在有效范围内
    if (m_ROIx < 0 || m_ROIy < 0 || m_ROIwidth <= 0 || m_ROIheight <= 0){return;}
    if (m_ROIx + m_ROIwidth > m_originalImage.cols || m_ROIy + m_ROIheight > m_originalImage.rows){return;}

    // 提取ROI区域
    cv::Rect roiRect(m_ROIx, m_ROIy, m_ROIwidth, m_ROIheight);
    m_ROIsegmentationImage=m_originalImage(roiRect);

    m_originalWithROIImage=m_originalImage.clone();
    // 绘制矩形框（BGR颜色格式）
    cv::rectangle(m_originalWithROIImage, roiRect, cv::Scalar(0, 255, 0), 10); // 绿色，线宽2像素

}
void ImageProcessor::setROIParams(int x,int y,int width,int height)
{
    m_ROIx=x;
    m_ROIy=y;
    m_ROIwidth=width;
    m_ROIheight=height;

    processAll();
}


// --------------------透视变换--------------------
void ImageProcessor::applyPerspectiveTransform()
{
    if (m_ROIsegmentationImage.empty()){return;}

    // 计算透视变换矩阵
    std::vector<cv::Point2f> dstPoints = {
        cv::Point2f(0, 0),
        cv::Point2f(m_outputWidth - 1, 0),
        cv::Point2f(m_outputWidth - 1, m_outputHeight - 1),
        cv::Point2f(0, m_outputHeight - 1)
    };
    cv::Mat transformMatrix = cv::getPerspectiveTransform(m_sourcePoints, dstPoints);
    cv::warpPerspective(m_ROIsegmentationImage, m_perspectiveTransformResult,transformMatrix, cv::Size(m_outputWidth, m_outputHeight));

    // 带透视变换区域标记的图像
    PTransformRangeImage = m_ROIsegmentationImage.clone();

    // 绘制透视变换区域
    std::vector<cv::Point> intPoints;
    for (const auto& pt : m_sourcePoints) //将坐标转换为整数坐标
    {
        intPoints.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }
    for (size_t i = 0; i < intPoints.size(); ++i) //绘制边界
    {
        cv::line(PTransformRangeImage, intPoints[i], intPoints[(i + 1) % intPoints.size()],cv::Scalar(0, 255, 0), 2);
    }
    for (const auto& pt : intPoints) //绘制角点
    {
        cv::circle(PTransformRangeImage, pt, 5, cv::Scalar(255, 0, 0), -1);
    }
}
void ImageProcessor::setPerspectivePoints(const std::vector<cv::Point2f> &points)
{
    if (points.size() == 4)
    {
        m_sourcePoints = points;
        processAll();
    }
}
void ImageProcessor::setOutputSize(int width, int height)
{
    m_outputWidth = width;
    m_outputHeight = height;
    processAll();
}

// --------------------高斯模糊--------------------
void ImageProcessor::convertToGray()
{
    if (m_perspectiveTransformResult.empty()){return;}

    cv::cvtColor(m_perspectiveTransformResult, m_grayImage, cv::COLOR_BGR2GRAY);
}
void ImageProcessor::applyGaussianBlur()
{
    if (m_grayImage.empty())
    {
        return;
    }

    cv::GaussianBlur(m_grayImage, m_blurredImage, cv::Size(9, 9), m_sigmaX, m_sigmaY);
}
void ImageProcessor::setGaussianSigma(double sigmaX, double sigmaY)
{
    m_sigmaX = sigmaX;
    m_sigmaY = sigmaY;

    processAll();
}


// --------------------边缘检测--------------------
void ImageProcessor::detectEdges()
{
    if (m_blurredImage.empty()){return;}

    cv::Canny(m_blurredImage, m_edgesImage, m_cannyThreshold1, m_cannyThreshold2);
}
void ImageProcessor::setCannyThresholds(int threshold1, int threshold2)
{
    m_cannyThreshold1 = threshold1;
    m_cannyThreshold2 = threshold2;

    processAll();
}


// --------------------霍夫圆检测--------------------
void ImageProcessor::detectCircles()
{
    if (m_edgesImage.empty()){return;}

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(m_edgesImage, circles, cv::HOUGH_GRADIENT, 1,
                     m_edgesImage.rows/16, 100, 30, m_minRadius, m_maxRadius);

    // 清空其他圆，只保留置信度最大的第一个
    if (!circles.empty()) {m_detectedCircle = circles[0];}
    m_circleCenterX = cvRound(m_detectedCircle[0]);
    m_circleCenterY = cvRound(m_detectedCircle[1]);
    m_circleCenter = cv::Point(m_circleCenterX,m_circleCenterY);
    m_circleRadius = cvRound(m_detectedCircle[2]);

    // 绘制检测到的圆
    m_circleImage = m_perspectiveTransformResult.clone();
    cv::circle(m_circleImage, m_circleCenter, m_circleRadius, cv::Scalar(0, 0, 255), 3); // 绘制圆周
    cv::circle(m_circleImage, m_circleCenter, 8, cv::Scalar(0, 255, 0), -1); // 绘制圆心
}
void ImageProcessor::setHoughCirclesParams(int minRadius, int maxRadius)
{
    m_minRadius = minRadius;
    m_maxRadius = maxRadius;

    processAll();
}


// --------------------创建仪表盘区域的掩膜--------------------
void ImageProcessor::maskROI()
{
    if (m_blurredImage.empty()){return;}

    // 每次重新创建掩码图像，确保清除之前的内容
    m_maskROIImage = cv::Mat::zeros(m_blurredImage.size(), m_blurredImage.type());

    // 创建圆形掩码
    cv::Mat mask = cv::Mat::zeros(m_blurredImage.size(), CV_8UC1);
    cv::circle(mask, m_circleCenter, m_maskRadius, cv::Scalar(255), -1); // -1表示填充

    // 应用掩码
    m_blurredImage.copyTo(m_maskROIImage, mask);
}
void ImageProcessor::setmaskROIParams(int maskRadius)
{
    m_maskRadius=maskRadius;

    processAll();
}


// --------------------二值化--------------------
void ImageProcessor::globalThreshold()
{
    if (m_maskROIImage.empty()){return;}

    cv::threshold(m_maskROIImage, m_thresholdImage, m_globalThreshold, 255, cv::THRESH_BINARY);
}
void ImageProcessor::setGlobalThresholdParams(double threshold, double maxValue)
{
    m_globalThreshold=threshold;
    m_maxValue=maxValue;

    processAll();
}


// --------------------腐蚀--------------------
void ImageProcessor::Erosion()
{
    if (m_thresholdImage.empty()){return;}

    if (m_kernelSize <= 0 || m_kernelSize % 2 == 0)
        m_kernelSize = 3; // 保证为正的奇数

    // 创建核（结构元素）
    m_kernel = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(m_kernelSize, m_kernelSize));
    cv::erode(m_thresholdImage, m_erosionResultImge, m_kernel, cv::Point(-1, -1), m_erosionIterations);
}
void ImageProcessor::setErosionParams(int kernelSize, int iterations)
{
    m_kernelSize=kernelSize;
    m_erosionIterations=iterations;

    processAll();
}


// --------------------形态学--------------------
void ImageProcessor::morphologyOperation()
{
    if (m_erosionResultImge.empty()){return;}

    cv::Mat result;
    // 开运算去除小噪点
    if (m_openKernelSize > 0)
    {
        cv::Mat openKernel = cv::getStructuringElement(m_kernelType,cv::Size(m_openKernelSize, m_openKernelSize));
        cv::morphologyEx(m_erosionResultImge, result, cv::MORPH_OPEN, openKernel,cv::Point(-1, -1), m_morphologyIterations);
    }

    // 闭运算连接断点
    if (m_closeKernelSize > 0)
    {
        cv::Mat closeKernel = cv::getStructuringElement(m_kernelType,cv::Size(m_closeKernelSize, m_closeKernelSize));
        cv::morphologyEx(result, m_morphologyResultImge, cv::MORPH_CLOSE, closeKernel,cv::Point(-1, -1), m_morphologyIterations);
    }

}
void ImageProcessor::setMorphologyOperationParams(int openKernelSize, int closeKernelSize, int kernelType, int iterations)
{
    m_openKernelSize=openKernelSize;
    m_closeKernelSize=closeKernelSize;
    m_kernelType = cv::MORPH_RECT;
    m_morphologyIterations=iterations;

    processAll();
}


// --------------------霍夫直线检测--------------------
void ImageProcessor::detectLines()
{
    if (m_morphologyResultImge.empty()){return;}

    lines.clear();
    HoughLinesP(m_morphologyResultImge, lines, m_rho, CV_PI/180, m_theta, m_minLineLength, m_maxLineGap);

    if (lines.empty()){m_lineImage=m_morphologyResultImge.clone();return;}

    // 找到最长的直线作为指针
    for (const auto& line : lines)
    {
        double length = norm(cv::Point2f(line[0], line[1]) - cv::Point2f(line[2], line[3]));
        if (length > maxLength)
        {
            maxLength = length;
            m_detectedLine = line;
        }
    }

    m_lineImage = cv::Mat::zeros(m_lineImage.size(), m_lineImage.type());

    m_lineImage=m_morphologyResultImge.clone();
    // 将灰度图转换为三通道图像
    if (m_lineImage.channels() == 1)
        cv::cvtColor(m_lineImage, m_lineImage, cv::COLOR_GRAY2BGR);

    // 计算指针角度（相对于圆心）
    cv::circle(m_lineImage, m_circleCenter, 5, cv::Scalar(0, 255, 0), -1); // -1表示填充
    cv::Point2i p1(m_detectedLine[0], m_detectedLine[1]);
    cv::Point2i p2(m_detectedLine[2], m_detectedLine[3]);

    // 取直线距圆心较远点作为指针末端
    double dist1 = norm(p1 - m_circleCenter);
    double dist2 = norm(p2 - m_circleCenter);
    pointerTip = (dist1 < dist2) ? p2 : p1;
    cv::line(m_lineImage, m_circleCenter, pointerTip, cv::Scalar(0, 0, 255), 4); // 红色，线宽2

}
void ImageProcessor::setHoughLinesParams(double rho,double theta,int threshold,double minLineLength, double maxLineGap)
{
    m_rho=rho;
    m_theta=theta;
    m_threshold=threshold;
    m_minLineLength = minLineLength;
    m_maxLineGap = maxLineGap;

    processAll();
}


// --------------------仪表分析--------------------
void ImageProcessor::analyzeGauge()
{
    // 计算角度
    m_angle = -atan2(m_circleCenter.y - pointerTip.y, pointerTip.x - m_circleCenter.x) * 180 / CV_PI;
    double adjustedAngle = (m_angle-m_gaugeMinValueAngle)/(m_gaugeMaxValueAngle-m_gaugeMinValueAngle);
    double range = m_gaugeMaxValue - m_gaugeMinValue;
    m_reading = adjustedAngle*range+m_gaugeMinValue;

}
void ImageProcessor::setGaugeParams(double minValueAngle, double maxValueAngle,double minValue, double maxValue)
{
    m_gaugeMinValueAngle=minValueAngle;
    m_gaugeMaxValueAngle=maxValueAngle;
    m_gaugeMinValue = minValue;
    m_gaugeMaxValue = maxValue;

    processAll();
}

// --------------------最终结果绘制--------------------
void ImageProcessor::Result()
{
    m_resultImage=m_perspectiveTransformResult.clone();
    cv::line(m_resultImage, m_circleCenter, pointerTip, cv::Scalar(0, 0, 255), 5); // 红色，线宽2
}

