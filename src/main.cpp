#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"
#include "model.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef Q_OS_LINUX
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

#ifdef Q_OS_WINDOWS
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP 1251"));
#endif

    a.setOrganizationName("sonnayasomnambula");
    a.setOrganizationDomain("sonnayasomnambula.github.io");
    a.setApplicationName("geotagger");
    a.setApplicationVersion("0.1");

    MainWindow w;
    w.show();

    return a.exec();
}
