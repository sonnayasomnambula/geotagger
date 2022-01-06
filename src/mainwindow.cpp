#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDesktopServices>
#include <QPixmap>
#include <QStandardPaths>
#include <QSettings>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QStyledItemDelegate>
#include <QQmlContext>
#include <QDateTime>
#include <QDirIterator>
#include <QTime>

#include "gpx/loader.h"

#include "abstractsettings.h"
#include "model.h"
#include "selectionwatcher.h"
#include "timeadjustwidget.h"

struct Settings : AbstractSettings
{
    struct {
        Tag<QString> gpx = "dirs/gpx";
        Tag<QString> photo = "dirs/photo";
    } dirs;

    struct {
        State state = "window/state";
        Geometry geometry = "window/geometry";
        struct { State state = "window/splitter.state"; } splitter;
        struct { State state = "window/header.state"; } header;
        Tag<bool> adjustTimestamp = "window/adjustTimestamp";
    } window;

    struct {
        Tag<int> d = "adjustTimestamp/d";
        Tag<int> h = "adjustTimestamp/h";
        Tag<int> m = "adjustTimestamp/m";
        Tag<int> s = "adjustTimestamp/s";
    } adjustTimestamp;

    struct {
        Tag<QString> gpx = "session/gpx";
        Tag<QStringList> photos = "session/photos";
        Tag<bool> restore = "session/restore";
    } session;
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

struct Dropped
{
    QString gpx;
    QStringList photos;

    explicit Dropped(const QMimeData* mime) {
        for (const QUrl& url: mime->urls()) {
            append(url.toLocalFile());
        }
        std::sort(photos.begin(), photos.end());
    }

    bool isEmpty() const {
        return gpx.isEmpty() && photos.isEmpty();
    }

    void append(const QFileInfo& file) {

        if (file.isDir()) {
            QDirIterator i(file.absoluteFilePath(), QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
            while (i.hasNext())
                append(i.next());
            return;
        }

        if (file.isFile()) {
            if (file.suffix().compare("gpx", Qt::CaseInsensitive) == 0)
                gpx = file.absoluteFilePath();

            if (file.suffix().compare("jpg", Qt::CaseInsensitive) == 0 ||
                file.suffix().compare("jpeg", Qt::CaseInsensitive) == 0)
                photos.append(file.absoluteFilePath());
        }
    }
};

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mModel(new Model),
    mSelection(new SelectionWatcher)
{
    ui->setupUi(this);
    QQmlEngine* engine = ui->map->engine();
    connect(ui->map, &QQuickWidget::statusChanged, [this](QQuickWidget::Status status){
        if (status == QQuickWidget::Status::Error) {
            QStringList descripton;
            for (const auto& error: ui->map->errors())
                descripton.append(error.toString());
            QMessageBox::warning(this, tr("QML load failed"), descripton.join("\n"));
        }
    });
    connect(ui->map, &QQuickWidget::sceneGraphError, [this](QQuickWindow::SceneGraphError, const QString& message){
        QMessageBox::warning(this, "", message);
    });


    engine->rootContext()->setContextProperty("controller", mModel);
    engine->rootContext()->setContextProperty("selection", mSelection);
    ui->map->setSource(QUrl("qrc:///qml/map.qml"));

    ui->photos->setModel(mModel);
    ui->photos->setItemDelegateForColumn(Model::TableHeader::Time, new TimeDelegate(this));
    ui->photos->setItemDelegateForColumn(Model::TableHeader::Position, new GeoCoordinateDelegate(this));

    connect(ui->photos->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onCurrentChanged);
//    connect(mSelection, &SelectionWatcher::currentChanged, mModel, &Model::photosChanged);
    connect(mSelection, &SelectionWatcher::currentChanged, this, [this](const QString& path){
        QString current = mModel->data(ui->photos->currentIndex(), Model::Role::Path).toString();
        if (path != current) {
            ui->photos->setCurrentIndex(mModel->index(path));
        }
    });

    connect(mModel, &Model::progress, this, [this](int i, int total){
        ui->progressBar->setMaximum(total);
        ui->progressBar->setValue(i);
        ui->progressBar->setVisible(i != total);
    });
    ui->progressBar->hide();

    connect(mModel, &Model::trackChanged, this, [this]{
        const auto& track = mModel->track();
        if (track.isEmpty()) {
            QMessageBox::warning(this, "", tr("The track is empty!"));
            return;
        }

        QTime start = track.first().timestamp().time();
        QTime finish = track.last().timestamp().time();

        ui->startTime->setText(start.toString());
        ui->finishTime->setText(finish.toString());
    });

    connect(mModel, &Model::photosChanged, this, [this]{
        ui->actionSave_EXIF->setEnabled(mModel->rowCount() > 0);
    });
    ui->actionSave_EXIF->setEnabled(mModel->rowCount() > 0);

    connect(ui->timeAdjistWidget, &TimeAdjustWidget::changed, this, [this]{
        mModel->setTimeAdjust(ui->timeAdjistWidget->value());
        mModel->guessPhotoCoordinates();
    });
    ui->timeAdjistWidget->hide();

    // playing with size policy and stretch factor didn't work
    ui->splitter->setSizes({860, 345});

    loadSettings();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    saveSettings();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    Dropped dropped(e->mimeData());
    if (dropped.isEmpty())
        return;

    e->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* e)
{
    e->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* e)
{
    e->accept();
}

void MainWindow::dropEvent(QDropEvent* e)
{
    Dropped dropped(e->mimeData());
    if (dropped.isEmpty()) return;

    e->acceptProposedAction();
    if (!dropped.gpx.isEmpty())
        loadGPX(dropped.gpx);
    if (!dropped.photos.isEmpty())
        loadPhotos(dropped.photos);
}

MainWindow::~MainWindow()
{
    delete ui;

    // QML-used objects needs to live long enough for the QML engine to complete all the calls
    // so don't pass 'this' to the ctor and delete it later

    mModel->deleteLater();
    mSelection->deleteLater();
}

void MainWindow::restoreSession()
{
    Settings settings;

    bool restore = settings.session.restore;
    QString gpx = settings.session.gpx;
    QStringList photos = settings.session.photos;

    if (!restore || gpx.isEmpty() || photos.isEmpty()) return;

    loadGPX(gpx);
    loadPhotos(photos);
}

void MainWindow::loadSettings()
{
    Settings settings;

    settings.window.state.restore(this);
    settings.window.geometry.restore(this);
    settings.window.splitter.state.restore(ui->splitter);
    settings.window.header.state.restore(ui->photos->header());
    ui->actionAdjust_photo_timestamp->setChecked(settings.window.adjustTimestamp);

    ui->timeAdjistWidget->setDays(settings.adjustTimestamp.d);
    ui->timeAdjistWidget->setHours(settings.adjustTimestamp.h);
    ui->timeAdjistWidget->setMinutes(settings.adjustTimestamp.m);
    ui->timeAdjistWidget->setSeconds(settings.adjustTimestamp.s);

    ui->actionRestore_session_on_startup->setChecked(settings.session.restore);
}

void MainWindow::saveSettings()
{
    Settings settings;

    settings.window.state.save(this);
    settings.window.geometry.save(this);
    settings.window.splitter.state.save(ui->splitter);
    settings.window.header.state.save(ui->photos->header());
    settings.window.adjustTimestamp = ui->actionAdjust_photo_timestamp->isChecked();

    settings.adjustTimestamp.d = ui->timeAdjistWidget->days();
    settings.adjustTimestamp.h = ui->timeAdjistWidget->hours();
    settings.adjustTimestamp.m = ui->timeAdjistWidget->minutes();
    settings.adjustTimestamp.s = ui->timeAdjistWidget->seconds();

    settings.session.restore = ui->actionRestore_session_on_startup->isChecked();
}

void MainWindow::onCurrentChanged(const QModelIndex& index)
{
    if (!index.isValid())
    {
        ui->picture->setPixmap({});
        return;
    }

    QString fileName = mModel->data(index, Model::Role::Path).toString();
    QPixmap pix(fileName);
    ui->picture->setPixmap(pix);
    ui->picture->setPath(fileName);
    ui->pictureDetails->setText(QString("%1 (%2x%3, %4)")
                                .arg(fileName)
                                .arg(pix.width())
                                .arg(pix.height())
                                .arg(QLocale().formattedDataSize(QFileInfo(fileName).size())));
    mSelection->setCurrent(fileName);
}

bool MainWindow::warn(const QString& title, const QString& message)
{
    qWarning().nospace().noquote() << title << ": " << message;
    QMessageBox::warning(this, title, message);
    return false;
}

void MainWindow::on_actionLoadTrack_triggered()
{
    Settings settings;

    QString directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    directory = settings.dirs.gpx(directory);
    QString name = QFileDialog::getOpenFileName(this, "", directory, "*.gpx");
    if (name.isEmpty()) return;

    directory = QFileInfo(name).absoluteDir().absolutePath();
    settings.dirs.gpx = directory;
    if (settings.dirs.photo.isNull())
        settings.dirs.photo = directory;

    loadGPX(name);
}

bool MainWindow::loadGPX(const QString& fileName)
{
    if (fileName.isEmpty()) return false;

    GPX::Loader loader;
    if (!loader.load(fileName))
        return warn(tr("Unable to load GPX file"), loader.lastError());

    setWindowTitle(loader.name().isEmpty() ? tr("Strava uploader 0.1") : loader.name()); // TODO extract version

    mModel->setTrack(loader.track());
    mModel->setCenter(loader.center());
    mModel->setZoom(9); // TODO

    Settings settings;
    settings.session.gpx = fileName;

    return true;
}

void MainWindow::on_actionLoadPhotos_triggered()
{
    Settings settings;

    QString directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    directory = settings.dirs.photo(directory);
    QStringList names = QFileDialog::getOpenFileNames(this, "", directory, "*.jpg");
    if (names.isEmpty()) return;

    directory = QFileInfo(names.first()).absoluteDir().absolutePath();
    settings.dirs.photo = directory;
    if (settings.dirs.gpx.isNull())
        settings.dirs.gpx = directory;

    std::sort(names.begin(), names.end());
    loadPhotos(names);
}

bool MainWindow::loadPhotos(const QStringList& fileNames)
{
    if (!mModel->setPhotos(fileNames))
        return warn(tr("Unable to load photos"), mModel->lastError());

    Settings().session.photos = fileNames;

    return true;
}

void MainWindow::on_actionAdjust_photo_timestamp_toggled(bool toggled)
{
    ui->timeAdjistWidget->setVisible(toggled);
    if (ui->timeAdjistWidget->isVisible())
        ui->timeAdjistWidget->setFocus();
}

void MainWindow::on_actionE_xit_triggered()
{
    close();
}

void MainWindow::on_actionSave_EXIF_triggered()
{
    // TODO save as
    if (QMessageBox::question(this, "", tr("Overwrite existing files?")) != QMessageBox::Yes)
        return;

    if (!mModel->savePhotos()) {
        QMessageBox::warning(this, tr("Save failed"), mModel->lastError());
        return;
    }

    QMessageBox::information(this, "", tr("Saved succesfully"));

    QString firstFile = mModel->data(static_cast<const QAbstractItemModel*>(mModel)->index(0, 0), Model::Role::Path).toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(firstFile).absolutePath()));
}
