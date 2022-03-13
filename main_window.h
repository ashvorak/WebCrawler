#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "input_window.h"
#include "search_window.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void display();

private:
    Ui::MainWindow *ui;

    InputWindow ui_input {};
    SearchWindow ui_search {};

private slots:
    void startSearch();

    void quitSearch();

    void finishSearch();

};
#endif // MAINWINDOW_H
