#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPixmap>
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
#include "model.h"
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
        GeoPoint p = value.toPointF();
        return QGeoCoordinate(p.lat(), p.lon()).toString(QGeoCoordinate::DegreesWithHemisphere);
    }
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      mModel(new Model(this))
{
    ui->setupUi(this);
    QQmlEngine* engine = ui->map->engine();
    engine->rootContext()->setContextProperty("controller", mModel);
    ui->map->setSource(QUrl("qrc:///qml/map.qml"));

    ui->photos->setModel(mModel);
    ui->photos->setItemDelegateForColumn(Model::TableHeader::Time, new TimeDelegate(this));
    ui->photos->setItemDelegateForColumn(Model::TableHeader::Position, new GeoCoordinateDelegate(this));

    connect(ui->photos->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::selectionChanged);

    // playing with size policy and stretch factor didn't work
    ui->splitter->setSizes({860, 345});

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

void MainWindow::selectionChanged()
{
    auto selection = ui->photos->selectionModel()->selectedIndexes();
    if (selection.isEmpty())
    {
        ui->picture->setPixmap({});
        return;
    }

    QString fileName = mModel->data(selection.last(), Model::Role::Path).toString();
    QPixmap pix(fileName);
    ui->picture->setPixmap(pix.scaledToWidth(ui->picture->width()));
}

void MainWindow::on_actionLoadTrack_triggered()
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

    mModel->setTrack(loader.geoPath());
    mModel->setCenter(loader.center());
    mModel->setZoom(9); // TODO

    QTime start = loader.startTime().time();
    QTime finish = loader.finishTime().time();

    ui->startTime->setText(start.isNull() ? tr("NaN") : start.toString());
    ui->finishTime->setText(finish.isNull() ? tr("NaN") : finish.toString());

    if (start.isNull() || finish.isNull())
    {
        QMessageBox::warning(this, "", tr("No time information in the track!"));
        return false;
    }

    return true;
}

void MainWindow::on_actionAddPhotos_triggered()
{
    Settings settings;

    QString directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    directory = settings.dirs.photo(directory);
    QStringList names = QFileDialog::getOpenFileNames(this, "", directory, "*.jpg");
    if (names.isEmpty()) return;


    directory = QFileInfo(names.first()).absoluteDir().absolutePath();
    settings.dirs.photo.save(directory);

    if (!mModel->setFiles(names))
    {
        QMessageBox::warning(this, "", mModel->lastError());
        return;
    }

    mModel->setFiles(names);
}
