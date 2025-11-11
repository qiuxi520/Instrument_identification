#ifndef IMGLOADER_H
#define IMGLOADER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QVector>
#include <QRect>
#include <opencv2/opencv.hpp>

#define ROISNUM 23

#define SPEEDMETER 0
#define TEMPMETER 1
#define OILMETER 2
#define VOLMETER 3

#define INDICATOR1      4
#define INDICATOR2      5
#define INDICATOR3      6
#define INDICATOR4      7
#define INDICATOR5      8
#define INDICATOR6      9
#define INDICATOR7      10
#define INDICATOR8      11
#define INDICATOR9      12
#define INDICATOR10     13
#define INDICATOR11     14
#define INDICATOR12     15
#define INDICATOR13     16
#define INDICATOR14     17
#define INDICATOR15     18
#define INDICATOR16     19
#define INDICATOR17     20
#define INDICATOR18     21


#define SWITCH1 22


// ROI类型定义
enum class ROIType
{
    METER,      // 仪表
    INDICATOR,  // 指示灯
    SWITCH      // 开关
};

// ROI结构体
struct ROI
{
    ROIType type;
    cv::Rect rect;
    cv::Mat image; // ROI区域图片
    cv::Mat transformedImage;    // 透视变换后的图像
    std::vector<cv::Point2f> transformPoints; // 透视变换的四个点
};

class ImgLoader : public QObject
{
    Q_OBJECT

public:
    explicit ImgLoader(QObject *parent = nullptr);
    ~ImgLoader();

    // 设置参数
    void setROIParams(int idx,ROIType type,cv::Rect rect,std::vector<cv::Point2f> transformPoints);
    void setROIParams(int idx, cv::Rect rect);
    void setROIParams(int idx, ROIType type);
    void setROIParams(int idx, std::vector<cv::Point2f> transformPoints);

    // 加载图片
    bool loadImage(cv::Mat frame);

    // 获取透视变换后的ROI图片
    cv::Mat getImage(int idx);

    // 获取图片
    cv::Mat getOriginalImage() const { return m_originalImage; }
    cv::Mat getOriginalWithROIsImage() const { return m_originalWithROIsImage; }

    ROI rois[ROISNUM]={};
private:
    // ROI分割
    void updataROI();
    // 透视变换
    void updataPerspectiveTransform();
    // 绘制ROI矩形框
    void drawROIRectangles();

    cv::Mat m_originalImage;
    cv::Mat m_originalWithROIsImage;


};

#endif // IMGLOADER_H
