#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PathController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Open_triggered();

private:
    bool open(const QString& name);

    Ui::MainWindow* ui = nullptr;
    PathController* mPathController = nullptr;
};
#endif // MAINWINDOW_H
