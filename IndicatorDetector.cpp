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
    greenRange.sMin1 = 0.3; greenRange.sMax1 = 0.5;
    greenRange.vMin1 = 0.15;  greenRange.vMax1 = 1.0;
    greenRange.hMin2 = 0.15; greenRange.hMax2 = 0.50;
    greenRange.sMin2 = 0.3; greenRange.sMax2 = 0.5;
    greenRange.vMin2 = 0.15;  greenRange.vMax2 = 1.0;


    yellowRange.hMin1 = 0; yellowRange.hMax1 = 0.2;
    yellowRange.sMin1 = 0.3; yellowRange.sMax1 = 1.0;
    yellowRange.vMin1 = 0.8; yellowRange.vMax1 = 1.0;
    yellowRange.hMin2 = 0; yellowRange.hMax2 = 0.20;
    yellowRange.sMin2 = 0.3; yellowRange.sMax2 = 1.0;
    yellowRange.vMin2 = 0.8; yellowRange.vMax2 = 1.0;
}

// IndicatorResult IndicatorDetector::detect(const cv::Mat& img, const QString& color)
// {
//     IndicatorResult result;
//     result.isOn = false;
//     result.confidence = 0.0;
//     result.color = color.toLower();
//     result.regionRatio = 0.0;

//     if (img.empty()) {
//         return result;
//     }

//     // 转换为HSV颜色空间
//     cv::Mat hsvImg;
//     cv::cvtColor(img, hsvImg, cv::COLOR_BGR2HSV);

//     // 归一化HSV值到0-1范围
//     hsvImg.convertTo(hsvImg, CV_32FC3, 1.0/255.0);

//     // 获取颜色范围
//     // HSVRange range = getColorRange(color);
//     HSVRange range = getRGYRange(color);

//     // 创建颜色掩码
//     cv::Mat colorMask = createColorMask(hsvImg, range);

//     // 形态学处理
//     cv::Mat processedMask = morphologicalProcess(colorMask);

//     // 计算颜色区域比例
//     double totalPixels = processedMask.rows * processedMask.cols;
//     double colorPixels = cv::countNonZero(processedMask);
//     double colorRatio = colorPixels / totalPixels;

//     // 判断是否亮起
//     result.isOn = colorRatio > m_threshold;
//     result.confidence = std::min(colorRatio * m_confidenceScale, 1.0);
//     result.colorMask = colorMask;
//     result.processedMask = processedMask;
//     result.regionRatio = colorRatio;

//     return result;
// }


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

    // 转换为 OpenCV HSV
    cv::Mat hsvImg;
    cv::cvtColor(img, hsvImg, cv::COLOR_BGR2HSV);

    // 获取颜色范围（0~1 范围 → 自动转换为 OpenCV 范围）
    HSVRange range = getRGYRange(color);

    // 创建颜色掩码（内部已正确处理 H=0 跨界）
    cv::Mat colorMask = createColorMask(hsvImg, range);

    // 形态学去噪
    cv::Mat processedMask = morphologicalProcess(colorMask);

    // 计算颜色区域比例
    double totalPixels = processedMask.rows * processedMask.cols;
    double colorPixels = cv::countNonZero(processedMask);
    double colorRatio = colorPixels / totalPixels;

    result.isOn = colorRatio > m_threshold;
    result.confidence = std::min(colorRatio * m_confidenceScale, 1.0);
    result.colorMask = colorMask;
    result.processedMask = processedMask;
    result.regionRatio = colorRatio;

    return result;
}


HSVRange IndicatorDetector::getRGYRange(const QString &color)
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

HSVRange IndicatorDetector::getColorRange(const QString& color)
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

// cv::Mat IndicatorDetector::createColorMask(const cv::Mat& hsvImg, const HSVRange& range)
// {
//     std::vector<cv::Mat> hsvChannels;
//     cv::split(hsvImg, hsvChannels);
//     cv::Mat H = hsvChannels[0];
//     cv::Mat S = hsvChannels[1];
//     cv::Mat V = hsvChannels[2];

//     // 第一个颜色范围
//     cv::Mat hMask1 = (H >= range.hMin1) & (H <= range.hMax1);
//     cv::Mat sMask1 = (S >= range.sMin1) & (S <= range.sMax1);
//     cv::Mat vMask1 = (V >= range.vMin1) & (V <= range.vMax1);
//     cv::Mat colorMask1 = hMask1 & sMask1 & vMask1;

//     // 第二个颜色范围
//     cv::Mat hMask2 = (H >= range.hMin2) & (H <= range.hMax2);
//     cv::Mat sMask2 = (S >= range.sMin2) & (S <= range.sMax2);
//     cv::Mat vMask2 = (V >= range.vMin2) & (V <= range.vMax2);
//     cv::Mat colorMask2 = hMask2 & sMask2 & vMask2;

//     // 合并颜色区域
//     cv::Mat colorMask = colorMask1 | colorMask2;
//     colorMask.convertTo(colorMask, CV_8UC1);

//     return colorMask;
// }
cv::Mat IndicatorDetector::createColorMask(const cv::Mat& hsvImg, const HSVRange& r)
{
    // 将 0~1 的配置转换为 OpenCV HSV 范围
    int hMin1 = r.hMin1 * 179;
    int hMax1 = r.hMax1 * 179;
    int hMin2 = r.hMin2 * 179;
    int hMax2 = r.hMax2 * 179;

    int sMin1 = r.sMin1 * 255;
    int sMax1 = r.sMax1 * 255;
    int vMin1 = r.vMin1 * 255;
    int vMax1 = r.vMax1 * 255;

    int sMin2 = r.sMin2 * 255;
    int sMax2 = r.sMax2 * 255;
    int vMin2 = r.vMin2 * 255;
    int vMax2 = r.vMax2 * 255;

    std::vector<cv::Mat> ch;
    cv::split(hsvImg, ch);
    cv::Mat H = ch[0], S = ch[1], V = ch[2];

    // 范围1
    cv::Mat mask1 = (H >= hMin1 & H <= hMax1) &
                    (S >= sMin1 & S <= sMax1) &
                    (V >= vMin1 & V <= vMax1);

    // 范围2
    cv::Mat mask2 = (H >= hMin2 & H <= hMax2) &
                    (S >= sMin2 & S <= sMax2) &
                    (V >= vMin2 & V <= vMax2);

    cv::Mat mask = mask1 | mask2;
    mask.convertTo(mask, CV_8UC1);

    return mask;
}

// cv::Mat IndicatorDetector::morphologicalProcess(const cv::Mat& mask)
// {
//     cv::Mat result = mask.clone();

//     // 开运算去除噪声
//     cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
//     cv::morphologyEx(result, result, cv::MORPH_OPEN, kernel);

//     // 闭运算填充空洞
//     cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel);

//     return result;
// }
cv::Mat IndicatorDetector::morphologicalProcess(const cv::Mat& mask)
{
    cv::Mat result;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));

    cv::morphologyEx(mask, result, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 1);
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 1);

    return result;
}
