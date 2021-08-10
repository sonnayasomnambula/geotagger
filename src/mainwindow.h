#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QString;
class QStringList;
QT_END_NAMESPACE

class Controller;
class Model;
class SelectionWatcher;
class TimeAdjustWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void restoreSession(const QString& directory);

private slots:
    void on_actionLoadTrack_triggered();
    void on_actionLoadPhotos_triggered();
    void on_actionAdjust_photo_timestamp_toggled(bool toggled);

private:
    void closeEvent(QCloseEvent* e) override;

    void loadSettings();
    void saveSettings();

    bool loadGPX(const QString& fileName);
    bool loadPhotos(const QStringList & fileNames);

    void onCurrentChanged(const QModelIndex& index);

    bool warn(const QString& title, const QString& message);

    Ui::MainWindow* ui = nullptr;
    Model* mModel = nullptr;
    SelectionWatcher* mSelection = nullptr;
};
#endif // MAINWINDOW_H
