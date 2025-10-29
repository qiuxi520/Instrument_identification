#ifndef PIXELVIEWERWIDGET_H
#define PIXELVIEWERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScrollArea>
#include <QStatusBar>
#include <QMainWindow>
#include <QScrollBar>
#include <QPainter>
#include <opencv2/opencv.hpp>

class PixelViewerImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit PixelViewerImageLabel(QWidget *parent = nullptr);
    void setOpenCVImage(const cv::Mat &image);
    void setScaleFactor(double scale);
    double getScaleFactor() const { return m_scaleFactor; }
    void resetView();

    // 图像操作
    void zoomIn();
    void zoomOut();
    void fitToWindow();

    // 十字准星设置
    void setCrosshairEnabled(bool enabled);
    void setCrosshairColor(const QColor &color);
    void setCrosshairSize(int size);
    bool isCrosshairEnabled() const { return m_crosshairEnabled; }

    void setDragEnabled(bool enabled) { m_dragEnabled = enabled; }
    bool isDragEnabled() const { return m_dragEnabled; }

signals:
    void pixelInfoUpdated(int x, int y, const QColor &color, const QPoint &imagePos);
    void scaleFactorChanged(double scale);
    void imageSizeChanged(const QSize &size);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    bool m_dragEnabled;  // 新增：拖动启用标志

    QPixmap m_pixmap;
    cv::Mat m_cvImage;
    double m_scaleFactor;
    QPoint m_lastMousePos;
    bool m_isDragging;

    // 十字准星相关
    bool m_crosshairEnabled;
    QColor m_crosshairColor;
    int m_crosshairSize;
    QPoint m_crosshairPos;
    bool m_mouseInWidget;
    bool m_cursorHidden;  // 标记光标是否已隐藏

    QPoint widgetToImagePos(const QPoint &widgetPos) const;
    void updateDisplay();
    QRect getImageDisplayRect() const;
    void drawCrosshair(QPainter &painter);
};

class PixelViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PixelViewerWidget(QWidget *parent = nullptr);

    // 图像加载和操作
    bool loadImage(const QString &fileName);
    bool setImage(const cv::Mat &image);
    bool setImage(const QPixmap &pixmap);
    void clearImage();

    // 视图控制
    void zoomIn();
    void zoomOut();
    void fitToWindow();
    void resetView();
    void setScaleFactor(double scale);

    // 信息获取
    QSize getImageSize() const;
    double getScaleFactor() const;
    bool hasImage() const;

    // 配置选项
    void setShowPixelInfo(bool show);
    void setShowStatusBar(bool show);
    void setZoomEnabled(bool enabled);
    void setDragEnabled(bool enabled);

    // 十字准星配置
    void setCrosshairEnabled(bool enabled);
    void setCrosshairColor(const QColor &color);
    void setCrosshairSize(int size);
    bool isCrosshairEnabled() const;

signals:
    void pixelHovered(int x, int y, const QColor &color);
    void pixelClicked(int x, int y, const QColor &color, Qt::MouseButton button);
    void viewScaleChanged(double scale);
    void imageLoaded(bool success);

public slots:
    void onPixelInfoUpdated(int x, int y, const QColor &color, const QPoint &imagePos);

private:
    void createUI();
    void setupConnections();

    PixelViewerImageLabel *m_imageLabel;
    QScrollArea *m_scrollArea;
    QStatusBar *m_statusBar;
    QLabel *m_statusLabel;

    bool m_showPixelInfo;
    bool m_showStatusBar;
    bool m_zoomEnabled;
    bool m_dragEnabled;
};

#endif // PIXELVIEWERWIDGET_H
