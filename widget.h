#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include "imageprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void updatePerspectivePoints();
private slots:
    void on_btn_openPic_clicked();
    void onProcessingCompleted();
    void onErrorOccurred(const QString &errorMessage);

    // ROI参数槽函数
    void on_sb_ROIx_valueChanged(int arg1);
    void on_sb_ROIy_valueChanged(int arg1);
    void on_sb_ROIwidth_valueChanged(int arg1);
    void on_sb_ROIheight_valueChanged(int arg1);

    // 透视变换参数槽函数
    void on_sb_Point1x_valueChanged(int arg1);
    void on_sb_Point1y_valueChanged(int arg1);
    void on_sb_Point2x_valueChanged(int arg1);
    void on_sb_Point2y_valueChanged(int arg1);
    void on_sb_Point3x_valueChanged(int arg1);
    void on_sb_Point3y_valueChanged(int arg1);
    void on_sb_Point4x_valueChanged(int arg1);
    void on_sb_Point4y_valueChanged(int arg1);
    void on_sb_outPutWidth_valueChanged(int arg1);
    void on_sb_outPutHeight_valueChanged(int arg1);
    void on_sld_step_valueChanged(int value);

    // 高斯模糊参数槽函数
    void on_dsb_simgaX_valueChanged(double arg1);
    void on_dsb_simgaY_valueChanged(double arg1);

    // 边缘检测参数槽函数
    void on_sld_Threshold1_valueChanged(int value);
    void on_sld_Threshold2_valueChanged(int value);

    // 霍夫圆检测参数槽函数
    void on_sb_minRadius_valueChanged(int arg1);
    void on_sb_maxRadius_valueChanged(int arg1);

    // 掩膜参数槽函数
    void on_sb_mask_valueChanged(int arg1);

    // 二值化参数槽函数
    void on_dsb_globalThreshold_valueChanged(double arg1);
    void on_dsb_globalmaxValue_valueChanged(double arg1);

    // 形态学参数槽函数
    void on_sb_openKernelSize_valueChanged(int arg1);
    void on_sb_closeKernelSize_valueChanged(int arg1);
    void on_sb_morphologyIterations_valueChanged(int arg1);

    // 腐蚀参数槽函数
    void on_sb_kernelSize_valueChanged(int arg1);
    void on_sb_erosionIterations_valueChanged(int arg1);

    // 霍夫直线检测参数槽函数
    void on_dsb_rho_valueChanged(double arg1);
    void on_dsb_theta_valueChanged(double arg1);
    void on_sb_threshold_valueChanged(int arg1);
    void on_dsb_minLineLength_valueChanged(double arg1);
    void on_dsb_maxLineGap_valueChanged(double arg1);

    // 仪表参数槽函数
    void on_sb_MinValueAngle_valueChanged(int arg1);
    void on_sb_MaxValueAngle_valueChanged(int arg1);
    void on_sb_minValue_valueChanged(int arg1);
    void on_sb_maxValue_valueChanged(int arg1);


private:
    void initializeUI();
    void setupConnections();
    void updateSpinBoxRanges();
    void updateDisplay();

private:
    Ui::Widget *ui;
    ImageProcessor *m_imageProcessor;
    int m_stepValue;
};
#endif // WIDGET_H
