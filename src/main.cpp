#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"
#include "pathcontroller.h"

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
    a.setOrganizationDomain("sonnayasomnambula.org");
    a.setApplicationVersion("0.1");

    qmlRegisterType<PathController>("org.sonnayasomnambula.pathcontroller", 0, 1, "PathController");

    MainWindow w;
    w.show();

    return a.exec();
}
