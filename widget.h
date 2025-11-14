#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "ui_widget.h"
#include "ImgLoader.h"


#include "IndicatorDetector.h"


#include <QDateTime>
#include <QDir>


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

    void saveParam2Ini(int idx,QString paramName,QString paramVal);
    QVector<int> getParamFromIni(int idx);
    void spinBoxInit();
private slots:
    void getFrame();
    void on_btn_openPic_clicked();
    void onProcessingCompleted();
    void onErrorOccurred(const QString &errorMessage);
    void on_sb_step_valueChanged(int arg1);
    void on_sb_ROIx_valueChanged(int arg1);
    void on_sb_ROIy_valueChanged(int arg1);
    void on_sb_ROIwidth_valueChanged(int arg1);
    void on_sb_ROIheight_valueChanged(int arg1);
    void on_cmb_ROIslt_currentIndexChanged(int index);
    void on_cmb_streamSlt_currentIndexChanged(int index);
    void on_dsb_scale_valueChanged(double arg1);

    void on_btn_saveframe_clicked();

    void on_cmb_maskSlt_currentIndexChanged(int index);

    void on_dsb_hMin1R_valueChanged(double arg1);

    void on_dsb_hMax1R_valueChanged(double arg1);

    void on_dsb_sMin1R_valueChanged(double arg1);

    void on_dsb_sMax1R_valueChanged(double arg1);

    void on_dsb_vMin1R_valueChanged(double arg1);

    void on_dsb_vMax1R_valueChanged(double arg1);

    void on_dsb_hMin2R_valueChanged(double arg1);

    void on_dsb_hMax2R_valueChanged(double arg1);

    void on_dsb_sMin2R_valueChanged(double arg1);

    void on_dsb_sMax2R_valueChanged(double arg1);

    void on_dsb_vMin2R_valueChanged(double arg1);

    void on_dsb_vMax2R_valueChanged(double arg1);

    void on_dsb_hMin1G_valueChanged(double arg1);

    void on_dsb_hMax1G_valueChanged(double arg1);



    void on_dsb_sMin1G_valueChanged(double arg1);

    void on_dsb_sMax1G_valueChanged(double arg1);

    void on_dsb_vMin1G_valueChanged(double arg1);

    void on_dsb_vMax1G_valueChanged(double arg1);

    void on_dsb_hMin2G_valueChanged(double arg1);

    void on_dsb_hMax2G_valueChanged(double arg1);

    void on_dsb_sMin2G_valueChanged(double arg1);

    void on_dsb_sMax2G_valueChanged(double arg1);

    void on_dsb_vMin2G_valueChanged(double arg1);

    void on_dsb_vMax2G_valueChanged(double arg1);

    void on_dsb_hMin1Y_valueChanged(double arg1);

    void on_dsb_hMax1Y_valueChanged(double arg1);

    void on_dsb_sMin1Y_valueChanged(double arg1);

    void on_dsb_sMax1Y_valueChanged(double arg1);

    void on_dsb_vMin1Y_valueChanged(double arg1);

    void on_dsb_vMax1Y_valueChanged(double arg1);

    void on_dsb_hMin2Y_valueChanged(double arg1);

    void on_dsb_hMax2Y_valueChanged(double arg1);

    void on_dsb_sMin2Y_valueChanged(double arg1);

    void on_dsb_sMax2Y_valueChanged(double arg1);

    void on_dsb_vMin2Y_valueChanged(double arg1);

    void on_dsb_vMax2Y_valueChanged(double arg1);

    void on_dsb_dectThreshold_valueChanged(double arg1);

    void on_dsb_confidenceScale_valueChanged(double arg1);

    void on_btn_previousImg_clicked();

    void on_btn_nextImg_clicked();

private:
    void imgLoaderInit();
    void updateDisplay();

    Ui::Widget *ui;
    ImgLoader *ImgLoader;
    QTimer *timer;
    cv::Mat frame;
    cv::Mat fileImg;
    cv::VideoCapture cap;

    QString path = QCoreApplication::applicationDirPath();          //获取源文件路径
    QSettings *m_iniFile = new QSettings(path + "../../../settings.ini", QSettings::IniFormat);  //保存.ini路径
    int streamIdx=0;
    int maskIdx=0;
    double scaleFactor = 1.0;

    IndicatorDetector indicatorDetector;
    IndicatorResult indicatorResult[18]={};


QString directoryPath = "D:/PhD/8-aircraft/code/saveFrame";
    QStringList jpgFiles;
QStringList imageFiles;
int currentImageIndex = -1;
};
#endif // WIDGET_H
