#include "pixelviewerwidget.h"
#include <QApplication>
#include <QScrollBar>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>

// PixelViewerImageLabel 实现
PixelViewerImageLabel::PixelViewerImageLabel(QWidget *parent)
    : QLabel(parent),
      m_scaleFactor(1.0),
      m_isDragging(false),
      m_crosshairEnabled(true),
      m_crosshairColor(Qt::red),
      m_crosshairSize(20),
      m_mouseInWidget(false),
      m_cursorHidden(false),  // 新增：初始化光标隐藏状态
      m_dragEnabled(true)  // 新增：默认启用拖动
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(100, 100);
    setMouseTracking(true);
    setScaledContents(false);
}

void PixelViewerImageLabel::setOpenCVImage(const cv::Mat &image)
{
    if (image.empty()) {
        m_cvImage.release();
        m_pixmap = QPixmap();
        update();
        return;
    }

    m_cvImage = image.clone();

    // 将 OpenCV Mat 转换为 QPixmap
    cv::Mat rgbImage;
    if (image.channels() == 3) {
        cv::cvtColor(image, rgbImage, cv::COLOR_BGR2RGB);
    } else if (image.channels() == 1) {
        cv::cvtColor(image, rgbImage, cv::COLOR_GRAY2RGB);
    } else {
        rgbImage = image.clone();
    }

    QImage qimage(rgbImage.data, rgbImage.cols, rgbImage.rows,
                  rgbImage.step, QImage::Format_RGB888);
    m_pixmap = QPixmap::fromImage(qimage.copy());

    m_scaleFactor = 1.0;
    updateDisplay();
    emit imageSizeChanged(m_pixmap.size());
}

void PixelViewerImageLabel::setScaleFactor(double scale)
{
    m_scaleFactor = qMax(0.1, qMin(10.0, scale));
    updateDisplay();
    emit scaleFactorChanged(m_scaleFactor);
}

void PixelViewerImageLabel::resetView()
{
    m_scaleFactor = 1.0;
    updateDisplay();
    emit scaleFactorChanged(m_scaleFactor);
}

void PixelViewerImageLabel::zoomIn()
{
    setScaleFactor(m_scaleFactor * 1.2);
}

void PixelViewerImageLabel::zoomOut()
{
    setScaleFactor(m_scaleFactor / 1.2);
}

void PixelViewerImageLabel::fitToWindow()
{
    if (m_pixmap.isNull()) return;

    QWidget *parent = parentWidget();
    if (!parent) return;

    QSize parentSize = parent->size();
    QSize imageSize = m_pixmap.size();

    double scaleX = (double)parentSize.width() / imageSize.width();
    double scaleY = (double)parentSize.height() / imageSize.height();

    m_scaleFactor = qMin(scaleX, scaleY) * 0.9;
    updateDisplay();
    emit scaleFactorChanged(m_scaleFactor);
}

// 十字准星设置
void PixelViewerImageLabel::setCrosshairEnabled(bool enabled)
{
    if (m_crosshairEnabled != enabled) {
        m_crosshairEnabled = enabled;

        // 新增：禁用十字准星时恢复光标
        if (!enabled && m_cursorHidden) {
            setCursor(Qt::ArrowCursor);
            m_cursorHidden = false;
        }

        update();
    }
}

void PixelViewerImageLabel::setCrosshairColor(const QColor &color)
{
    if (m_crosshairColor != color) {
        m_crosshairColor = color;
        update();
    }
}

void PixelViewerImageLabel::setCrosshairSize(int size)
{
    if (m_crosshairSize != size) {
        m_crosshairSize = qMax(5, qMin(100, size));
        update();
    }
}

void PixelViewerImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    m_crosshairPos = event->pos();
    m_mouseInWidget = true;

    // 新增：在图像显示区域内隐藏光标，区域外显示光标
    QRect displayRect = getImageDisplayRect();
    bool inImageArea = displayRect.contains(event->pos());

    if (m_crosshairEnabled && inImageArea) {
        if (!m_cursorHidden) {
            setCursor(Qt::BlankCursor);  // 隐藏光标
            m_cursorHidden = true;
        }
    } else {
        if (m_cursorHidden) {
            setCursor(Qt::ArrowCursor);  // 恢复光标
            m_cursorHidden = false;
        }
    }

    QPoint imagePos = widgetToImagePos(event->pos());

    if (!m_cvImage.empty() &&
        imagePos.x() >= 0 && imagePos.x() < m_cvImage.cols &&
        imagePos.y() >= 0 && imagePos.y() < m_cvImage.rows) {

        // 获取像素颜色
        QColor pixelColor;
        if (m_cvImage.channels() == 3) {
            cv::Vec3b pixel = m_cvImage.at<cv::Vec3b>(imagePos.y(), imagePos.x());
            pixelColor = QColor(pixel[2], pixel[1], pixel[0]); // BGR to RGB
        } else if (m_cvImage.channels() == 1) {
            uchar pixel = m_cvImage.at<uchar>(imagePos.y(), imagePos.x());
            pixelColor = QColor(pixel, pixel, pixel);
        }

        emit pixelInfoUpdated(imagePos.x(), imagePos.y(), pixelColor, event->pos());
    } else {
        emit pixelInfoUpdated(-1, -1, QColor(), event->pos());
    }

    // 处理拖动 - 添加拖动启用检查
    if (m_isDragging && m_dragEnabled && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->pos() - m_lastMousePos;

//        qDebug() << "Dragging delta:" << delta;  // ✅ 调试

        // 查找 QScrollArea
        QScrollArea *scrollArea = nullptr;
        QWidget *p = parentWidget();
        while (p) {
            scrollArea = qobject_cast<QScrollArea*>(p);
            if (scrollArea) break;
            p = p->parentWidget();
        }

        if (scrollArea) {
            QScrollBar *hBar = scrollArea->horizontalScrollBar();
            QScrollBar *vBar = scrollArea->verticalScrollBar();
            hBar->setValue(hBar->value() - delta.x());
            vBar->setValue(vBar->value() - delta.y());
        }
    }

    m_lastMousePos = event->pos();
    update(); // 重绘以更新十字准星位置

    QLabel::mouseMoveEvent(event);
}

void PixelViewerImageLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragEnabled) {  // 添加拖动启用检查
        m_isDragging = true;
        m_lastMousePos = event->pos();
//        qDebug() << "Dragging started at" << m_lastMousePos;  // ✅ 调试
        // 拖动时使用抓手光标，不隐藏
        setCursor(Qt::ClosedHandCursor);
        m_cursorHidden = false;
    }

    QLabel::mousePressEvent(event);
//    qDebug() << "Start drag at:" << event->pos();
}

void PixelViewerImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragEnabled) {  // 添加拖动启用检查
        m_isDragging = false;
        // 释放后根据位置决定是否隐藏光标
        QRect displayRect = getImageDisplayRect();
        bool inImageArea = displayRect.contains(event->pos());

        if (m_crosshairEnabled && inImageArea) {
            setCursor(Qt::BlankCursor);
            m_cursorHidden = true;
        } else {
            setCursor(Qt::ArrowCursor);
            m_cursorHidden = false;
        }
    }
    QLabel::mouseReleaseEvent(event);
}

void PixelViewerImageLabel::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
    event->accept();
}

void PixelViewerImageLabel::leaveEvent(QEvent *event)
{
    m_mouseInWidget = false;

    // 新增：离开控件时恢复光标
    if (m_cursorHidden) {
        setCursor(Qt::ArrowCursor);
        m_cursorHidden = false;
    }

    update(); // 重绘以隐藏十字准星
    QLabel::leaveEvent(event);
}

void PixelViewerImageLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), QColor(50, 50, 50));

    if (m_pixmap.isNull()) {
        // 没有图像时显示提示
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "请加载图像");
        return;
    }

    // 绘制图像
    QRect displayRect = getImageDisplayRect();
    painter.drawPixmap(displayRect, m_pixmap);

    // 绘制背景网格（可选）
    if (m_scaleFactor > 2.0) {
        painter.setPen(QPen(QColor(0, 0, 0, 50), 1));
        for (int x = displayRect.left(); x <= displayRect.right(); x += m_scaleFactor) {
            painter.drawLine(x, displayRect.top(), x, displayRect.bottom());
        }
        for (int y = displayRect.top(); y <= displayRect.bottom(); y += m_scaleFactor) {
            painter.drawLine(displayRect.left(), y, displayRect.right(), y);
        }
    }

    // 绘制图像边框
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(displayRect);

    // 绘制十字准星
    if (m_crosshairEnabled && m_mouseInWidget && displayRect.contains(m_crosshairPos) && !m_isDragging) {
        drawCrosshair(painter);
    }
}

void PixelViewerImageLabel::drawCrosshair(QPainter &painter)
{
    // 保存原始画笔状态
    QPen originalPen = painter.pen();

    // 设置十字准星画笔
    QPen crosshairPen(m_crosshairColor);
    crosshairPen.setWidth(2);
    crosshairPen.setStyle(Qt::SolidLine);
    painter.setPen(crosshairPen);

    int x = m_crosshairPos.x();
    int y = m_crosshairPos.y();
    int halfSize = m_crosshairSize / 2;

    // 绘制水平线
    painter.drawLine(x - halfSize, y, x + halfSize, y);

    // 绘制垂直线
    painter.drawLine(x, y - halfSize, x, y + halfSize);

    // 绘制中心点
    painter.setBrush(m_crosshairColor);
    painter.drawEllipse(m_crosshairPos, 2, 2);

    // 恢复原始画笔
    painter.setPen(originalPen);
}

QPoint PixelViewerImageLabel::widgetToImagePos(const QPoint &widgetPos) const
{
    if (m_pixmap.isNull()) return QPoint(-1, -1);

    QRect displayRect = getImageDisplayRect();

    if (!displayRect.contains(widgetPos)) {
        return QPoint(-1, -1);
    }

    // 转换为图像坐标
    int imageX = (widgetPos.x() - displayRect.x()) / m_scaleFactor;
    int imageY = (widgetPos.y() - displayRect.y()) / m_scaleFactor;

    return QPoint(imageX, imageY);
}

void PixelViewerImageLabel::updateDisplay()
{
    if (m_pixmap.isNull()) return;

        QSize scaledSize = m_pixmap.size() * m_scaleFactor;
        resize(scaledSize);  // ❗ 关键：更新 QLabel 实际大小


    update(); // 触发重绘
}

QRect PixelViewerImageLabel::getImageDisplayRect() const
{
    if (m_pixmap.isNull()) return QRect();

    QSize scaledSize = m_pixmap.size() * m_scaleFactor;
    QRect contentRect = contentsRect();

    int x = (contentRect.width() - scaledSize.width()) / 2;
    int y = (contentRect.height() - scaledSize.height()) / 2;

    return QRect(x, y, scaledSize.width(), scaledSize.height());
}

// PixelViewerWidget 实现
PixelViewerWidget::PixelViewerWidget(QWidget *parent)
    : QWidget(parent),
      m_showPixelInfo(true),
      m_showStatusBar(true),
      m_zoomEnabled(true),
      m_dragEnabled(true)
{
    createUI();
    setupConnections();
}

void PixelViewerWidget::createUI()
{


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建滚动区域
    m_scrollArea = new QScrollArea;
    m_scrollArea->setBackgroundRole(QPalette::Dark);
//    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidgetResizable(false);  // ❗ 关键：必须为 false 才能滚动
    m_scrollArea->setAlignment(Qt::AlignCenter);

    // 创建图像标签
    m_imageLabel = new PixelViewerImageLabel;
    m_scrollArea->setWidget(m_imageLabel);

    mainLayout->addWidget(m_scrollArea);

    // 创建状态栏
    m_statusBar = new QStatusBar;
    m_statusLabel = new QLabel("就绪");
    m_statusBar->addWidget(m_statusLabel);

    if (m_showStatusBar) {
        mainLayout->addWidget(m_statusBar);
    } else {
        m_statusBar->hide();
    }


}

void PixelViewerWidget::setupConnections()
{
    connect(m_imageLabel, &PixelViewerImageLabel::pixelInfoUpdated,
            this, &PixelViewerWidget::onPixelInfoUpdated);
    connect(m_imageLabel, &PixelViewerImageLabel::scaleFactorChanged,
            this, &PixelViewerWidget::viewScaleChanged);
}

// 十字准星配置方法
void PixelViewerWidget::setCrosshairEnabled(bool enabled)
{
    m_imageLabel->setCrosshairEnabled(enabled);
}

void PixelViewerWidget::setCrosshairColor(const QColor &color)
{
    m_imageLabel->setCrosshairColor(color);
}

void PixelViewerWidget::setCrosshairSize(int size)
{
    m_imageLabel->setCrosshairSize(size);
}

bool PixelViewerWidget::isCrosshairEnabled() const
{
    return m_imageLabel->isCrosshairEnabled();
}

// 其他现有方法保持不变...
bool PixelViewerWidget::loadImage(const QString &fileName)
{
    cv::Mat image = cv::imread(fileName.toStdString());
    if (image.empty()) {
        emit imageLoaded(false);
        return false;
    }

    m_imageLabel->setOpenCVImage(image);
    emit imageLoaded(true);
    return true;
}

bool PixelViewerWidget::setImage(const cv::Mat &image)
{
    if (image.empty()) {
        emit imageLoaded(false);
        return false;
    }

    m_imageLabel->setOpenCVImage(image);
    emit imageLoaded(true);
    return true;
}

bool PixelViewerWidget::setImage(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        emit imageLoaded(false);
        return false;
    }

    // 将 QPixmap 转换为 OpenCV Mat 以便获取像素信息
    QImage qimage = pixmap.toImage();
    cv::Mat mat(qimage.height(), qimage.width(), CV_8UC3);
    for (int y = 0; y < qimage.height(); ++y) {
        for (int x = 0; x < qimage.width(); ++x) {
            QColor color = qimage.pixelColor(x, y);
            mat.at<cv::Vec3b>(y, x) = cv::Vec3b(color.blue(), color.green(), color.red());
        }
    }

    return setImage(mat);
}

void PixelViewerWidget::clearImage()
{
    m_imageLabel->setOpenCVImage(cv::Mat());
    m_statusLabel->setText("就绪");
}

void PixelViewerWidget::zoomIn()
{
    if (m_zoomEnabled) {
        m_imageLabel->zoomIn();
    }
}

void PixelViewerWidget::zoomOut()
{
    if (m_zoomEnabled) {
        m_imageLabel->zoomOut();
    }
}

void PixelViewerWidget::fitToWindow()
{
    m_imageLabel->fitToWindow();
}

void PixelViewerWidget::resetView()
{
    m_imageLabel->resetView();
}

void PixelViewerWidget::setScaleFactor(double scale)
{
    m_imageLabel->setScaleFactor(scale);
}

QSize PixelViewerWidget::getImageSize() const
{
    return m_imageLabel->pixmap() ? m_imageLabel->pixmap()->size() : QSize();
}

double PixelViewerWidget::getScaleFactor() const
{
    return m_imageLabel->getScaleFactor();
}

bool PixelViewerWidget::hasImage() const
{
    return !getImageSize().isEmpty();
}

void PixelViewerWidget::setShowPixelInfo(bool show)
{
    m_showPixelInfo = show;
    if (!show) {
        m_statusLabel->setText("就绪");
    }
}

void PixelViewerWidget::setShowStatusBar(bool show)
{
    m_showStatusBar = show;
    m_statusBar->setVisible(show);
}

void PixelViewerWidget::setZoomEnabled(bool enabled)
{
    m_zoomEnabled = enabled;
}

void PixelViewerWidget::setDragEnabled(bool enabled)
{
    m_dragEnabled = enabled;
    m_imageLabel->setDragEnabled(enabled);  // 传递给图像标签
}

void PixelViewerWidget::onPixelInfoUpdated(int x, int y, const QColor &color, const QPoint &imagePos)
{
    if (m_showPixelInfo && x >= 0 && y >= 0) {
        QString info = QString("坐标: (%1, %2) | RGB: (%3, %4, %5) | 十六进制: %6")
                          .arg(x).arg(y)
                          .arg(color.red()).arg(color.green()).arg(color.blue())
                          .arg(color.name().toUpper());
        m_statusLabel->setText(info);

        emit pixelHovered(x, y, color);
    } else if (m_showPixelInfo) {
        m_statusLabel->setText("就绪");
    }
}
