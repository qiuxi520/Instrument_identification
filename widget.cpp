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
}
void Widget::initializeUI()
{
    // 初始化步长设置
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

    // 设置默认值
    ui->sb_Point1x->setValue(60);
    ui->sb_Point1y->setValue(41);
    ui->sb_Point2x->setValue(621);
    ui->sb_Point2y->setValue(36);
    ui->sb_Point3x->setValue(586);
    ui->sb_Point3y->setValue(528);
    ui->sb_Point4x->setValue(55);
    ui->sb_Point4y->setValue(582);

    ui->sb_outPutWidth->setValue(612);
    ui->sb_outPutHeight->setValue(580);

    ui->sld_step->setRange(1, 10);
    ui->sld_step->setValue(m_stepValue);
    ui->sld_step->setTickInterval(2);
    ui->sld_step->setTickPosition(QSlider::TicksBelow);

    // ******************************************
    ui->dsb_simgaX->setRange(0, 4);
    ui->dsb_simgaX->setSingleStep(0.1);
    ui->dsb_simgaX->setValue(2.0);
    ui->dsb_simgaY->setRange(0, 4);
    ui->dsb_simgaY->setSingleStep(0.1);
    ui->dsb_simgaY->setValue(2.0);

    // ******************************************
    ui->sld_Threshold1->setRange(0, 255);
    ui->sld_Threshold1->setValue(50);
    ui->sld_Threshold2->setRange(0, 255);
    ui->sld_Threshold2->setValue(150);

    // ******************************************
    ui->sb_minRadius->setValue(281);
    ui->sb_maxRadius->setValue(377);

    // ******************************************
    ui->sb_rho->setRange(0, 10);
    ui->sb_rho->setSingleStep(1);
    ui->sb_rho->setValue(1);

    ui->dsb_theta->setRange(0, CV_PI*2);
    ui->dsb_theta->setSingleStep(0.1);
    ui->dsb_theta->setValue(CV_PI/180);

    ui->sb_threshold->setRange(0, 255);
    ui->sb_threshold->setSingleStep(1);
    ui->sb_threshold->setValue(30);

    ui->sb_minLineLength->setRange(0, 255);
    ui->sb_minLineLength->setSingleStep(1);
    ui->sb_minLineLength->setValue(50);

    ui->sb_maxLineGap->setRange(0, 255);
    ui->sb_maxLineGap->setSingleStep(1);
    ui->sb_maxLineGap->setValue(150);

    // ******************************************
    ui->sb_minValue->setRange(-5, 10);
    ui->sb_minValue->setSingleStep(1);
    ui->sb_minValue->setValue(0);
    ui->sb_maxValue->setRange(0, 50);
    ui->sb_maxValue->setSingleStep(1);
    ui->sb_maxValue->setValue(15);
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
        }
    }
}

// 更新显示
void Widget::updateDisplay()
{
    // 显示原始图像（带透视变换区域标记）
    cv::Mat originalWithMarkers = m_imageProcessor->getOriginalImage().clone();
    std::vector<cv::Point2f> points = {
        cv::Point2f(ui->sb_Point1x->value(), ui->sb_Point1y->value()),
        cv::Point2f(ui->sb_Point2x->value(), ui->sb_Point2y->value()),
        cv::Point2f(ui->sb_Point3x->value(), ui->sb_Point3y->value()),
        cv::Point2f(ui->sb_Point4x->value(), ui->sb_Point4y->value())
    };

    // 绘制透视变换区域
    std::vector<cv::Point> intPoints;
    for (const auto& pt : points)
    {
        intPoints.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    for (size_t i = 0; i < intPoints.size(); ++i)
    {
        cv::line(originalWithMarkers, intPoints[i], intPoints[(i + 1) % intPoints.size()],cv::Scalar(0, 255, 0), 2);
    }

    for (const auto& pt : intPoints)
    {
        cv::circle(originalWithMarkers, pt, 5, cv::Scalar(255, 0, 0), -1);
    }

    ui->pixelViewer_original->setImage(originalWithMarkers);
    ui->pixelViewer_PTtransform->setImage(m_imageProcessor->getPerspectiveTransformResult());
    ui->pixelViewer_gray->setImage(m_imageProcessor->getGrayImage());
    ui->pixelViewer_Gauss->setImage(m_imageProcessor->getBlurredImage());
    ui->pixelViewer_edge->setImage(m_imageProcessor->getEdgesImage());
    ui->pixelViewer_cricle->setImage(m_imageProcessor->getCirclesImage());

    ui->pixelViewer_line->setImage(m_imageProcessor->getLineImage());

    ui->led_Display->setText(QString("%1").arg(m_imageProcessor->getReading()));
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

// --------------------霍夫直线检测参数槽函数--------------------
void Widget::on_sb_rho_valueChanged(int arg1)
{
    m_imageProcessor->setHoughLinesParams(arg1,
                                            ui->dsb_theta->value(),
                                            ui->sb_threshold->value(),
                                            ui->sb_minLineLength->value(),
                                            ui->sb_maxLineGap->value());
}
void Widget::on_dsb_theta_valueChanged(double arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->sb_rho->value(),
                                          arg1,
                                          ui->sb_threshold->value(),
                                          ui->sb_minLineLength->value(),
                                          ui->sb_maxLineGap->value());
}
void Widget::on_sb_threshold_valueChanged(int arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->sb_rho->value(),
                                          ui->dsb_theta->value(),
                                          arg1,
                                          ui->sb_minLineLength->value(),
                                          ui->sb_maxLineGap->value());
}
void Widget::on_sb_minLineLength_valueChanged(int arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->sb_rho->value(),
                                          ui->dsb_theta->value(),
                                          ui->sb_threshold->value(),
                                          arg1,
                                          ui->sb_maxLineGap->value());
}
void Widget::on_sb_maxLineGap_valueChanged(int arg1)
{
    m_imageProcessor->setHoughLinesParams(ui->sb_rho->value(),
                                          ui->dsb_theta->value(),
                                          ui->sb_threshold->value(),
                                          ui->sb_minLineLength->value(),
                                          arg1);
}

// --------------------仪表分析参数槽函数--------------------
void Widget::on_sb_minValue_valueChanged(int arg1)
{
    m_imageProcessor->setGaugeRange(arg1,ui->sb_maxValue->value());
}
void Widget::on_sb_maxValue_valueChanged(int arg1)
{
    m_imageProcessor->setGaugeRange(ui->sb_minValue->value(),arg1);
}

