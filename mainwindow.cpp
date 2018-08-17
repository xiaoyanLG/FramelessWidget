#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : FramelessWidget(parent)
{
    setCentralWidget(new QWidget);
}

MainWindow::~MainWindow()
{

}
