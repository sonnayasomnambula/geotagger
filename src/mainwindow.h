#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QString;
class QStringList;
QT_END_NAMESPACE

class PathController;
class PhotosListModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Open_triggered();
    void on_action_Add_photos_triggered();

private:
    void closeEvent(QCloseEvent* e) override;
    bool openTrack(const QString& name);

    void loadSettings();
    void saveSettings();

    Ui::MainWindow* ui = nullptr;
    PathController* mPathController = nullptr;
    PhotosListModel* mModel = nullptr;
};
#endif // MAINWINDOW_H
