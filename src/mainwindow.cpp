#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QQmlContext>

#include "gpx/loader.h"
#include "pathcontroller.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      mPathController(new PathController(this))
{
    ui->setupUi(this);
    QQmlEngine * engine = ui->map->engine();
    engine->rootContext()->setContextProperty("pathController", mPathController);
    ui->map->setSource(QUrl("qrc:///qml/map.qml"));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_action_Open_triggered()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString name = QFileDialog::getOpenFileName(this, "", directory, "*.gpx");
    if (name.isEmpty()) return;

    open(name);
}

bool MainWindow::open(const QString& name)
{
    GPX::Loader loader;
    if (!loader.load(name)) {
        QMessageBox::warning(this, "", loader.lastError());
        return false;
    }

    mPathController->setGeoPath(loader.geoPath());
    mPathController->setCenter(loader.center());
    return true;
}
