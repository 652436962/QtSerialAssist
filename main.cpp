#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QIcon icon;
    icon.addFile(":/icons/icon_16x16.png", QSize(16, 16));
    icon.addFile(":/icons/icon_32x32.png", QSize(32, 32));
    icon.addFile(":/icons/icon_48x48.png", QSize(48, 48));
    icon.addFile(":/icons/icon_64x64.png", QSize(64, 64));
    a.setWindowIcon(icon);
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
