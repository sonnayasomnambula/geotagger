#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QString;
class QStringList;
class QMimeData;
QT_END_NAMESPACE

class Model;
class SelectionWatcher;
class TimeAdjustWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void restoreSession();

private slots:
    void on_actionLoadTrack_triggered();
    void on_actionAddPhotos_triggered();
    void on_actionAdjust_photo_timestamp_toggled(bool toggled);
    void on_actionE_xit_triggered();
    void on_actionSave_EXIF_triggered();

private:
    void closeEvent(QCloseEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dropEvent(QDropEvent* e) override;

    void loadSettings();
    void saveSettings();

    bool loadGPX(const QString& fileName);
    bool addPhotos(const QStringList & fileNames);

    void onCurrentChanged(const QModelIndex& index);

    bool warn(const QString& title, const QString& message);
    bool warn(const QString& message);

    Ui::MainWindow* ui = nullptr;
    Model* mModel = nullptr;
    SelectionWatcher* mSelection = nullptr;
};
#endif // MAINWINDOW_H
