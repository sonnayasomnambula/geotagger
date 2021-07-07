#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QStandardPaths>
#include <QSettings>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QQmlContext>
#include <QDateTime>
#include <QTime>

#include "abstractsettings.h"
#include "pathcontroller.h"
#include "photoslistmodel.h"
#include "gpx/loader.h"

struct Settings : AbstractSettings
{
    struct {
        Tag gpx = "dirs/gpx";
        Tag photo = "dirs/photo";
    } dirs;

    struct {
        State state = "window/state";
        Geometry geometry = "window/geometry";
        State splitterState = "window/splitterState";
        State photosHeaderState = "window/photosHeaderState";
    } window;
};

class TimeDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QString displayText(const QVariant& value, const QLocale& /*locale*/) const override {
        return value.toTime().toString(Qt::TextDate);
    }
};

class GeoCoordinateDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QString displayText(const QVariant& value, const QLocale& /*locale*/) const override {
        if (!value.isValid()) return "";
        QPointF p = value.toPointF();
        return QGeoCoordinate(p.x(), p.y()).toString(QGeoCoordinate::DegreesWithHemisphere);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      mPathController(new PathController(this)),
      mModel(new PhotosListModel(this))
{
    ui->setupUi(this);
    QQmlEngine * engine = ui->map->engine();
    engine->rootContext()->setContextProperty("pathController", mPathController);
    ui->map->setSource(QUrl("qrc:///qml/map.qml"));

    ui->photos->setModel(mModel);
    ui->photos->setItemDelegateForColumn(PhotosListModel::Header::Time, new TimeDelegate(this));
    ui->photos->setItemDelegateForColumn(PhotosListModel::Header::Position, new GeoCoordinateDelegate(this));

    loadSettings();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    saveSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettings()
{
    Settings settings;

    settings.window.state.restore(this);
    settings.window.geometry.restore(this);
    settings.window.splitterState.restore(ui->splitter);
    settings.window.photosHeaderState.restore(ui->photos->header());
}

void MainWindow::saveSettings()
{
    Settings settings;

    settings.window.state.save(this);
    settings.window.geometry.save(this);
    settings.window.splitterState.save(ui->splitter);
    settings.window.photosHeaderState.save(ui->photos->header());
}

void MainWindow::on_action_Open_triggered()
{
    Settings settings;

    QString directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    directory = settings.dirs.gpx(directory);
    QString name = QFileDialog::getOpenFileName(this, "", directory, "*.gpx");
    if (name.isEmpty()) return;

    directory = QFileInfo(name).absoluteDir().absolutePath();
    settings.dirs.gpx.save(directory);

    openTrack(name);
}

bool MainWindow::openTrack(const QString& name)
{
    GPX::Loader loader;
    if (!loader.load(name)) {
        QMessageBox::warning(this, "", loader.lastError());
        return false;
    }

    mPathController->setGeoPath(loader.geoPath());
    mPathController->setCenter(loader.center());
    mPathController->setZoom(9); // TODO
    return true;
}

void MainWindow::on_action_Add_photos_triggered()
{
    Settings settings;

    QString directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    directory = settings.dirs.photo(directory);
    QStringList names = QFileDialog::getOpenFileNames(this, "", directory, "*.jpg");
    if (names.isEmpty()) return;


    directory = QFileInfo(names.first()).absoluteDir().absolutePath();
    settings.dirs.photo.save(directory);

    if (!mModel->setFiles(names))
        QMessageBox::warning(this, "", mModel->lastError());
}
