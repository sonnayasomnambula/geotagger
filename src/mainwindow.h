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
class TimeAdjustWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionLoadTrack_triggered();
    void on_actionAddPhotos_triggered();
    void on_actionAdjust_photo_timestamp_toggled(bool toggled);

private:
    void closeEvent(QCloseEvent* e) override;

    void loadSettings();
    void saveSettings();

    void selectionChanged();

    Ui::MainWindow* ui = nullptr;
    Model* mModel = nullptr;
};
#endif // MAINWINDOW_H
