#include "IndicatorDetector.h"


IndicatorDetector::IndicatorDetector()
{
    // 默认参数
    m_threshold = 0.001;      // 默认检测阈值
    m_confidenceScale = 100.0; // 默认置信度缩放因子



    redRange.hMin1 = 0.874; redRange.hMax1 = 1.000;
    redRange.sMin1 = 0.525; redRange.sMax1 = 0.955;
    redRange.vMin1 = 0.502; redRange.vMax1 = 0.976;
    redRange.hMin2 = 0.00; redRange.hMax2 = 0.10;
    redRange.sMin2 = 0.525; redRange.sMax2 = 0.955;
    redRange.vMin2 = 0.502; redRange.vMax2 = 0.976;


    greenRange.hMin1 = 0.15; greenRange.hMax1 = 0.50;
    greenRange.sMin1 = 0.02; greenRange.sMax1 = 0.30;
    greenRange.vMin1 = 0.4;  greenRange.vMax1 = 1.0;
    greenRange.hMin2 = 0.15; greenRange.hMax2 = 0.50;
    greenRange.sMin2 = 0.02; greenRange.sMax2 = 0.30;
    greenRange.vMin2 = 0.4;  greenRange.vMax2 = 1.0;


    yellowRange.hMin1 = 0.12; yellowRange.hMax1 = 0.18;
    yellowRange.sMin1 = 0.5; yellowRange.sMax1 = 1.0;
    yellowRange.vMin1 = 0.4; yellowRange.vMax1 = 1.0;
    yellowRange.hMin2 = 0.10; yellowRange.hMax2 = 0.20;
    yellowRange.sMin2 = 0.4; yellowRange.sMax2 = 1.0;
    yellowRange.vMin2 = 0.3; yellowRange.vMax2 = 1.0;
}

IndicatorResult IndicatorDetector::detect(const cv::Mat& img, const QString& color)
{
    IndicatorResult result;
    result.isOn = false;
    result.confidence = 0.0;
    result.color = color.toLower();
    result.regionRatio = 0.0;

    if (img.empty()) {
        return result;
    }

    // 转换为HSV颜色空间
    cv::Mat hsvImg;
    cv::cvtColor(img, hsvImg, cv::COLOR_BGR2HSV);

    // 归一化HSV值到0-1范围
    hsvImg.convertTo(hsvImg, CV_32FC3, 1.0/255.0);

    // 获取颜色范围
    // HSVRange range = getColorRange(color);
    HSVRange range = getRGYRange(color);

    // 创建颜色掩码
    cv::Mat colorMask = createColorMask(hsvImg, range);

    // 形态学处理
    cv::Mat processedMask = morphologicalProcess(colorMask);

    // 计算颜色区域比例
    double totalPixels = processedMask.rows * processedMask.cols;
    double colorPixels = cv::countNonZero(processedMask);
    double colorRatio = colorPixels / totalPixels;

    // 判断是否亮起
    result.isOn = colorRatio > m_threshold;
    result.confidence = std::min(colorRatio * m_confidenceScale, 1.0);
    result.colorMask = colorMask;
    result.processedMask = processedMask;
    result.regionRatio = colorRatio;

    return result;
}

void IndicatorDetector::setRRangeParam(double hMin1, double hMax1, double sMin1, double sMax1, double vMin1, double vMax1,
                                        double hMin2,double hMax2,double sMin2,double sMax2,double vMin2,double vMax2)
{
    redRange.hMin1 = hMin1; redRange.hMax1 = hMax1;
    redRange.sMin1 = sMin1; redRange.sMax1 = sMax1;
    redRange.vMin1 = vMin1; redRange.vMax1 = vMax1;
    redRange.hMin2 = hMin2; redRange.hMax2 = hMax2;
    redRange.sMin2 = sMin2; redRange.sMax2 = sMax2;
    redRange.vMin2 = vMin2; redRange.vMax2 = vMax2;

}

void IndicatorDetector::setGRangeParam(double hMin1, double hMax1, double sMin1, double sMax1, double vMin1, double vMax1,
                                       double hMin2, double hMax2, double sMin2, double sMax2, double vMin2, double vMax2)
{
    greenRange.hMin1 = hMin1; greenRange.hMax1 = hMax1;
    greenRange.sMin1 = sMin1; greenRange.sMax1 = sMax1;
    greenRange.vMin1 = vMin1; greenRange.vMax1 = vMax1;
    greenRange.hMin2 = hMin2; greenRange.hMax2 = hMax2;
    greenRange.sMin2 = sMin2; greenRange.sMax2 = sMax2;
    greenRange.vMin2 = vMin2; greenRange.vMax2 = vMax2;
}

void IndicatorDetector::setYRangeParam(double hMin1, double hMax1, double sMin1, double sMax1, double vMin1, double vMax1,
                                       double hMin2, double hMax2, double sMin2, double sMax2, double vMin2, double vMax2)
{
    yellowRange.hMin1 = hMin1; yellowRange.hMax1 = hMax1;
    yellowRange.sMin1 = sMin1; yellowRange.sMax1 = sMax1;
    yellowRange.vMin1 = vMin1; yellowRange.vMax1 = vMax1;
    yellowRange.hMin2 = hMin2; yellowRange.hMax2 = hMax2;
    yellowRange.sMin2 = sMin2; yellowRange.sMax2 = sMax2;
    yellowRange.vMin2 = vMin2; yellowRange.vMax2 = vMax2;
}

IndicatorDetector::HSVRange IndicatorDetector::getRGYRange(const QString &color)
{
    if (color == "red")
        return redRange;
    else if (color == "green")
        return greenRange;
    else if (color == "yellow")
        return yellowRange;
    else
        return redRange;
}

IndicatorDetector::HSVRange IndicatorDetector::getColorRange(const QString& color)
{
    HSVRange range;
    QString colorLower = color.toLower();

    if (colorLower == "red") {
        // 基于数据分析的优化红色范围
        range.hMin1 = 0.874; range.hMax1 = 1.000;
        range.sMin1 = 0.525; range.sMax1 = 0.955;
        range.vMin1 = 0.502; range.vMax1 = 0.976;

        range.hMin2 = 0.00; range.hMax2 = 0.10;
        range.sMin2 = 0.525; range.sMax2 = 0.955;
        range.vMin2 = 0.502; range.vMax2 = 0.976;
    }


    else if (colorLower == "green") {
        // 扩展绿色范围以包含两种亮起状态：
        // (0.188,0.032,0.988) - 低饱和度、高亮度
        // (0.314,0.291,0.459) - 中饱和度、中亮度

        range.hMin1 = 0.15; range.hMax1 = 0.50;      // 色调范围保持不变
        range.sMin1 = 0.02; range.sMax1 = 0.30;      // 饱和度上限从1.0→0.30，包含0.291
        range.vMin1 = 0.4;  range.vMax1 = 1.0;       // 明度下限从0.9→0.4，包含0.459

        range.hMin2 = 0.15; range.hMax2 = 0.50;
        range.sMin2 = 0.02; range.sMax2 = 0.30;
        range.vMin2 = 0.4;  range.vMax2 = 1.0;
    }

    else if (colorLower == "yellow") {
        // 黄色范围（替换原来的蓝色范围）
        range.hMin1 = 0.12; range.hMax1 = 0.18;
        range.sMin1 = 0.5; range.sMax1 = 1.0;
        range.vMin1 = 0.4; range.vMax1 = 1.0;
        range.hMin2 = 0.10; range.hMax2 = 0.20;
        range.sMin2 = 0.4; range.sMax2 = 1.0;
        range.vMin2 = 0.3; range.vMax2 = 1.0;
    }
    else {
        // 默认使用红色范围
        range.hMin1 = 0.95; range.hMax1 = 1.0;
        range.sMin1 = 0.5; range.sMax1 = 1.0;
        range.vMin1 = 0.3; range.vMax1 = 1.0;
        range.hMin2 = 0.0; range.hMax2 = 0.05;
        range.sMin2 = 0.3; range.sMax2 = 1.0;
        range.vMin2 = 0.5; range.vMax2 = 1.0;
    }

    return range;
}

cv::Mat IndicatorDetector::createColorMask(const cv::Mat& hsvImg, const HSVRange& range)
{
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvImg, hsvChannels);
    cv::Mat H = hsvChannels[0];
    cv::Mat S = hsvChannels[1];
    cv::Mat V = hsvChannels[2];

    // 第一个颜色范围
    cv::Mat hMask1 = (H >= range.hMin1) & (H <= range.hMax1);
    cv::Mat sMask1 = (S >= range.sMin1) & (S <= range.sMax1);
    cv::Mat vMask1 = (V >= range.vMin1) & (V <= range.vMax1);
    cv::Mat colorMask1 = hMask1 & sMask1 & vMask1;

    // 第二个颜色范围
    cv::Mat hMask2 = (H >= range.hMin2) & (H <= range.hMax2);
    cv::Mat sMask2 = (S >= range.sMin2) & (S <= range.sMax2);
    cv::Mat vMask2 = (V >= range.vMin2) & (V <= range.vMax2);
    cv::Mat colorMask2 = hMask2 & sMask2 & vMask2;

    // 合并颜色区域
    cv::Mat colorMask = colorMask1 | colorMask2;
    colorMask.convertTo(colorMask, CV_8UC1);

    return colorMask;
}

cv::Mat IndicatorDetector::morphologicalProcess(const cv::Mat& mask)
{
    cv::Mat result = mask.clone();

    // 开运算去除噪声
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(result, result, cv::MORPH_OPEN, kernel);

    // 闭运算填充空洞
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel);

    return result;
}
