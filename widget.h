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

private:
    void imgLoaderInit();
    void setupConnections();
    void updateDisplay();

    Ui::Widget *ui;
    ImageProcessor *m_imageProcessor;
    ImgLoader *ImgLoader;
    QTimer *timer;
    cv::Mat frame;
    cv::VideoCapture cap;

    QString path = QCoreApplication::applicationDirPath();          //获取源文件路径
    QSettings *m_iniFile = new QSettings(path + "../../../settings.ini", QSettings::IniFormat);  //保存.ini路径

};
#endif // WIDGET_H
