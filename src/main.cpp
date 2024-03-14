#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"
#include "model.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("sonnayasomnambula");
    a.setOrganizationDomain("sonnayasomnambula.github.io");
    a.setApplicationName("geotagger");
    a.setApplicationVersion("0.2");

    MainWindow w;
    w.show();

    return a.exec();
}
