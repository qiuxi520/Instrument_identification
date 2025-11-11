#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_imageProcessor(new ImageProcessor(this))
    , ImgLoader(new class ImgLoader(this))
{
    ui->setupUi(this);

    setWindowTitle("仪表识别");
    setupConnections();

    this->setAutoFillBackground(true);
    QPixmap pixMap(":/img/bg.jpg");
    QPalette backPalette;
    backPalette.setBrush(this->backgroundRole(), QBrush(pixMap));
    this->setPalette(backPalette);

    // 打开默认摄像头（摄像头索引通常为0）
    cap.open(0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::getFrame);
    timer->start(50);  // 每50ms触发一次


    spinBoxInit();
    ui->cmb_ROIslt->addItem("转速表");
    ui->cmb_ROIslt->addItem("温度表");
    ui->cmb_ROIslt->addItem("燃油表");
    ui->cmb_ROIslt->addItem("电压表");
    ui->cmb_ROIslt->addItem("灯1");
    ui->cmb_ROIslt->addItem("灯2");
    ui->cmb_ROIslt->addItem("灯3");
    ui->cmb_ROIslt->addItem("灯4");
    ui->cmb_ROIslt->addItem("灯5");
    ui->cmb_ROIslt->addItem("灯6");
    ui->cmb_ROIslt->addItem("灯7");
    ui->cmb_ROIslt->addItem("灯8");
    ui->cmb_ROIslt->addItem("灯9");
    ui->cmb_ROIslt->addItem("灯10");
    ui->cmb_ROIslt->addItem("灯11");
    ui->cmb_ROIslt->addItem("灯12");
    ui->cmb_ROIslt->addItem("灯13");
    ui->cmb_ROIslt->addItem("灯14");
    ui->cmb_ROIslt->addItem("灯15");
    ui->cmb_ROIslt->addItem("灯16");
    ui->cmb_ROIslt->addItem("灯17");
    ui->cmb_ROIslt->addItem("灯18");
    ui->cmb_ROIslt->addItem("开关");

    imgLoaderInit();

    qDebug()<<path;
}

Widget::~Widget()
{
    delete ui;
    // 释放资源
    cap.release();
    cv::destroyAllWindows();
}

void Widget::imgLoaderInit()
{
    for (int i = 0; i < 4; i++) ImgLoader->setROIParams(i,ROIType::METER);
    for (int i = 4; i < 22; i++) ImgLoader->setROIParams(i,ROIType::INDICATOR);
    for (int i = 22; i < 23; i++) ImgLoader->setROIParams(i,ROIType::SWITCH);


    for (int i = 0; i < ROISNUM; i++)
        ImgLoader->setROIParams(i,cv::Rect(getParamFromIni(i)[0], getParamFromIni(i)[1], getParamFromIni(i)[2], getParamFromIni(i)[3]));

    // 默认不做透视变换，指示灯识别不需要透视变换
    for (int i = 0; i < ROISNUM; i++)
    {
        // 在图像上显示原始角点
        std::vector<cv::Point2f> originalCorners = {
            cv::Point2f(0, 0),
            cv::Point2f(ImgLoader->rois[i].rect.width, 0),
            cv::Point2f(ImgLoader->rois[i].rect.width, ImgLoader->rois[i].rect.height),
            cv::Point2f(0, ImgLoader->rois[i].rect.height)
        };
        ImgLoader->setROIParams(i,originalCorners);
    }

}
void Widget::spinBoxInit()
{
    ui->sb_step->setRange(1, 20);
    ui->sb_step->setSingleStep(1);
    ui->sb_step->setValue(1);

    ui->sb_ROIx->setRange(1, 640);
    ui->sb_ROIy->setRange(1, 480);
    ui->sb_ROIwidth->setRange(1, 640);
    ui->sb_ROIheight->setRange(1, 480);
}

void Widget::getFrame()
{
    // 读取一帧
    cap >> frame;

    // 检查帧是否为空
    if (frame.empty())
    {
        qDebug() << "无法获取帧";
    }
    ImgLoader->loadImage(frame);

    //--------------------------处理逻辑--------------------------


    // ImgLoader->getImage(SPEEDMETER);
    // ImgLoader->getImage(TEMPMETER);
    // ImgLoader->getImage(OILMETER);
    // ImgLoader->getImage(VOLMETER);


    // ImgLoader->getImage(INDICATOR1);
    // ImgLoader->getImage(INDICATOR2);
    // ImgLoader->getImage(INDICATOR3);
    // ImgLoader->getImage(INDICATOR4);
    // ImgLoader->getImage(INDICATOR5);
    // ImgLoader->getImage(INDICATOR6);


    // ImgLoader->getImage(SWITCH1);

    //--------------------------处理逻辑--------------------------
    updateDisplay();

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

        }
    }
}

// 更新显示
void Widget::updateDisplay()
{
    ui->pixelViewer_1->setImage(ImgLoader->getOriginalWithROIsImage());
    ui->pixelViewer_2->setImage(ImgLoader->getImage(SPEEDMETER));
    ui->pixelViewer_3->setImage(ImgLoader->getImage(TEMPMETER));
    ui->pixelViewer_4->setImage(ImgLoader->getImage(OILMETER));
    ui->pixelViewer_5->setImage(ImgLoader->getImage(VOLMETER));
    ui->pixelViewer_6->setImage(ImgLoader->getImage(INDICATOR1));
    ui->pixelViewer_7->setImage(ImgLoader->getImage(INDICATOR2));
    ui->pixelViewer_8->setImage(ImgLoader->getImage(INDICATOR3));
    ui->pixelViewer_9->setImage(ImgLoader->getImage(INDICATOR4));
    ui->pixelViewer_10->setImage(ImgLoader->getImage(INDICATOR5));
    ui->pixelViewer_11->setImage(ImgLoader->getImage(INDICATOR6));
    ui->pixelViewer_12->setImage(ImgLoader->getImage(SWITCH1));

    // ui->pixelViewer_1->setImage(m_imageProcessor->getOriginalImage());
    // ui->pixelViewer_2->setImage(m_imageProcessor->getROIImage());
    // ui->pixelViewer_3->setImage(m_imageProcessor->getPerspectiveTransformRange());
    // ui->pixelViewer_4->setImage(m_imageProcessor->getPerspectiveTransformResult());
    // ui->pixelViewer_5->setImage(m_imageProcessor->getGrayImage());
    // ui->pixelViewer_6->setImage(m_imageProcessor->getBlurredImage());
    // ui->pixelViewer_7->setImage(m_imageProcessor->getEdgesImage());
    // ui->pixelViewer_8->setImage(m_imageProcessor->getCirclesImage());
    // ui->pixelViewer_9->setImage(m_imageProcessor->getMaskROIImage());
    // ui->pixelViewer_10->setImage(m_imageProcessor->getThresholdImage());
    // ui->pixelViewer_11->setImage(m_imageProcessor->getErosionImage());
    // ui->pixelViewer_12->setImage(m_imageProcessor->getmorphologyOperationImage());
}

void Widget::on_cmb_ROIslt_currentIndexChanged(int index)
{
    //读取ini，设置值
    QVector<int> roiParams;
    roiParams=getParamFromIni(index);

    ui->sb_ROIx->setValue(roiParams[0]);
    ui->sb_ROIy->setValue(roiParams[1]);
    ui->sb_ROIwidth->setValue(roiParams[2]);
    ui->sb_ROIheight->setValue(roiParams[3]);
}
void Widget::on_sb_step_valueChanged(int arg1)
{
    ui->sb_ROIx->setSingleStep(arg1);
    ui->sb_ROIy->setSingleStep(arg1);
    ui->sb_ROIwidth->setSingleStep(arg1);
    ui->sb_ROIheight->setSingleStep(arg1);
}
void Widget::on_sb_ROIx_valueChanged(int arg1)
{
    int currentIndex = ui->cmb_ROIslt->currentIndex();
    ImgLoader->setROIParams(currentIndex,cv::Rect(arg1, ui->sb_ROIy->value(), ui->sb_ROIwidth->value(), ui->sb_ROIheight->value()));
    //写入ini
    saveParam2Ini(currentIndex,"ROIx",QString::number(arg1));
}
void Widget::on_sb_ROIy_valueChanged(int arg1)
{
    int currentIndex = ui->cmb_ROIslt->currentIndex();
    ImgLoader->setROIParams(currentIndex,cv::Rect(ui->sb_ROIx->value(), arg1, ui->sb_ROIwidth->value(), ui->sb_ROIheight->value()));
    saveParam2Ini(currentIndex,"ROIy",QString::number(arg1));
}
void Widget::on_sb_ROIwidth_valueChanged(int arg1)
{
    int currentIndex = ui->cmb_ROIslt->currentIndex();
    ImgLoader->setROIParams(currentIndex,cv::Rect(ui->sb_ROIx->value(), ui->sb_ROIy->value(), arg1, ui->sb_ROIheight->value()));
    saveParam2Ini(currentIndex,"ROIwidth",QString::number(arg1));
}
void Widget::on_sb_ROIheight_valueChanged(int arg1)
{
    int currentIndex = ui->cmb_ROIslt->currentIndex();
    ImgLoader->setROIParams(currentIndex,cv::Rect(ui->sb_ROIx->value(), ui->sb_ROIy->value(), ui->sb_ROIwidth->value(), arg1));
    saveParam2Ini(currentIndex,"ROIheight",QString::number(arg1));
}

void Widget::saveParam2Ini(int idx,QString paramName,QString paramVal)
{
    QString group;
    switch (idx)
    {
    case SPEEDMETER: group = "Speed"; break;
    case TEMPMETER: group = "Temp"; break;
    case OILMETER: group = "Oil"; break;
    case VOLMETER: group = "Voltage"; break;
    case INDICATOR1: group = "Ind1"; break;
    case INDICATOR2: group = "Ind2"; break;
    case INDICATOR3: group = "Ind3"; break;
    case INDICATOR4: group = "Ind4"; break;
    case INDICATOR5: group = "Ind5"; break;
    case INDICATOR6: group = "Ind6"; break;
    case INDICATOR7: group = "Ind7"; break;
    case INDICATOR8: group = "Ind8"; break;
    case INDICATOR9: group = "Ind9"; break;
    case INDICATOR10: group = "Ind10"; break;
    case INDICATOR11: group = "Ind11"; break;
    case INDICATOR12: group = "Ind12"; break;
    case INDICATOR13: group = "Ind13"; break;
    case INDICATOR14: group = "Ind14"; break;
    case INDICATOR15: group = "Ind15"; break;
    case INDICATOR16: group = "Ind16"; break;
    case INDICATOR17: group = "Ind17"; break;
    case INDICATOR18: group = "Ind18"; break;
    case SWITCH1: group = "Switch"; break;
    default: group = "Unknown"; break;
    }

    m_iniFile->beginGroup(group);
    m_iniFile->setValue(paramName, paramVal);
    m_iniFile->endGroup();
}
QVector<int> Widget::getParamFromIni(int idx)
{
    QString group;
    switch (idx)
    {
    case SPEEDMETER: group = "Speed"; break;
    case TEMPMETER: group = "Temp"; break;
    case OILMETER: group = "Oil"; break;
    case VOLMETER: group = "Voltage"; break;
    case INDICATOR1: group = "Ind1"; break;
    case INDICATOR2: group = "Ind2"; break;
    case INDICATOR3: group = "Ind3"; break;
    case INDICATOR4: group = "Ind4"; break;
    case INDICATOR5: group = "Ind5"; break;
    case INDICATOR6: group = "Ind6"; break;
    case INDICATOR7: group = "Ind7"; break;
    case INDICATOR8: group = "Ind8"; break;
    case INDICATOR9: group = "Ind9"; break;
    case INDICATOR10: group = "Ind10"; break;
    case INDICATOR11: group = "Ind11"; break;
    case INDICATOR12: group = "Ind12"; break;
    case INDICATOR13: group = "Ind13"; break;
    case INDICATOR14: group = "Ind14"; break;
    case INDICATOR15: group = "Ind15"; break;
    case INDICATOR16: group = "Ind16"; break;
    case INDICATOR17: group = "Ind17"; break;
    case INDICATOR18: group = "Ind18"; break;
    case SWITCH1: group = "Switch"; break;
    default: group = "Unknown"; break;
    }

    m_iniFile->beginGroup(group);
    QVector<int> roiParams;
    roiParams.append(m_iniFile->value("ROIx", "0").toInt());
    roiParams.append(m_iniFile->value("ROIy", "0").toInt());
    roiParams.append(m_iniFile->value("ROIwidth", "200").toInt());
    roiParams.append(m_iniFile->value("ROIheight", "200").toInt());

    m_iniFile->endGroup();
    return roiParams;
}


