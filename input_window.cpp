#include "input_window.h"
#include "./ui_input_window.h"

#include <QThread>
#include <QIntValidator>
#include <QRegularExpressionValidator>

static const auto       IDEAL_THREAD_COUNT = QThread::idealThreadCount();
static constexpr auto   MAX_URLS_COUNT = 9999;

InputWindow::InputWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputWindow),
    max_threads_ {"1"}
{
    ui->setupUi(this);
    setFixedSize(size());

    QRegularExpression re(
                "https?:\\/\\/(www\\.)?"\
                "[-a-zA-Z0-9@:%._\\+~#=]{1,256}"\
                "\\.[a-zA-Z0-9()]{1,6}\\b([-a-z"\
                "A-Z0-9()@:%_\\+.~#?&//=]*)", QRegularExpression::CaseInsensitiveOption);
    auto url_validator = new QRegularExpressionValidator(re, this);
    ui->startUrlLineEdit->setValidator(url_validator);

    ui->maxThreads->setText("Threads Max Number (1 - " + QString::number(IDEAL_THREAD_COUNT) + ") :");
    ui->maxThreadsSpinBox->setMaximum(IDEAL_THREAD_COUNT);

    ui->maxUrlsLabel->setText("Urls Max Number (1 - " + QString::number(MAX_URLS_COUNT) + ") :");
    ui->maxUrlsLineEdit->setValidator(new QIntValidator(1, MAX_URLS_COUNT, ui->maxUrlsLineEdit));
}

InputWindow::~InputWindow()
{
    delete ui;
}

void InputWindow::on_startUrlLineEdit_textEdited(const QString &arg1)
{
    InputWindow::start_url_ = arg1;
}

void InputWindow::on_maxThreadsSpinBox_textChanged(const QString &arg1)
{
    InputWindow::max_threads_ = arg1;
}

void InputWindow::on_searchTextLineEdit_textEdited(const QString &arg1)
{
    InputWindow::search_text_ = arg1;
}

void InputWindow::on_maxUrlsLineEdit_textEdited(const QString &arg1)
{
    InputWindow::max_urls_ = arg1;
}

void InputWindow::on_startPushButton_clicked()
{
    qDebug() << "Start Search";
    emit start_button_clicked();
}

void InputWindow::on_quitPushButton_clicked()
{
    qDebug() << "Quit App";
    emit quit_button_clicked();
}

QString InputWindow::GetStartUrl() const
{
    return start_url_;
}

QString InputWindow::GetMaxThreads() const
{
    return max_threads_;
}

QString InputWindow::GetSearchText() const
{
    return search_text_;
}

QString InputWindow::GetMaxUrls() const
{
    return max_urls_;
}
