#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "framelesswidget.h"

class MainWindow : public FramelessWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
