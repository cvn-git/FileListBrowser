#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // For QSettings
    QCoreApplication::setOrganizationName("CVN");
    QCoreApplication::setOrganizationDomain("github.com");
    QCoreApplication::setApplicationName("FileListBrowser");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
