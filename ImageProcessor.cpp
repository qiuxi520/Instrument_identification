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

    applyPerspectiveTransform();
    convertToGray();
    applyGaussianBlur();
    detectEdges();
    detectCircles();
    detectLines();
    analyzeGauge();

    emit processingCompleted();
}

// --------------------透视变换--------------------
void ImageProcessor::applyPerspectiveTransform()
{
    if (m_originalImage.empty())
    {
        return;
    }

    // 计算透视变换矩阵
    std::vector<cv::Point2f> dstPoints = {
        cv::Point2f(0, 0),
        cv::Point2f(m_outputWidth - 1, 0),
        cv::Point2f(m_outputWidth - 1, m_outputHeight - 1),
        cv::Point2f(0, m_outputHeight - 1)
    };

    cv::Mat transformMatrix = cv::getPerspectiveTransform(m_sourcePoints, dstPoints);
    cv::warpPerspective(m_originalImage, m_perspectiveTransformResult,
                        transformMatrix, cv::Size(m_outputWidth, m_outputHeight));
}
void ImageProcessor::setPerspectivePoints(const std::vector<cv::Point2f> &points)
{
    if (points.size() == 4)
    {
        m_sourcePoints = points;
        if (!m_originalImage.empty())
        {
            processAll();
        }
    }
}
void ImageProcessor::setOutputSize(int width, int height)
{
    m_outputWidth = width;
    m_outputHeight = height;
    if (!m_originalImage.empty())
    {
        processAll();
    }
}


// --------------------高斯模糊--------------------
void ImageProcessor::convertToGray()
{
    if (m_perspectiveTransformResult.empty())
    {
        return;
    }

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
    if (!m_originalImage.empty())
    {
        applyGaussianBlur();
        detectEdges();
        detectCircles();
        detectLines();
        analyzeGauge();

        emit processingCompleted();
    }
}


// --------------------边缘检测--------------------
void ImageProcessor::detectEdges()
{
    if (m_blurredImage.empty())
    {
        return;
    }

    cv::Canny(m_blurredImage, m_edgesImage, m_cannyThreshold1, m_cannyThreshold2);
}

void ImageProcessor::setCannyThresholds(int threshold1, int threshold2)
{
    m_cannyThreshold1 = threshold1;
    m_cannyThreshold2 = threshold2;
    if (!m_originalImage.empty())
    {
        detectEdges();
        detectCircles();
        detectLines();
        analyzeGauge();

        emit processingCompleted();
    }
}


// --------------------霍夫圆检测--------------------
void ImageProcessor::detectCircles()
{
    if (m_edgesImage.empty())
    {
        return;
    }

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(m_edgesImage, circles, cv::HOUGH_GRADIENT, 1,
                     m_edgesImage.rows/16, 100, 30, m_minRadius, m_maxRadius);

    // 清空其他圆，只保留置信度最大的第一个
    if (!circles.empty()) {m_detectedCircle = circles[0];}
    x = cvRound(m_detectedCircle[0]);
    y = cvRound(m_detectedCircle[1]);
    radius = cvRound(m_detectedCircle[2]);

    // 绘制检测到的圆并进行仪表分析
    m_circleImage = m_perspectiveTransformResult.clone();

    cv::Point center(x, y);
    cv::circle(m_circleImage, center, radius, cv::Scalar(0, 0, 255), 2); // 绘制圆周
    cv::circle(m_circleImage, center, 3, cv::Scalar(0, 255, 0), -1); // 绘制圆心
}

void ImageProcessor::setHoughCirclesParams(int minRadius, int maxRadius)
{
    m_minRadius = minRadius;
    m_maxRadius = maxRadius;
    if (!m_originalImage.empty())
    {
        detectCircles();
        detectLines();
        analyzeGauge();

        emit processingCompleted();
    }
}


// --------------------霍夫直线检测--------------------
void ImageProcessor::detectLines()
{
    if (m_edgesImage.empty())
    {
        return;
    }

    // 创建ROI区域
    cv::Rect roi(x - radius, y - radius, radius * 2, radius * 2);
    roi = roi & cv::Rect(0, 0, m_edgesImage.cols, m_edgesImage.rows);
    if (roi.width <= 0 || roi.height <= 0)
    {
        return;
    }

    cv::Mat roiImage = m_edgesImage(roi).clone();

    // 检测直线（指针）
    // HoughLinesP(roiImage, lines, 1, CV_PI/180, 30, radius/2, radius/4);
    HoughLinesP(roiImage, lines, 1, CV_PI/180, 30, m_minLineLength, m_maxLineGap);
    // HoughLinesP(roiImage, lines, m_rho, m_theta, m_threshold, m_minLineLength, m_maxLineGap);
    if (lines.empty())
    {
        return;
    }

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

    m_lineImage=m_circleImage.clone();

    cv::rectangle(m_lineImage, roi, cv::Scalar(255, 255, 0), 2);
    // 绘制直线
    // cv::Point pt1(m_detectedLine[0], m_detectedLine[1]);
    // cv::Point pt2(m_detectedLine[2], m_detectedLine[3]);

    // 绘制直线（注意坐标转换）
    cv::Point pt1(m_detectedLine[0] + roi.x, m_detectedLine[1] + roi.y);
    cv::Point pt2(m_detectedLine[2] + roi.x, m_detectedLine[3] + roi.y);
    cv::line(m_lineImage, pt1, pt2, cv::Scalar(0, 0, 255), 2); // 红色，线宽2
}

void ImageProcessor::setHoughLinesParams(int rho,double theta,int threshold,int minLineLength, int maxLineGap)
{
    m_rho=rho;
    m_theta=theta;
    m_threshold=threshold;
    m_minLineLength = minLineLength;
    m_maxLineGap = maxLineGap;
    if (!m_originalImage.empty())
    {
        detectLines();
        analyzeGauge();

        emit processingCompleted();
    }
}




// --------------------仪表分析--------------------
void ImageProcessor::analyzeGauge()
{
    // 计算指针角度（相对于圆心）
    cv::Point2f center(radius, radius); // ROI内的相对中心
    cv::Point2f p1(m_detectedLine[0], m_detectedLine[1]);
    cv::Point2f p2(m_detectedLine[2], m_detectedLine[3]);

    // 确定哪个端点更接近圆心
    double dist1 = norm(p1 - center);
    double dist2 = norm(p2 - center);
    cv::Point2f pointerTip = (dist1 < dist2) ? p2 : p1;

    // 计算角度（0-360度，0点在右侧，顺时针增加）
    double angle = atan2(center.y - pointerTip.y, pointerTip.x - center.x) * 180 / CV_PI;
    if (angle < 0) angle += 360;

    // 转换为实际读数
    reading = calculateReading(angle, m_gaugeMinValue, m_gaugeMaxValue);
}

double ImageProcessor::calculateReading(double angle, double minValue, double maxValue)
{
    // 假设0度在右侧，270度在顶部（模拟实际仪表）
    // 调整角度起始位置
    double adjustedAngle = fmod(angle + 270, 360);

    // 将角度转换为读数
    double range = maxValue - minValue;
    double reading = (adjustedAngle / 360.0) * range + minValue;

    return reading;
}

void ImageProcessor::setGaugeRange(double minValue, double maxValue)
{
    m_gaugeMinValue = minValue;
    m_gaugeMaxValue = maxValue;

    analyzeGauge();
    emit processingCompleted();
}

