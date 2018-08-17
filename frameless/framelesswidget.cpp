#include "framelesswidget.h"

#include <QApplication>
#include <QLabel>
#include <QToolButton>
#include <QMenuBar>
#include <QVBoxLayout>

static const char * const qt_close_xpm[] = {
"10 10 2 1",
"# c #FFFFFF",
". c None",
"##......##",
".##....##.",
"..##..##..",
"...####...",
"....##....",
"...####...",
"..##..##..",
".##....##.",
"##......##",
".........."};

static const char * const qt_maximize_xpm[]={
"10 10 2 1",
"# c #FFFFFF",
". c None",
"#########.",
"#########.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#########.",
".........."};

static const char * const qt_minimize_xpm[] = {
"10 10 2 1",
"# c #FFFFFF",
". c None",
"..........",
"..........",
"..........",
"..........",
"..........",
"..........",
"..........",
".########.",
".########.",
".........."};

static const char * const qt_normalizeup_xpm[] = {
"10 10 2 1",
"# c #FFFFFF",
". c None",
"...######.",
"...######.",
"...#....#.",
".######.#.",
".######.#.",
".#....###.",
".#....#...",
".#....#...",
".######...",
".........."};

FramelessWidget::FramelessWidget(QWidget *parent)
    : QWidget(parent), d(new FramelessWidgetPrivate)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#if defined(Q_OS_WIN)
    setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);

    auto hwnd = reinterpret_cast<HWND>(this->winId());

    const long style = d->compositionEnabled() ? d->aeroBorderlessFlag : d->basicBorderlessFlag;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    if (d->compositionEnabled()) {
        QtWin::extendFrameIntoClientArea(this, 1, 1, 1, 1);
    }

    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);

    d->layout = new QVBoxLayout(this);
    d->layout->addWidget(d->titleBar = new QWidget);
    d->layout->addLayout(d->contentLayout = new QVBoxLayout, 1);

    d->titleBar->installEventFilter(this);

    d->titleBar->setFixedHeight(28);
    d->layout->setSpacing(0);
    d->layout->setContentsMargins(QMargins(2, 2, 2, 2));
    d->layout->setMargin(0);
    d->contentLayout->setContentsMargins(QMargins(2, 0, 2, 2));

    auto makeTitleBar = [this] {
        auto layout = new QHBoxLayout(d->titleBar);
        layout->setContentsMargins(QMargins(8, 0, 8, 0));
        layout->setSpacing(8);
        layout->addWidget(d->iconLabel = new QLabel);
        d->iconLabel->setScaledContents(true);
        d->iconLabel->setFixedSize(16, 16);
        layout->addWidget(d->titleLabel = new QLabel, 2);
        layout->addWidget(d->minButton = new XYButton(this, XYButton::MINBTN));
        layout->addWidget(d->maxButton = new XYButton(this, XYButton::MAXBTN));
        layout->addWidget(d->closeButton = new XYButton(this, XYButton::CLOSEBTN));

        auto palette = d->titleLabel->palette();
        palette.setColor(QPalette::WindowText, Qt::white);
        d->titleLabel->setPalette(palette);
    };

    makeTitleBar();

    connect(d->closeButton, &QToolButton::clicked, this, [this]{this->close();});
    connect(d->minButton, &QToolButton::clicked, this, [this]{this->showMinimized();});
    connect(d->maxButton, &QToolButton::clicked, this, [this] {
        if (this->isMaximized()) {
            this->showNormal();
        } else {
            this->showMaximized();
        }
    });

#else
    d->layout = new QVBoxLayout(this);
    d->layout->setContentsMargins(QMargins(0, 0, 0, 0));
    d->layout->setSpacing(0);
#endif
}

FramelessWidget::~FramelessWidget()
{

}

bool FramelessWidget::event(QEvent *event)
{
#if defined(Q_OS_WIN)
    if (event->type() == QEvent::WindowStateChange) {
        d->maxButton->update();
        update();
    } else if (event->type() == QEvent::Resize) {
        update();
    }
#endif

    return QWidget::event(event);
}

void FramelessWidget::setWindowTitle(const QString &title)
{
#if defined(Q_OS_WIN)
    d->titleLabel->setText(title);
#else
    QWidget::setWindowTitle(title);
#endif
}

void FramelessWidget::setWindowIcon(const QPixmap &icon)
{
#if defined(Q_OS_WIN)
    d->iconLabel->setPixmap(icon);
#else
    QWidget::setWindowIcon(icon);
#endif
}

void FramelessWidget::setTitleBarColor(const QColor &color)
{
    d->titleBarColor = color;
}

void FramelessWidget::setBorderColor(const QColor &color)
{
    d->borderColor = color;
}

void FramelessWidget::setCentralWidget(QWidget *widget)
{
    if (d->centralWidget != nullptr) {
        d->centralWidget->setVisible(false);
        d->contentLayout->removeWidget(d->centralWidget);
    }
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->contentLayout->addWidget(widget);
    d->centralWidget = widget;
}

QPixmap FramelessWidget::icon() const
{
#if defined(Q_OS_WIN)
    if (d->iconLabel->pixmap())
        return *d->iconLabel->pixmap();
    return QPixmap();
#else
    return this->windowIcon().pixmap(48, 48);
#endif
}

bool FramelessWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->titleBar && event->type() == QEvent::Paint) {
        QPainter painter(d->titleBar);
        painter.setPen(d->borderColor);
        painter.setBrush(d->titleBarColor);
        painter.drawRect(d->titleBar->rect());
    }
    return QWidget::eventFilter(watched, event);
}

#if defined(Q_OS_WIN)
bool FramelessWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG* msg = *reinterpret_cast<MSG**>(message);
#else
    MSG* msg = reinterpret_cast<MSG*>(message);
#endif

    switch(msg->message) {
    case WM_SETFOCUS:
    {
        Qt::FocusReason reason;
        if (::GetKeyState(VK_LBUTTON) < 0 || ::GetKeyState(VK_RBUTTON) < 0)
            reason = Qt::MouseFocusReason;
        else if (::GetKeyState(VK_SHIFT) < 0)
            reason = Qt::BacktabFocusReason;
        else
            reason = Qt::TabFocusReason;

        QFocusEvent e(QEvent::FocusIn, reason);
        QApplication::sendEvent(this, &e);
    }
        break;
    case WM_NCCALCSIZE:
    {
        *result = 0;
        return true;
    }
        break;
    case WM_SYSCOMMAND:
    {
        if (msg->wParam == SC_KEYMENU && msg->lParam == VK_SPACE) {

            auto hwnd = (HWND)this->winId();
            HMENU menu = GetSystemMenu(hwnd, false);

            if (menu) {
                MENUITEMINFO mii;
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_STATE;
                mii.fType = 0;

                mii.fState = MF_ENABLED;

                SetMenuItemInfo(menu, SC_RESTORE, FALSE, &mii);
                SetMenuItemInfo(menu, SC_MAXIMIZE, FALSE, &mii);
                SetMenuItemInfo(menu, SC_MINIMIZE, FALSE, &mii);

                // update the options
                mii.fState = MF_DISABLED;
                SetMenuItemInfo(menu, SC_SIZE, TRUE, &mii);
                SetMenuItemInfo(menu, SC_MOVE, TRUE, &mii);

                mii.fState = MF_GRAYED;

                WINDOWPLACEMENT wp;
                GetWindowPlacement(hwnd, &wp);

                switch (wp.showCmd)
                {
                case SW_SHOWMAXIMIZED:
                    SetMenuItemInfo(menu, SC_MAXIMIZE, FALSE, &mii);
                    SetMenuDefaultItem(menu, SC_CLOSE, FALSE);
                    break;
                case SW_SHOWMINIMIZED:
                    SetMenuItemInfo(menu, SC_MINIMIZE, FALSE, &mii);
                    SetMenuDefaultItem(menu, SC_RESTORE, FALSE);
                    break;
                case SW_SHOWNORMAL:
                    SetMenuItemInfo(menu, SC_RESTORE, FALSE, &mii);
                    SetMenuDefaultItem(menu, SC_CLOSE, FALSE);
                    break;
                }

                auto devicePixelRatio = qApp->devicePixelRatio();
                auto localPos = d->iconLabel->frameGeometry().bottomLeft();
                localPos.setY(d->titleBar->frameGeometry().bottom());
                auto globalPos = this->mapToGlobal(localPos);
                globalPos.rx() *= devicePixelRatio;
                globalPos.ry() *= devicePixelRatio;
                BOOL cmd = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, globalPos.x(), globalPos.y(), hwnd, nullptr);
                if (cmd) PostMessage(hwnd, WM_SYSCOMMAND, cmd, 0);

                *result = 0;
                return true;
            }
        }
    }
        break;
    case WM_SYSKEYDOWN:
    {
        if (msg->wParam == VK_SPACE) {
            *result = 0;
            return true;
        }
    }
        break;
    case WM_NCHITTEST:
    {
        auto x = GET_X_LPARAM(msg->lParam);
        auto y = GET_Y_LPARAM(msg->lParam);
        auto borderX = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
        auto borderY = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
        borderX /= qApp->devicePixelRatio();
        borderY /= qApp->devicePixelRatio();
        RECT winrect;
        GetWindowRect((HWND)(this->winId()), &winrect);

        auto devicePixelRatio = qApp->devicePixelRatio();
        auto localPos = this->mapFromGlobal(QPoint(x / devicePixelRatio, y / devicePixelRatio));

        if (d->iconLabel->frameGeometry().contains(localPos)) {
            *result = HTSYSMENU;
            return true;
        }

        if (d->closeButton->frameGeometry().contains(localPos) || d->minButton->frameGeometry().contains(localPos) || d->maxButton->frameGeometry().contains(localPos)) {
            *result = HTCLIENT;
            return true;
        }

        auto titleBarGeometry = d->titleBar->frameGeometry();
        if (this->isMaximized()) {
            if (titleBarGeometry.contains(localPos)) {
                *result = HTCAPTION;
                return true;
            }
        }
        titleBarGeometry -= QMargins(borderX, borderY, borderX, 0);
        if (titleBarGeometry.contains(localPos)) {
            *result = HTCAPTION;
            return true;
        }

        if (x >= winrect.left && x <= winrect.left + borderX) {
            if (y >= winrect.top && y <= winrect.top + borderY) {
                *result = HTTOPLEFT;
                return true;
            }
            if (y > winrect.top + borderY && y < winrect.bottom - borderY) {
                *result = HTLEFT;
                return true;
            }
            if (y >= winrect.bottom - borderY && y <= winrect.bottom) {
                *result = HTBOTTOMLEFT;
                return true;
            }
        } else if (x > winrect.left + borderX && x < winrect.right - borderX) {
            if (y >= winrect.top && y <= winrect.top + borderY) {
                *result = HTTOP;
                return true;
            }
            if (y > winrect.top + borderY && y < winrect.top + borderY + 20) {
                *result = HTCAPTION;
                return true;
            }
            if (y >= winrect.bottom - borderY && y <= winrect.bottom) {
                *result = HTBOTTOM;
                return true;
            }
        } else if (x >= winrect.right - borderX && x <= winrect.right) {
            if (y >= winrect.top && y <= winrect.top + borderY) {
                *result = HTTOPRIGHT;
                return true;
            }
            if (y > winrect.top + borderY && y < winrect.bottom - borderY) {
                *result = HTRIGHT;
                return true;
            }
            if (y >= winrect.bottom - borderY && y <= winrect.bottom) {
                *result = HTBOTTOMRIGHT;
                return true;
            }
        } else {
            *result = HTNOWHERE;
            return true;
        }
    }
        break;
    case WM_NCACTIVATE:
    {
        if (!d->compositionEnabled()) {
            *result = 1;
            return true;
        }
    }
        break;
    case WM_DWMCOMPOSITIONCHANGED:
    {
        if (this->isMaximized()) {
            if (d->compositionEnabled()) {
                auto margin = 8 / this->devicePixelRatioF();
                d->layout->setContentsMargins(QMargins(margin, margin, margin, margin));
            } else {
                d->layout->setContentsMargins(QMargins(0, 0, 0, 0));
            }
        } else {
            d->layout->setContentsMargins(QMargins(0, 0, 0, 0));
        }
    }
        break;
    case WM_GETMINMAXINFO:
    {
        auto info = (MINMAXINFO*)msg->lParam;
        info->ptMinTrackSize.x = this->minimumWidth();
        info->ptMinTrackSize.y = this->minimumHeight();
        info->ptMaxTrackSize.x = this->maximumWidth();
        info->ptMaxTrackSize.y = this->maximumHeight();

        if (::MonitorFromWindow(::FindWindow(L"Shell_TrayWnd", nullptr), MONITOR_DEFAULTTONEAREST) ==
                ::MonitorFromWindow((HWND)(this->winId()) , MONITOR_DEFAULTTONEAREST))
        {
            info->ptMaxPosition.x = 0;
            info->ptMaxPosition.y = 0;

            info->ptMaxSize.x = GetSystemMetrics(SM_CXFULLSCREEN) + GetSystemMetrics(SM_CXDLGFRAME)
             + GetSystemMetrics(SM_CXBORDER)+ GetSystemMetrics(SM_CYBORDER);

            info-> ptMaxSize.y = GetSystemMetrics(SM_CYFULLSCREEN) + GetSystemMetrics(SM_CYCAPTION)
             + GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYBORDER);
        }
        return true;
    }
        break;
    default:
        break;
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void FramelessWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // 这里只是绘制边框
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto rect = this->rect();
    if (this->isMaximized() && d->compositionEnabled()) {
        auto margin = 8 / this->devicePixelRatioF();
        rect -= QMargins(margin, margin, margin, margin);
    }

    QPen pen;
    pen.setWidth(1);
    pen.setColor(d->borderColor);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect);
}
#endif

bool FramelessWidget::FramelessWidgetPrivate::compositionEnabled() const
{
    return QtWin::isCompositionEnabled();
}

XYButton::XYButton(FramelessWidget *frame, XYButton::Type type)
   :m_frame(frame), m_type(type)
{
    setFixedSize(QSize(10, 10));
}

void XYButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QPixmap pixmap;
    switch (m_type) {
    case CLOSEBTN:
        pixmap = QPixmap(qt_close_xpm);
        break;
    case MINBTN:
        pixmap = QPixmap(qt_minimize_xpm);
        break;
    case MAXBTN:
        if (m_frame->isMaximized()) {
            pixmap = QPixmap(qt_normalizeup_xpm);
        } else {
            pixmap = QPixmap(qt_maximize_xpm);
        }
        break;
    default:
        break;
    }

    painter.drawPixmap(rect(), pixmap);
}
