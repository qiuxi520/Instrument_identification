#include "ImgLoader.h"
#include <QDebug>

ImgLoader::ImgLoader(QObject *parent)
    : QObject(parent)
{
}

ImgLoader::~ImgLoader()
{
}

// 设置参数
void ImgLoader::setROIParams(int idx, ROIType type, cv::Rect rect, std::vector<cv::Point2f> transformPoints)
{
    if (idx >= 0 && idx < ROISNUM)
    {
        rois[idx].type = type;
        rois[idx].rect = rect;
        rois[idx].transformPoints = transformPoints;
    }
}
// 重载 - 设置索引和矩形
void ImgLoader::setROIParams(int idx,cv::Rect rect)
{
    if (idx >= 0 && idx < ROISNUM)
    {
        rois[idx].rect = rect;
    }
}

void ImgLoader::setROIParams(int idx, ROIType type)
{
    if (idx >= 0 && idx < ROISNUM)
    {
        rois[idx].type = type;
    }
}

void ImgLoader::setROIParams(int idx, std::vector<cv::Point2f> transformPoints)
{
    if (idx >= 0 && idx < ROISNUM)
    {
        rois[idx].transformPoints = transformPoints;
    }
}

// 加载图片
bool ImgLoader::loadImage(cv::Mat frame)
{
    if (frame.empty())
    {
        qDebug() << "Failed to capture frame from camera";
        return false;
    }

    // 获取到一帧新图片后，同时更新全部的roi区域并对所有roi区域做相应的透视变换
    m_originalImage = frame;
    updataROI();
    updataPerspectiveTransform();
    drawROIRectangles();
    return true;
}

// 获取透视变换后的ROI图片
cv::Mat ImgLoader::getImage(int idx)
{
    if (idx >= 0 && idx < ROISNUM && !rois[idx].transformedImage.empty())
    {
        return rois[idx].transformedImage;
    }
    // 默认返回空的cv::Mat
    return cv::Mat();
}


// 更新全部ROI图片
void ImgLoader::updataROI()
{
    // 根据setROIParams()中设置好的rois[idx].rect参数和当前的m_originalImage对每个rois[idx].image进行更新
    for (int i = 0; i < ROISNUM; ++i)
    {
        // 检查ROI矩形是否有效且在图像范围内
        if (rois[i].rect.width > 0 && rois[i].rect.height > 0 &&
            rois[i].rect.x >= 0 && rois[i].rect.y >= 0 &&
            rois[i].rect.x + rois[i].rect.width <= m_originalImage.cols &&
            rois[i].rect.y + rois[i].rect.height <= m_originalImage.rows)
        {
            // 从原图中提取ROI区域
            rois[i].image = m_originalImage(rois[i].rect).clone();
        }
        else
        {
            qDebug() << "Invalid ROI rectangle for index" << i;
            // 创建一个空的图像作为占位符
            rois[i].image = cv::Mat();
        }
    }
}

// 更新全部透视变换图片
void ImgLoader::updataPerspectiveTransform()
{
    // 根据setROIParams()中设置好的rois[idx].transformPoints参数和当前的rois[idx].image对每个rois[idx].image做透视变换，结果保存到rois[idx].transformedImage中
    for (int i = 0; i < ROISNUM; ++i)
    {
        // 检查源图像和变换点是否有效
        if (rois[i].image.empty() || rois[i].transformPoints.size() != 4){continue;}

        // 定义目标图像的四个点（假设我们希望将图像变换为矩形）
        std::vector<cv::Point2f> dstPoints;
        float width = rois[i].rect.width;
        float height = rois[i].rect.height;

        dstPoints.push_back(cv::Point2f(0, 0));
        dstPoints.push_back(cv::Point2f(width, 0));
        dstPoints.push_back(cv::Point2f(width, height));
        dstPoints.push_back(cv::Point2f(0, height));

        // 计算透视变换矩阵
        cv::Mat transformMatrix = cv::getPerspectiveTransform(rois[i].transformPoints, dstPoints);

        // 应用透视变换
        cv::warpPerspective(rois[i].image, rois[i].transformedImage, transformMatrix,cv::Size(width, height));
    }
}

// 绘制ROI矩形框
void ImgLoader::drawROIRectangles()
{
    // 根据rois[idx].rect参数将方框画在m_originalImage中，结果保存到m_originalWithROIsImage中
    // 复制原图
    m_originalWithROIsImage = m_originalImage.clone();

    // 定义不同ROI类型的颜色
    std::map<ROIType, cv::Scalar> colorMap = {
        {ROIType::METER, cv::Scalar(0, 0, 255)},     // 红色 - 仪表
        {ROIType::INDICATOR, cv::Scalar(0, 255, 0)}, // 绿色 - 指示灯
        {ROIType::SWITCH, cv::Scalar(255, 0, 0)}     // 蓝色 - 开关
    };

    // 绘制每个ROI的矩形框
    for (int i = 0; i < ROISNUM; ++i)
    {
        if (rois[i].rect.width > 0 && rois[i].rect.height > 0)
        {
            // 获取对应类型的颜色
            cv::Scalar color = colorMap[rois[i].type];

            // 绘制矩形框
            cv::rectangle(m_originalWithROIsImage, rois[i].rect, color, 2);

            // 添加标签文本
            std::string label;
            switch (i)
            {
            case SPEEDMETER: label = "Speed"; break;
            case TEMPMETER: label = "Temp"; break;
            case OILMETER: label = "Oil"; break;
            case VOLMETER: label = "Voltage"; break;
            case INDICATOR1: label = "Ind1"; break;
            case INDICATOR2: label = "Ind2"; break;
            case INDICATOR3: label = "Ind3"; break;
            case INDICATOR4: label = "Ind4"; break;
            case INDICATOR5: label = "Ind5"; break;
            case INDICATOR6: label = "Ind6"; break;
            case INDICATOR7: label = "Ind7"; break;
            case INDICATOR8: label = "Ind8"; break;
            case INDICATOR9: label = "Ind9"; break;
            case INDICATOR10: label = "Ind10"; break;
            case INDICATOR11: label = "Ind11"; break;
            case INDICATOR12: label = "Ind12"; break;
            case INDICATOR13: label = "Ind13"; break;
            case INDICATOR14: label = "Ind14"; break;
            case INDICATOR15: label = "Ind15"; break;
            case INDICATOR16: label = "Ind16"; break;
            case INDICATOR17: label = "Ind17"; break;
            case INDICATOR18: label = "Ind18"; break;
            case SWITCH1: label = "Switch"; break;
            default: label = "Unknown"; break;
            }

            // 在矩形框上方绘制标签
            cv::putText(m_originalWithROIsImage, label,cv::Point(rois[i].rect.x, rois[i].rect.y - 5),cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
        }
    }
}




