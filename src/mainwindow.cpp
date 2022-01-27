#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QPixmap>
#include <QQmlContext>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QStyledItemDelegate>
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
        struct {
            Tag<int> d = "window/adjustTimestamp.d";
            Tag<int> h = "window/adjustTimestamp.h";
            Tag<int> m = "window/adjustTimestamp.m";
            Tag<int> s = "window/adjustTimestamp.s";
            Tag<bool> visible = "window/adjustTimestamp.visible";
        } adjustTimestamp;
    } window;

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
        auto p = value.toPointF();
        return QGeoCoordinate(p.x(), p.y()).toString(QGeoCoordinate::DegreesWithHemisphere);
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
    connect(ui->map, &QQuickWidget::statusChanged, [this](QQuickWidget::Status status){
        if (status == QQuickWidget::Status::Error) {
            QStringList descripton;
            for (const auto& error: ui->map->errors())
                descripton.append(error.toString());
            warn(tr("QML load failed"), descripton.join("\n"));
        }
    });
    connect(ui->map, &QQuickWidget::sceneGraphError, [this](QQuickWindow::SceneGraphError, const QString& message){
        warn(message);
    });


    QQmlEngine* engine = ui->map->engine();
    engine->rootContext()->setContextProperty("controller", mModel);
    engine->rootContext()->setContextProperty("selection", mSelection);
    ui->map->setSource(QUrl("qrc:///qml/map.qml"));

    ui->photos->setModel(mModel);
    ui->photos->setItemDelegateForColumn(Model::Column::Time, new TimeDelegate(this));
    ui->photos->setItemDelegateForColumn(Model::Column::Position, new GeoCoordinateDelegate(this));

    QAction * actionRemove = new QAction(tr("Remove"), this);
    actionRemove->setShortcut(QKeySequence::Delete);
    connect(actionRemove, &QAction::triggered, this, [this]{
        mModel->remove(ui->photos->selectionModel()->selectedRows());
    });
    ui->photos->addAction(actionRemove);

    connect(ui->photos->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onCurrentChanged);
    connect(mSelection, &SelectionWatcher::currentChanged, this, [this](int current){
        ui->photos->setCurrentIndex(mModel->index(current));
    });

    ui->progressBar->hide();

    connect(mModel, &Model::trackChanged, this, [this]{
        const auto& track = mModel->track();
        if (track.isEmpty()) {
            warn(tr("The track is empty!"));
            return;
        }

        ui->startTime->setText(track.first().timestamp().time().toString());
        ui->finishTime->setText(track.last().timestamp().time().toString());
    });

    connect(mModel, &Model::dataChanged, this, [this]{
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

    setTitle();
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

void MainWindow::dropEvent(QDropEvent* e)
{
    Dropped dropped(e->mimeData());
    if (dropped.isEmpty()) return;

    e->acceptProposedAction();
    if (!dropped.gpx.isEmpty())
        loadGPX(dropped.gpx);
    if (!dropped.photos.isEmpty())
        addPhotos(dropped.photos);
}

MainWindow::~MainWindow()
{
    delete ui;

    // QML-used objects needs to live long enough for the QML engine to complete all the operations
    // so don't pass 'this' to the ctor of these objects and delete it after

    mModel->deleteLater();
    mSelection->deleteLater();
}

void MainWindow::restoreSession()
{
    Settings settings;

    if (!settings.session.restore) return;

    loadGPX(settings.session.gpx);
    addPhotos(settings.session.photos);
}

void MainWindow::loadSettings()
{
    Settings settings;

    settings.window.state.restore(this);
    settings.window.geometry.restore(this);
    settings.window.splitter.state.restore(ui->splitter);
    settings.window.header.state.restore(ui->photos->header());
    ui->actionAdjust_photo_timestamp->setChecked(settings.window.adjustTimestamp.visible);

    ui->timeAdjistWidget->setDays(settings.window.adjustTimestamp.d);
    ui->timeAdjistWidget->setHours(settings.window.adjustTimestamp.h);
    ui->timeAdjistWidget->setMinutes(settings.window.adjustTimestamp.m);
    ui->timeAdjistWidget->setSeconds(settings.window.adjustTimestamp.s);

    ui->actionRestore_session_on_startup->setChecked(settings.session.restore);
}

void MainWindow::saveSettings()
{
    Settings settings;

    settings.window.state.save(this);
    settings.window.geometry.save(this);
    settings.window.splitter.state.save(ui->splitter);
    settings.window.header.state.save(ui->photos->header());
    settings.window.adjustTimestamp.visible = ui->actionAdjust_photo_timestamp->isChecked();

    settings.window.adjustTimestamp.d = ui->timeAdjistWidget->days();
    settings.window.adjustTimestamp.h = ui->timeAdjistWidget->hours();
    settings.window.adjustTimestamp.m = ui->timeAdjistWidget->minutes();
    settings.window.adjustTimestamp.s = ui->timeAdjistWidget->seconds();

    QStringList photos;
    for (int row = 0; row < mModel->rowCount(); ++row)
        photos += mModel->data(mModel->index(row), Model::Role::Path).toString();
    settings.session.photos = photos;

    settings.session.restore = ui->actionRestore_session_on_startup->isChecked();
}

void MainWindow::onCurrentChanged(const QModelIndex& index)
{
    if (!index.isValid())
    {
        ui->picture->setPixmap({});
        ui->picture->setPath("");
        ui->pictureDetails->clear();
        mSelection->setCurrent(-1);
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
    mSelection->setCurrent(index.row());
}

bool MainWindow::warn(const QString& title, const QString& message)
{
    qWarning().nospace().noquote() << title << ": " << message;
    QMessageBox::warning(this, title, message);
    return false;
}

bool MainWindow::warn(const QString& message)
{
    return warn("", message);
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

    setTitle(loader.name());

    mModel->setTrack(loader.track());
    mModel->setCenter(loader.center());
    mModel->setZoom(9); // TODO

    Settings().session.gpx = fileName;

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
    settings.dirs.photo = directory;
    if (settings.dirs.gpx.isNull())
        settings.dirs.gpx = directory;

    std::sort(names.begin(), names.end());
    addPhotos(names);
}

template <class ProgressSignaller>
class ProgressHandler
{
    QProgressBar* mProgressBar;

public:
    ProgressHandler(ProgressSignaller* signaller, QProgressBar* bar) : mProgressBar(bar) {
        Q_ASSERT(mProgressBar);
        QObject::connect(signaller, &ProgressSignaller::progress, signaller, [this](int current, int total){
            mProgressBar->setMaximum(total);
            mProgressBar->setValue(current);
        });
        mProgressBar->setValue(0);
        mProgressBar->show();
    }

    ~ProgressHandler() { mProgressBar->hide(); }
};

bool MainWindow::addPhotos(const QStringList& fileNames)
{
    jpeg::Loader loader;
    ProgressHandler progressHandler(&loader, ui->progressBar);

    if (!loader.load(fileNames))
        return warn(tr("Unable to load photos"), loader.errors.join("\n"));

    if (!loader.loaded.isEmpty())
    {
        mModel->add(loader.loaded);
        mModel->setCenter(loader.center);
        mModel->setZoom(9); // TODO
    }

    return true;
}

void MainWindow::setTitle(const QString& title)
{
    setWindowTitle(title.isEmpty() ? tr("geotagger %1").arg(qApp->applicationVersion()) : title);
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

    jpeg::Saver saver;
    ProgressHandler progressHandler(&saver, ui->progressBar);

    if (!saver.save(mModel->photos(), mModel->timeAdjust())) {
        warn(tr("Save failed"), saver.errors.join("\n"));
        return;
    }

    QMessageBox::information(this, "", tr("Saved succesfully"));

    QString firstFile = mModel->data(mModel->index(0), Model::Role::Path).toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(firstFile).absolutePath()));
}
