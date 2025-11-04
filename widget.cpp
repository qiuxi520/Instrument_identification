#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_imageProcessor(new ImageProcessor(this))
    , m_stepValue(5)
{
    ui->setupUi(this);
    setWindowTitle("仪表识别");
    setupConnections();

    this->setAutoFillBackground(true);
    QPixmap pixMap(":/img/bg.jpg");
    QPalette backPalette;
    backPalette.setBrush(this->backgroundRole(), QBrush(pixMap));
    this->setPalette(backPalette);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::updateSpinBoxRanges()
{
    int width = m_imageProcessor->getImageWidth();
    int height = m_imageProcessor->getImageHeight();

    ui->sb_Point1x->setRange(0, width);
    ui->sb_Point2x->setRange(0, width);
    ui->sb_Point3x->setRange(0, width);
    ui->sb_Point4x->setRange(0, width);
    ui->sb_Point1y->setRange(0, height);
    ui->sb_Point2y->setRange(0, height);
    ui->sb_Point3y->setRange(0, height);
    ui->sb_Point4y->setRange(0, height);

    ui->sb_outPutWidth->setRange(0, width);
    ui->sb_outPutHeight->setRange(0, height);

    ui->sb_minRadius->setRange(0, qMax(width, height));
    ui->sb_maxRadius->setRange(0, qMax(width, height));


    ui->sb_ROIx->setRange(0, width);;
    ui->sb_ROIy->setRange(0, height);
    ui->sb_ROIwidth->setRange(0, width);;
    ui->sb_ROIheight->setRange(0, height);
}
void Widget::initializeUI()
{
    // 禁用多个控件的信号
    QList<QWidget*> widgets = {
        // 透视变换点坐标和输出尺寸
        ui->sb_Point1x, ui->sb_Point1y, ui->sb_Point2x, ui->sb_Point2y,
        ui->sb_Point3x, ui->sb_Point3y, ui->sb_Point4x, ui->sb_Point4y,
        ui->sb_outPutWidth, ui->sb_outPutHeight,

        // 步长滑块
        ui->sld_step,

        // 高斯滤波参数
        ui->dsb_simgaX, ui->dsb_simgaY,

        // Canny阈值参数
        ui->sld_Threshold1, ui->sld_Threshold2,

        // 霍夫圆检测参数
        ui->sb_minRadius, ui->sb_maxRadius,

        // 霍夫线检测参数
        ui->dsb_rho, ui->dsb_theta, ui->sb_threshold,
        ui->dsb_minLineLength, ui->dsb_maxLineGap,

        // 其他参数
        ui->sb_minValue, ui->sb_maxValue,

        // ROI参数
        ui->sb_ROIx, ui->sb_ROIy, ui->sb_ROIwidth, ui->sb_ROIheight,

        // 掩膜参数
        ui->sb_mask,

        // 全局阈值参数
        ui->dsb_globalThreshold, ui->dsb_globalmaxValue,

        // 腐蚀参数
        ui->sb_kernelSize, ui->sb_erosionIterations,

        // 形态学参数
        ui->sb_openKernelSize,ui->sb_closeKernelSize,ui->sb_morphologyIterations,

        // 仪表
        ui->sb_MinValueAngle,ui->sb_MaxValueAngle,ui->sb_minValue,ui->sb_maxValue
    };

    // 阻塞所有信号
    for (QWidget* widget : widgets) {
        if (QObject* obj = qobject_cast<QObject*>(widget)) {
            obj->blockSignals(true);
        }
    }


// --------------------ROI参数槽函数--------------------
    ui->sb_ROIx->setValue(3200);
    ui->sb_ROIy->setValue(1500);
    ui->sb_ROIwidth->setValue(750);
    ui->sb_ROIheight->setValue(750);
    m_imageProcessor->setROIParams(ui->sb_ROIx->value(),ui->sb_ROIy->value(),ui->sb_ROIwidth->value(),ui->sb_ROIheight->value());


// --------------------透视变换--------------------
    // 初始化步长设置
    QList<QSpinBox *> perspectiveWidgets = {
        ui->sb_Point1x, ui->sb_Point1y, ui->sb_Point2x, ui->sb_Point2y,
        ui->sb_Point3x, ui->sb_Point3y, ui->sb_Point4x, ui->sb_Point4y,
        ui->sb_outPutWidth, ui->sb_outPutHeight
    };
    for (auto* spinBox : perspectiveWidgets) {spinBox->setSingleStep(m_stepValue);}

    // 设置默认值
    ui->sb_Point1x->setValue(88);
    ui->sb_Point1y->setValue(70);
    ui->sb_Point2x->setValue(730);
    ui->sb_Point2y->setValue(80);
    ui->sb_Point3x->setValue(699);
    ui->sb_Point3y->setValue(690);
    ui->sb_Point4x->setValue(17);
    ui->sb_Point4y->setValue(700);
    ui->sb_outPutWidth->setValue(476);
    ui->sb_outPutHeight->setValue(455);

    updatePerspectivePoints();
    m_imageProcessor->setOutputSize(ui->sb_outPutWidth->value(), ui->sb_outPutHeight->value());

    ui->sld_step->setRange(1, 10);
    ui->sld_step->setValue(5);
    ui->sld_step->setTickInterval(2);
    ui->sld_step->setTickPosition(QSlider::TicksBelow);


// --------------------高斯模糊--------------------
    ui->dsb_simgaX->setRange(0, 4);
    ui->dsb_simgaX->setSingleStep(0.1);
    ui->dsb_simgaX->setValue(2.0);
    ui->dsb_simgaY->setRange(0, 4);
    ui->dsb_simgaY->setSingleStep(0.1);
    ui->dsb_simgaY->setValue(2.0);
    m_imageProcessor->setGaussianSigma(ui->dsb_simgaX->value(), ui->dsb_simgaY->value());


// --------------------边缘检测--------------------
    ui->sld_Threshold1->setRange(0, 255);
    ui->sld_Threshold1->setValue(50);
    ui->sld_Threshold2->setRange(0, 255);
    ui->sld_Threshold2->setValue(150);

    m_imageProcessor->setCannyThresholds(ui->sld_Threshold1->value(), ui->sld_Threshold2->value());


// --------------------霍夫圆检测--------------------
    ui->sb_minRadius->setValue(190);
    ui->sb_maxRadius->setValue(220);
    m_imageProcessor->setHoughCirclesParams(ui->sb_minRadius->value(), ui->sb_maxRadius->value());


// --------------------掩膜参数--------------------
    ui->sb_mask->setRange(50, 500);
    ui->sb_mask->setSingleStep(10);
    ui->sb_mask->setValue(90);
    m_imageProcessor->setmaskROIParams(ui->sb_mask->value());


// --------------------二值化参数--------------------
    ui->dsb_globalThreshold->setRange(0, 255);
    ui->dsb_globalThreshold->setSingleStep(1);
    ui->dsb_globalThreshold->setValue(190);

    ui->dsb_globalmaxValue->setRange(0, 255);
    ui->dsb_globalmaxValue->setSingleStep(1);
    ui->dsb_globalmaxValue->setValue(255);

    m_imageProcessor->setGlobalThresholdParams(ui->dsb_globalThreshold->value(), ui->dsb_globalmaxValue->value());


// --------------------腐蚀参数--------------------
    ui->sb_kernelSize->setRange(1, 21);
    ui->sb_kernelSize->setSingleStep(2);
    ui->sb_kernelSize->setValue(3);

    ui->sb_erosionIterations->setRange(1, 10);
    ui->sb_erosionIterations->setSingleStep(1);
    ui->sb_erosionIterations->setValue(2);

    m_imageProcessor->setErosionParams(ui->sb_kernelSize->value(), ui->sb_erosionIterations->value());


// --------------------形态学参数--------------------
    ui->sb_openKernelSize->setRange(1, 21);
    ui->sb_openKernelSize->setSingleStep(2);
    ui->sb_openKernelSize->setValue(1);

    ui->sb_closeKernelSize->setRange(1, 21);
    ui->sb_closeKernelSize->setSingleStep(2);
    ui->sb_closeKernelSize->setValue(3);

    ui->sb_morphologyIterations->setRange(1, 10);
    ui->sb_morphologyIterations->setSingleStep(1);
    ui->sb_morphologyIterations->setValue(3);

    m_imageProcessor->setMorphologyOperationParams(ui->sb_openKernelSize->value(), ui->sb_closeKernelSize->value(), 0, ui->sb_morphologyIterations->value());


// --------------------霍夫直线检测参数--------------------
    ui->dsb_rho->setRange(1, 10);
    ui->dsb_rho->setSingleStep(1);
    ui->dsb_rho->setValue(1);

    ui->dsb_theta->setRange(0, CV_PI*2);
    ui->dsb_theta->setSingleStep(0.1);
    ui->dsb_theta->setValue(CV_PI/180);

    ui->sb_threshold->setRange(0, 255);
    ui->sb_threshold->setSingleStep(1);
    ui->sb_threshold->setValue(30);

    ui->dsb_minLineLength->setRange(0, 255);
    ui->dsb_minLineLength->setSingleStep(1);
    ui->dsb_minLineLength->setValue(50);

    ui->dsb_maxLineGap->setRange(0, 255);
    ui->dsb_maxLineGap->setSingleStep(1);
    ui->dsb_maxLineGap->setValue(150);

    m_imageProcessor->setHoughLinesParams(ui->dsb_rho->value(),
                                          ui->dsb_theta->value(),
                                          ui->sb_threshold->value(),
                                          ui->dsb_minLineLength->value(),
                                          ui->dsb_maxLineGap->value());

// --------------------仪表参数--------------------
    ui->sb_MinValueAngle->setRange(-360, 360);
    ui->sb_MinValueAngle->setSingleStep(1);
    ui->sb_MinValueAngle->setValue(-10);

    ui->sb_MaxValueAngle->setRange(-360, 360);
    ui->sb_MaxValueAngle->setSingleStep(1);
    ui->sb_MaxValueAngle->setValue(350);


    ui->sb_minValue->setRange(-5, 10);
    ui->sb_minValue->setSingleStep(1);
    ui->sb_minValue->setValue(-1);
    ui->sb_maxValue->setRange(0, 50);
    ui->sb_maxValue->setSingleStep(1);
    ui->sb_maxValue->setValue(15);

    m_imageProcessor->setGaugeParams(ui->sb_MinValueAngle->value(),ui->sb_MaxValueAngle->value(),ui->sb_minValue->value(),ui->sb_maxValue->value());

    // 重新启用所有信号
    for (QWidget* widget : widgets) {
        if (QObject* obj = qobject_cast<QObject*>(widget)) {
            obj->blockSignals(false);
        }
    }

}

void Widget::setupConnections()
{
    connect(m_imageProcessor, &ImageProcessor::processingCompleted,this, &Widget::onProcessingCompleted);
    connect(m_imageProcessor, &ImageProcessor::errorOccurred,this, &Widget::onErrorOccurred);
}
void Widget::onProcessingCompleted()
{
    updateDisplay();
}
void Widget::onErrorOccurred(const QString &errorMessage)
{
    QMessageBox::critical(this, "错误", errorMessage);
}

// 打开图像
void Widget::on_btn_openPic_clicked()
{
    QString fileDir = "..\\..\\pic";
    QString fileName = QFileDialog::getOpenFileName(this, "打开图像", fileDir,"图像文件 (*.png *.jpg *.jpeg *.bmp *.tiff *.tif)");

    if (!fileName.isEmpty())
    {

        if (m_imageProcessor->loadImage(fileName))
        {
            updateSpinBoxRanges();
            initializeUI();
            // qDebug() << "initializeUI size:";
        }
    }
}

// 更新显示
void Widget::updateDisplay()
{
    ui->pixelViewer_1->setImage(m_imageProcessor->getOriginalWithROIImage());
    ui->pixelViewer_2->setImage(m_imageProcessor->getROIImage());
    ui->pixelViewer_3->setImage(m_imageProcessor->getPerspectiveTransformRange());
    ui->pixelViewer_4->setImage(m_imageProcessor->getPerspectiveTransformResult());
    ui->pixelViewer_5->setImage(m_imageProcessor->getGrayImage());
    ui->pixelViewer_6->setImage(m_imageProcessor->getBlurredImage());
    ui->pixelViewer_7->setImage(m_imageProcessor->getEdgesImage());
    ui->pixelViewer_8->setImage(m_imageProcessor->getCirclesImage());
    ui->pixelViewer_9->setImage(m_imageProcessor->getMaskROIImage());
    ui->pixelViewer_10->setImage(m_imageProcessor->getThresholdImage());
    ui->pixelViewer_11->setImage(m_imageProcessor->getErosionImage());
    ui->pixelViewer_12->setImage(m_imageProcessor->getmorphologyOperationImage());
    ui->pixelViewer_13->setImage(m_imageProcessor->getLineImage());
    ui->pixelViewer_14->setImage(m_imageProcessor->getResultImage());

    ui->led_Display->setText(QString("%1").arg(m_imageProcessor->getReading()));
    ui->led_angle->setText(QString("%1").arg(m_imageProcessor->getAngle()));
}


// --------------------ROI参数槽函数--------------------
void Widget::on_sb_ROIx_valueChanged(int arg1)
{
    m_imageProcessor->setROIParams(arg1,ui->sb_ROIy->value(),ui->sb_ROIwidth->value(),ui->sb_ROIheight->value());
}
void Widget::on_sb_ROIy_valueChanged(int arg1)
{
    m_imageProcessor->setROIParams(ui->sb_ROIx->value(),arg1,ui->sb_ROIwidth->value(),ui->sb_ROIheight->value());
}
void Widget::on_sb_ROIwidth_valueChanged(int arg1)
{
    m_imageProcessor->setROIParams(ui->sb_ROIx->value(),ui->sb_ROIy->value(),arg1,ui->sb_ROIheight->value());
}
void Widget::on_sb_ROIheight_valueChanged(int arg1)
{
    m_imageProcessor->setROIParams(ui->sb_ROIx->value(),ui->sb_ROIy->value(),ui->sb_ROIwidth->value(),arg1);
}

// --------------------透视变换参数槽函数--------------------
void Widget::on_sb_Point1x_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::on_sb_Point1y_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::on_sb_Point2x_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::on_sb_Point2y_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::on_sb_Point3x_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::on_sb_Point3y_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::on_sb_Point4x_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::on_sb_Point4y_valueChanged(int arg1) { updatePerspectivePoints(); }
void Widget::updatePerspectivePoints()
{
    std::vector<cv::Point2f> points = {
        cv::Point2f(ui->sb_Point1x->value(), ui->sb_Point1y->value()),
        cv::Point2f(ui->sb_Point2x->value(), ui->sb_Point2y->value()),
        cv::Point2f(ui->sb_Point3x->value(), ui->sb_Point3y->value()),
        cv::Point2f(ui->sb_Point4x->value(), ui->sb_Point4y->value())
    };
    m_imageProcessor->setPerspectivePoints(points);
}

void Widget::on_sb_outPutWidth_valueChanged(int arg1)
{
    m_imageProcessor->setOutputSize(arg1, ui->sb_outPutHeight->value());
}
void Widget::on_sb_outPutHeight_valueChanged(int arg1)
{
    m_imageProcessor->setOutputSize(ui->sb_outPutWidth->value(), arg1);
}
void Widget::on_sld_step_valueChanged(int value)
{
    m_stepValue = value;
    ui->sb_Point1x->setSingleStep(m_stepValue);
    ui->sb_Point2x->setSingleStep(m_stepValue);
    ui->sb_Point3x->setSingleStep(m_stepValue);
    ui->sb_Point4x->setSingleStep(m_stepValue);
    ui->sb_Point1y->setSingleStep(m_stepValue);
    ui->sb_Point2y->setSingleStep(m_stepValue);
    ui->sb_Point3y->setSingleStep(m_stepValue);
    ui->sb_Point4y->setSingleStep(m_stepValue);
    ui->sb_outPutWidth->setSingleStep(m_stepValue);
    ui->sb_outPutHeight->setSingleStep(m_stepValue);
}

// --------------------高斯模糊参数槽函数--------------------
void Widget::on_dsb_simgaX_valueChanged(double arg1)
{
    m_imageProcessor->setGaussianSigma(arg1, ui->dsb_simgaY->value());
}
void Widget::on_dsb_simgaY_valueChanged(double arg1)
{
    m_imageProcessor->setGaussianSigma(ui->dsb_simgaX->value(), arg1);
}

// --------------------边缘检测参数槽函数--------------------
void Widget::on_sld_Threshold1_valueChanged(int value)
{
    ui->led_Threshold1->setText(QString::number(value));
    m_imageProcessor->setCannyThresholds(value, ui->sld_Threshold2->value());
}
void Widget::on_sld_Threshold2_valueChanged(int value)
{
    ui->led_Threshold2->setText(QString::number(value));
    m_imageProcessor->setCannyThresholds(ui->sld_Threshold1->value(), value);
}

// --------------------霍夫圆检测参数槽函数--------------------
void Widget::on_sb_minRadius_valueChanged(int arg1)
{
    m_imageProcessor->setHoughCirclesParams(arg1, ui->sb_maxRadius->value());
}
void Widget::on_sb_maxRadius_valueChanged(int arg1)
{
    m_imageProcessor->setHoughCirclesParams(ui->sb_minRadius->value(), arg1);
}

// --------------------掩膜参数槽函数--------------------
void Widget::on_sb_mask_valueChanged(int arg1)
{
    m_imageProcessor->setmaskROIParams(arg1);
}

// --------------------二值化参数槽函数--------------------
void Widget::on_dsb_globalThreshold_valueChanged(double arg1)
{
    m_imageProcessor->setGlobalThresholdParams(arg1, ui->dsb_globalmaxValue->value());
}
void Widget::on_dsb_globalmaxValue_valueChanged(double arg1)
{
    m_imageProcessor->setGlobalThresholdParams(ui->dsb_globalThreshold->value(), arg1);
}


// --------------------腐蚀参数槽函数--------------------
void Widget::on_sb_kernelSize_valueChanged(int arg1)
{
    m_imageProcessor->setErosionParams(arg1,ui->sb_erosionIterations->value());
}
void Widget::on_sb_erosionIterations_valueChanged(int arg1)
{
    m_imageProcessor->setErosionParams(ui->sb_kernelSize->value(),arg1);
}

// --------------------形态学参数槽函数--------------------
void Widget::on_sb_openKernelSize_valueChanged(int arg1)
{
    m_imageProcessor->setMorphologyOperationParams(arg1, ui->sb_closeKernelSize->value(), 0, ui->sb_morphologyIterations->value());
}
void Widget::on_sb_closeKernelSize_valueChanged(int arg1)
{
    m_imageProcessor->setMorphologyOperationParams(ui->sb_openKernelSize->value(), arg1, 0, ui->sb_morphologyIterations->value());
}
void Widget::on_sb_morphologyIterations_valueChanged(int arg1)
{
    m_imageProcessor->setMorphologyOperationParams(ui->sb_openKernelSize->value(), ui->sb_closeKernelSize->value(), 0, arg1);
}

// --------------------霍夫直线检测参数槽函数--------------------
void Widget::on_dsb_rho_valueChanged(double arg1)
{
    m_imageProcessor->setHoughLinesParams(arg1,
                                          ui->dsb_theta->value(),
                                          ui->sb_threshold->value(),
                                          ui->dsb_minLineLength->value(),
                                          ui->dsb_maxLineGap->value());
}
void Widget::on_dsb_theta_valueChanged(double arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->dsb_rho->value(),
                                          arg1,
                                          ui->sb_threshold->value(),
                                          ui->dsb_minLineLength->value(),
                                          ui->dsb_maxLineGap->value());
}
void Widget::on_sb_threshold_valueChanged(int arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->dsb_rho->value(),
                                          ui->dsb_theta->value(),
                                          arg1,
                                          ui->dsb_minLineLength->value(),
                                          ui->dsb_maxLineGap->value());
}
void Widget::on_dsb_minLineLength_valueChanged(double arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->dsb_rho->value(),
                                          ui->dsb_theta->value(),
                                          ui->sb_threshold->value(),
                                          arg1,
                                          ui->dsb_maxLineGap->value());
}
void Widget::on_dsb_maxLineGap_valueChanged(double arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->dsb_rho->value(),
                                          ui->dsb_theta->value(),
                                          ui->sb_threshold->value(),
                                          ui->dsb_minLineLength->value(),
                                          arg1);
}


// --------------------仪表分析参数槽函数--------------------
void Widget::on_sb_MinValueAngle_valueChanged(int arg1)
{
    m_imageProcessor->setGaugeParams(arg1,ui->sb_MaxValueAngle->value(),ui->sb_minValue->value(),ui->sb_maxValue->value());
}
void Widget::on_sb_MaxValueAngle_valueChanged(int arg1)
{
    m_imageProcessor->setGaugeParams(ui->sb_MinValueAngle->value(),arg1,ui->sb_minValue->value(),ui->sb_maxValue->value());
}
void Widget::on_sb_minValue_valueChanged(int arg1)
{
    m_imageProcessor->setGaugeParams(ui->sb_MinValueAngle->value(),ui->sb_MaxValueAngle->value(),arg1,ui->sb_maxValue->value());
}
void Widget::on_sb_maxValue_valueChanged(int arg1)
{
    m_imageProcessor->setGaugeParams(ui->sb_MinValueAngle->value(),ui->sb_MaxValueAngle->value(),ui->sb_minValue->value(),arg1);
}

