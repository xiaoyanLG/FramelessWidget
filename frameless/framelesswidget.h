#ifndef FRAMELESSWIDGET_H
#define FRAMELESSWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QPointer>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#include <windowsx.h>
#include <QtWinExtras>
#endif

class QVBoxLayout;
class QWidget;
class QLabel;
class QMenuBar;
class FramelessWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPixmap icon READ icon WRITE setWindowIcon)
public:
    explicit FramelessWidget(QWidget *parent = nullptr);
    ~FramelessWidget();

#if defined(Q_OS_WIN)
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    void paintEvent(QPaintEvent *event) override;
#endif
    void setWindowTitle(const QString &);
    void setWindowIcon(const QPixmap &icon);
    void setTitleBarColor(const QColor &color);
    void setBorderColor(const QColor &color);
    void setCentralWidget(QWidget *widget);

    QPixmap icon() const;

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;

    class FramelessWidgetPrivate;
    QPointer<FramelessWidgetPrivate> d;
};

class XYButton: public QToolButton
{
    Q_OBJECT
public:
    enum Type {CLOSEBTN, MINBTN, MAXBTN};
    XYButton(FramelessWidget *frame, Type type);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Type             m_type;
    FramelessWidget *m_frame = nullptr;
    friend class FramelessWidget;
};

class FramelessWidget::FramelessWidgetPrivate: public QObject
{
    Q_OBJECT
public:
    QVBoxLayout* layout = nullptr;

#if defined(Q_OS_WIN)
    long aeroBorderlessFlag = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    long basicBorderlessFlag = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

    bool compositionEnabled() const;

    QColor  titleBarColor = "#003377";
    QColor  borderColor = "#003377";
    QLabel* iconLabel = nullptr;
    QLabel* titleLabel = nullptr;
    QToolButton* minButton = nullptr;
    QToolButton* maxButton = nullptr;
    QToolButton* closeButton = nullptr;
    QWidget* centralWidget = nullptr;
    QWidget* titleBar = nullptr;
    QMenuBar* menuBar = nullptr;

    QVBoxLayout* contentLayout = nullptr;
#endif

};

#endif // FRAMELESSWIDGET_H
