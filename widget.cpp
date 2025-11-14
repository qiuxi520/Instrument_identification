#include "widget.h"
#include <QList>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , ImgLoader(new class ImgLoader(this))
{
    ui->setupUi(this);

    setWindowTitle("仪表识别");

    this->setAutoFillBackground(true);
    QPixmap pixMap(":/img/bg.jpg");
    QPalette backPalette;
    backPalette.setBrush(this->backgroundRole(), QBrush(pixMap));
    this->setPalette(backPalette);

    // 打开默认摄像头（摄像头索引通常为0）
    cap.open(0);

    fileImg=cv::imread("D:/PhD/8-aircraft/code/saveFrame/on.jpg");

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


    ui->cmb_streamSlt->addItem("转速表");
    ui->cmb_streamSlt->addItem("温度表");
    ui->cmb_streamSlt->addItem("燃油表");
    ui->cmb_streamSlt->addItem("电压表");
    ui->cmb_streamSlt->addItem("灯1");
    ui->cmb_streamSlt->addItem("灯2");
    ui->cmb_streamSlt->addItem("灯3");
    ui->cmb_streamSlt->addItem("灯4");
    ui->cmb_streamSlt->addItem("灯5");
    ui->cmb_streamSlt->addItem("灯6");
    ui->cmb_streamSlt->addItem("灯7");
    ui->cmb_streamSlt->addItem("灯8");
    ui->cmb_streamSlt->addItem("灯9");
    ui->cmb_streamSlt->addItem("灯10");
    ui->cmb_streamSlt->addItem("灯11");
    ui->cmb_streamSlt->addItem("灯12");
    ui->cmb_streamSlt->addItem("灯13");
    ui->cmb_streamSlt->addItem("灯14");
    ui->cmb_streamSlt->addItem("灯15");
    ui->cmb_streamSlt->addItem("灯16");
    ui->cmb_streamSlt->addItem("灯17");
    ui->cmb_streamSlt->addItem("灯18");
    ui->cmb_streamSlt->addItem("开关");

    ui->cmb_maskSlt->addItem("ROI");
    ui->cmb_maskSlt->addItem("colorMask");
    ui->cmb_maskSlt->addItem("processedMask");


    imgLoaderInit();

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

    ui->dsb_scale->setRange(0.5, 5);
    ui->dsb_scale->setSingleStep(0.5);
    ui->dsb_scale->setValue(1);


    ui->dsb_dectThreshold->setRange(0.001,0.1);
    ui->dsb_dectThreshold->setSingleStep(0.001);
    ui->dsb_dectThreshold->setValue(0.0001);

    ui->dsb_confidenceScale->setRange(50,150);
    ui->dsb_confidenceScale->setSingleStep(1);
    ui->dsb_confidenceScale->setValue(100);
    // 定义颜色后缀和控件名称映射
    QString colorSuffixes[] = {"R", "G", "Y"};
    QString paramNames[] = {"hMin1", "hMax1", "sMin1", "sMax1", "vMin1", "vMax1",
                            "hMin2", "hMax2", "sMin2", "sMax2", "vMin2", "vMax2"};

    // 批量设置范围和步长
    for (const QString& color : colorSuffixes)
    {
        for (const QString& param : paramNames)
        {
            QString controlName = QString("dsb_%1%2").arg(param).arg(color);
            QDoubleSpinBox* spinBox = findChild<QDoubleSpinBox*>(controlName);
            if (spinBox)
            {
                spinBox->setRange(0, 1);
                spinBox->setSingleStep(0.01);
            }
        }
    }


    // 定义颜色范围结构体数组
    struct ColorRange {
        const char* suffix;
        double hMin1, hMax1, sMin1, sMax1, vMin1, vMax1;
        double hMin2, hMax2, sMin2, sMax2, vMin2, vMax2;
    };

    ColorRange colorRanges[] = {
        {"R", 0.874, 1.000, 0.525, 0.955, 0.502, 0.976, 0.00, 0.10, 0.525, 0.955, 0.502, 0.976},
        {"G", 0.15, 0.50, 0.02, 0.30, 0.4, 1.0, 0.15, 0.50, 0.02, 0.30, 0.4, 1.0},
        {"Y", 0.12, 0.18, 0.5, 1.0, 0.4, 1.0, 0.10, 0.20, 0.4, 1.0, 0.3, 1.0}
    };

    // 批量设置初始值
    for (const auto& range : colorRanges)
    {
        QString suffix = range.suffix;

        // 第一组参数
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_hMin1%1").arg(suffix))->setValue(range.hMin1);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_hMax1%1").arg(suffix))->setValue(range.hMax1);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_sMin1%1").arg(suffix))->setValue(range.sMin1);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_sMax1%1").arg(suffix))->setValue(range.sMax1);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_vMin1%1").arg(suffix))->setValue(range.vMin1);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_vMax1%1").arg(suffix))->setValue(range.vMax1);

        // 第二组参数
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_hMin2%1").arg(suffix))->setValue(range.hMin2);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_hMax2%1").arg(suffix))->setValue(range.hMax2);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_sMin2%1").arg(suffix))->setValue(range.sMin2);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_sMax2%1").arg(suffix))->setValue(range.sMax2);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_vMin2%1").arg(suffix))->setValue(range.vMin2);
        ui->tabWidget->findChild<QDoubleSpinBox*>(QString("dsb_vMax2%1").arg(suffix))->setValue(range.vMax2);
    }
}

void Widget::getFrame()
{
    // 读取一帧
    cap >> frame;

    // 检查帧是否为空
    if (frame.empty())
    {
        // qDebug() << "无法获取帧";
    }
    else
        ImgLoader->loadImage(frame);


    if (fileImg.empty())
        ImgLoader->loadImage(cv::Mat());
    else
        ImgLoader->loadImage(fileImg);


    //--------------------------处理逻辑--------------------------
    // ImgLoader->getImage(SPEEDMETER);
    // ImgLoader->getImage(TEMPMETER);
    // ImgLoader->getImage(OILMETER);
    // ImgLoader->getImage(VOLMETER);

    // 指示灯检测
    for (int i = 0; i < 18; i++)
    {
        if(i<6 || (i>=10 && i<14))
            indicatorResult[i]=indicatorDetector.detect(ImgLoader->getImage(i+4),"red");

        if((i>=6 && i<10) || (i>=14 && i<16))
            indicatorResult[i]=indicatorDetector.detect(ImgLoader->getImage(i+4),"green");

        if(i>=16)
            indicatorResult[i]=indicatorDetector.detect(ImgLoader->getImage(i+4),"yellow");
    }

    // 开关检测
    // yoloResults = Yolodetector.detect(frame);
    // yoloResults = Yolodetector.detect(fileImg);
    // qDebug() << "检测到" << yoloResults.size() << "个目标";
    // for (const auto& result : yoloResults)
    // {
    //     qDebug() << "Class:" << QString::fromStdString(result.className)
    //             << "Confidence:" << result.confidence
    //             << "Box:" << result.box.x << result.box.y
    //             << result.box.width << result.box.height;
    // }

    //--------------------------处理逻辑--------------------------
    updateDisplay();

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
        fileImg=cv::imread(fileName.toStdString());
        // if (m_imageProcessor->loadImage(fileName))
        // {

        // }
    }
}

// 更新显示
void Widget::updateDisplay()
{
    ui->pixelViewer_1->setImage(ImgLoader->getOriginalWithROIsImage());

    cv::Mat resizedMat;
    if(maskIdx==0)
    {
        cv::resize(ImgLoader->getImage(streamIdx), resizedMat, cv::Size(), scaleFactor, scaleFactor, cv::INTER_LINEAR);
        ui->pixelViewer_2->setImage(resizedMat);
    }

    if(maskIdx==1)
    {
        cv::resize(indicatorResult[streamIdx].colorMask, resizedMat, cv::Size(), scaleFactor, scaleFactor, cv::INTER_LINEAR);
        ui->pixelViewer_2->setImage(resizedMat);

    }

    if(maskIdx==2)
    {
        cv::resize(indicatorResult[streamIdx].processedMask, resizedMat, cv::Size(), scaleFactor, scaleFactor, cv::INTER_LINEAR);
        ui->pixelViewer_2->setImage(resizedMat);

    }







    ui->led_indicator1->setText(QString::number(indicatorResult[0].isOn));
    ui->led_indicator2->setText(QString::number(indicatorResult[1].isOn));
    ui->led_indicator3->setText(QString::number(indicatorResult[2].isOn));
    ui->led_indicator4->setText(QString::number(indicatorResult[3].isOn));
    ui->led_indicator5->setText(QString::number(indicatorResult[4].isOn));
    ui->led_indicator6->setText(QString::number(indicatorResult[5].isOn));
    ui->led_indicator7->setText(QString::number(indicatorResult[6].isOn));
    ui->led_indicator8->setText(QString::number(indicatorResult[7].isOn));
    ui->led_indicator9->setText(QString::number(indicatorResult[8].isOn));
    ui->led_indicator10->setText(QString::number(indicatorResult[9].isOn));
    ui->led_indicator11->setText(QString::number(indicatorResult[10].isOn));
    ui->led_indicator12->setText(QString::number(indicatorResult[11].isOn));
    ui->led_indicator13->setText(QString::number(indicatorResult[12].isOn));
    ui->led_indicator14->setText(QString::number(indicatorResult[13].isOn));
    ui->led_indicator15->setText(QString::number(indicatorResult[14].isOn));
    ui->led_indicator16->setText(QString::number(indicatorResult[15].isOn));
    ui->led_indicator17->setText(QString::number(indicatorResult[16].isOn));
    ui->led_indicator18->setText(QString::number(indicatorResult[17].isOn));


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



void Widget::on_cmb_streamSlt_currentIndexChanged(int index)
{
    streamIdx=index;
}


void Widget::on_dsb_scale_valueChanged(double arg1)
{
    scaleFactor=arg1;
}

void Widget::on_cmb_maskSlt_currentIndexChanged(int index)
{
    maskIdx=index;
}


void Widget::on_btn_saveframe_clicked()
{
    // 设置固定保存路径
    QString savePath = "D:/PhD/8-aircraft/code/saveFrame/";

    // 确保目录存在
    QDir dir(savePath);
    if (!dir.exists()) {dir.mkpath(".");}

    // 生成带时间戳的文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QString filename = savePath + "frame_" + timestamp + ".jpg";

    cv::imwrite(filename.toStdString(), frame);
}

void Widget::on_dsb_dectThreshold_valueChanged(double arg1)
{
    indicatorDetector.setDetectionThreshold(arg1);
}


void Widget::on_dsb_confidenceScale_valueChanged(double arg1)
{
    indicatorDetector.setConfidenceScale(arg1);
}


void Widget::on_dsb_hMin1R_valueChanged(double arg1){indicatorDetector.redRange.hMin1=arg1;}
void Widget::on_dsb_hMax1R_valueChanged(double arg1){indicatorDetector.redRange.hMax1=arg1;}
void Widget::on_dsb_sMin1R_valueChanged(double arg1){indicatorDetector.redRange.sMin1=arg1;}
void Widget::on_dsb_sMax1R_valueChanged(double arg1){indicatorDetector.redRange.sMax1=arg1;}
void Widget::on_dsb_vMin1R_valueChanged(double arg1){indicatorDetector.redRange.vMin1=arg1;}
void Widget::on_dsb_vMax1R_valueChanged(double arg1){indicatorDetector.redRange.vMax1=arg1;}
void Widget::on_dsb_hMin2R_valueChanged(double arg1){indicatorDetector.redRange.hMin2=arg1;}
void Widget::on_dsb_hMax2R_valueChanged(double arg1){indicatorDetector.redRange.hMax2=arg1;}
void Widget::on_dsb_sMin2R_valueChanged(double arg1){indicatorDetector.redRange.sMin2=arg1;}
void Widget::on_dsb_sMax2R_valueChanged(double arg1){indicatorDetector.redRange.sMax2=arg1;}
void Widget::on_dsb_vMin2R_valueChanged(double arg1){indicatorDetector.redRange.vMin2=arg1;}
void Widget::on_dsb_vMax2R_valueChanged(double arg1){indicatorDetector.redRange.vMax2=arg1;}


void Widget::on_dsb_hMin1G_valueChanged(double arg1){indicatorDetector.greenRange.hMin1=arg1;}
void Widget::on_dsb_hMax1G_valueChanged(double arg1){indicatorDetector.greenRange.hMax1=arg1;}
void Widget::on_dsb_sMin1G_valueChanged(double arg1){indicatorDetector.greenRange.sMin1=arg1;}
void Widget::on_dsb_sMax1G_valueChanged(double arg1){indicatorDetector.greenRange.sMax1=arg1;}
void Widget::on_dsb_vMin1G_valueChanged(double arg1){indicatorDetector.greenRange.vMin1=arg1;}
void Widget::on_dsb_vMax1G_valueChanged(double arg1){indicatorDetector.greenRange.vMax1=arg1;}
void Widget::on_dsb_hMin2G_valueChanged(double arg1){indicatorDetector.greenRange.hMin2=arg1;}
void Widget::on_dsb_hMax2G_valueChanged(double arg1){indicatorDetector.greenRange.hMax2=arg1;}
void Widget::on_dsb_sMin2G_valueChanged(double arg1){indicatorDetector.greenRange.sMin2=arg1;}
void Widget::on_dsb_sMax2G_valueChanged(double arg1){indicatorDetector.greenRange.sMax2=arg1;}
void Widget::on_dsb_vMin2G_valueChanged(double arg1){indicatorDetector.greenRange.vMin2=arg1;}
void Widget::on_dsb_vMax2G_valueChanged(double arg1){indicatorDetector.greenRange.vMax2=arg1;}


void Widget::on_dsb_hMin1Y_valueChanged(double arg1){indicatorDetector.yellowRange.hMin1=arg1;}
void Widget::on_dsb_hMax1Y_valueChanged(double arg1){indicatorDetector.yellowRange.hMax1=arg1;}
void Widget::on_dsb_sMin1Y_valueChanged(double arg1){indicatorDetector.yellowRange.sMin1=arg1;}
void Widget::on_dsb_sMax1Y_valueChanged(double arg1){indicatorDetector.yellowRange.sMax1=arg1;}
void Widget::on_dsb_vMin1Y_valueChanged(double arg1){indicatorDetector.yellowRange.vMin1=arg1;}
void Widget::on_dsb_vMax1Y_valueChanged(double arg1){indicatorDetector.yellowRange.vMax1=arg1;}
void Widget::on_dsb_hMin2Y_valueChanged(double arg1){indicatorDetector.yellowRange.hMin2=arg1;}
void Widget::on_dsb_hMax2Y_valueChanged(double arg1){indicatorDetector.yellowRange.hMax2=arg1;}
void Widget::on_dsb_sMin2Y_valueChanged(double arg1){indicatorDetector.yellowRange.sMin2=arg1;}
void Widget::on_dsb_sMax2Y_valueChanged(double arg1){indicatorDetector.yellowRange.sMax2=arg1;}
void Widget::on_dsb_vMin2Y_valueChanged(double arg1){indicatorDetector.yellowRange.vMin2=arg1;}
void Widget::on_dsb_vMax2Y_valueChanged(double arg1){indicatorDetector.yellowRange.vMax2=arg1;}





