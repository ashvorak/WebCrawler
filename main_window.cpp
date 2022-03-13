#include "main_window.h"
#include "./ui_main_window.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(size());

    connect(&ui_input, SIGNAL(start_button_clicked()),
            this, SLOT(startSearch()));
    connect(&ui_input, SIGNAL(quit_button_clicked()),
            this, SLOT(quitSearch()));
    connect(&ui_input, SIGNAL(destroyed()),
            this, SLOT(show()));

    connect(&ui_search, SIGNAL(finish_button_clicked()),
            this, SLOT(finishSearch()));
    connect(&ui_search, SIGNAL(destroyed()),
            this, SLOT(show()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::display()
{
    ui_input.show();
}

void MainWindow::startSearch()
{
    auto start_url = ui_input.GetStartUrl();
    if (start_url.isEmpty()) {
        QMessageBox::critical(this, "Error", "Empty Start Url field.");
        return;
    }

    auto threads_count = ui_input.GetMaxThreads();
    if (threads_count.isEmpty()) {
        QMessageBox::critical(this, "Error", "Empty Max Threads field.");
        return;
    }

    auto search_text = ui_input.GetSearchText();
    if (search_text.isEmpty()) {
        QMessageBox::critical(this, "Error", "Empty Search Text field.");
        return;
    }

    auto max_urls = ui_input.GetMaxUrls();
    if (max_urls.isEmpty()) {
        QMessageBox::critical(this, "Error", "Empty Max Urls field.");
        return;
    }

    ui_input.hide();
    ui_search.show();

    ui_search.Start(
        start_url,
        threads_count.toUShort(),
        search_text,
        max_urls.toUInt()
    );
}

void MainWindow::quitSearch()
{
    ui_input.close();
    this->close();
}

void MainWindow::finishSearch()
{
    ui_search.close();
    ui_input.show();
}

