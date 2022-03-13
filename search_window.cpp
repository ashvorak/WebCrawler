#include "search_window.h"
#include "./ui_search_window.h"

#include <QMessageBox>

#include <unordered_map>

#include "search_engine.h"

static constexpr auto       TABLE_COLUMN_COUNT = 3;
static const QStringList    TABLE_COLUMN_HEADER = QStringList {"URL", "Status", "Result"};

static constexpr auto TABLE_COLUMN_URL = 0;
static constexpr auto TABLE_COLUMN_STATUS = 1;
static constexpr auto TABLE_COLUMN_RESULT = 2;

static const QString PROGRESS_BAR_STYLE_PROCESS     = "QProgressBar::chunk { background-color: #0017FF; width: 20px;}";
static const QString PROGRESS_BAR_STYLE_FOUND       = "QProgressBar::chunk { background-color: #60D811; width: 20px;}";
static const QString PROGRESS_BAR_STYLE_NOT_FOUND   = "QProgressBar::chunk { background-color: #F81818; width: 20px;}";

static const std::unordered_map<UrlSearchStatus, QString> gErrorStatusMessages
{
    {UrlSearchStatus::kErrorTimeout,                    "Timeout Error"},
    {UrlSearchStatus::kErrorConnectionRefused,          "Connection Refused"},
    {UrlSearchStatus::kErrorRemoteHostClosed,           "Remote Host Closed"},
    {UrlSearchStatus::kErrorHostNotFound,               "Host Not Found"},
    {UrlSearchStatus::kErrorOperationCanceled,          "Operation Canceled"},
    {UrlSearchStatus::kErrorSslHandshakeFailed,         "Ssl Handshake Error"},
    {UrlSearchStatus::kErrorTemporaryNetworkFailure,    "Temporary Network Failure"},
    {UrlSearchStatus::kErrorNetworkSessionFailed,       "Network Session Failed"},
    {UrlSearchStatus::kErrorUnknownNetwork,             "Unknown Network Error"},
    {UrlSearchStatus::kErrorProtocolUnknown,            "Unknown Protocol Error"},
    {UrlSearchStatus::kErrorUnknown,                    "Unknown Error"}
};

SearchWindow::SearchWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchWindow)
{
    ui->setupUi(this);
    setFixedSize(size());

    ui->resumePushButton->setEnabled(false);

    InitTable();
    InitProgressBar();

    connect(&engine_, &SearchEngine::update_url_status,
            this, &SearchWindow::on_tableUpdate);
    connect(&engine_, &SearchEngine::search_result,
            this, &SearchWindow::on_searchResult);
}

SearchWindow::~SearchWindow()
{
    delete ui;
}

void SearchWindow::on_resumePushButton_clicked()
{
    qDebug() << "Resume Search";
    ResumeEngine();
}

void SearchWindow::on_pausePushButton_clicked()
{
    qDebug() << "Pause Search";
    PauseEngine();
}

void SearchWindow::on_stopPushButton_clicked()
{
    qDebug() << "Stop Search";
    StopEngine();
}

void SearchWindow::on_finishPushButton_clicked()
{
    qDebug() << "Finish Search";
    FinishEngine();
    ResetTable();
    ResetProgressBar();
    emit finish_button_clicked();
}

void SearchWindow::on_tableUpdate(const QString& url, UrlSearchStatus url_status)
{

    if (engine_.GetStatus() == EngineStatus::kStop) {
        UpdateTable(url, Qt::CheckState::Checked, "Interrupted", QColor(Qt::darkGray));
        return;
    }

    if (url_status != UrlSearchStatus::kProcess) {
        UpdateProgressBar();
    }

    switch (url_status) {
    case UrlSearchStatus::kFound:
        UpdateTable(url, Qt::CheckState::Checked, "Found", QColor(Qt::green));
        break;
    case UrlSearchStatus::kProcess:
        UpdateTable(url, Qt::CheckState::PartiallyChecked, "Process", QColor(Qt::blue));
        break;
    case UrlSearchStatus::kNotFound:
        UpdateTable(url, Qt::CheckState::Checked, "Not Found", QColor(Qt::lightGray));
        break;
    case UrlSearchStatus::kErrorTimeout:
    case UrlSearchStatus::kErrorConnectionRefused:
    case UrlSearchStatus::kErrorRemoteHostClosed:
    case UrlSearchStatus::kErrorHostNotFound:
    case UrlSearchStatus::kErrorOperationCanceled:
    case UrlSearchStatus::kErrorSslHandshakeFailed:
    case UrlSearchStatus::kErrorTemporaryNetworkFailure:
    case UrlSearchStatus::kErrorNetworkSessionFailed:
    case UrlSearchStatus::kErrorUnknownNetwork:
    case UrlSearchStatus::kErrorProtocolUnknown:
    case UrlSearchStatus::kErrorUnknown:
    {
        auto statusIt {gErrorStatusMessages.find(url_status)};
        if (statusIt != gErrorStatusMessages.end()) {
            UpdateTable(url, Qt::CheckState::Checked, statusIt->second, QColor(Qt::red));
        }
        else {
            UpdateTable(url, Qt::CheckState::Checked, "Unknown Error", QColor(Qt::red));
        }
        break;
    }
    default:
        UpdateTable(url, Qt::CheckState::Checked, "Unknown Error", QColor(Qt::red));
        break;
    }
}

void SearchWindow::on_searchResult(SearchResult result)
{
    UpdateProgressBarToMax();
    StopEngine();

    if (result == SearchResult::kFound) {
        SetProgressBarStyle(PROGRESS_BAR_STYLE_FOUND);
        QMessageBox::information(this, "Result", "Text Found!");
    }
    else
    {
        SetProgressBarStyle(PROGRESS_BAR_STYLE_NOT_FOUND);
        QMessageBox::information(this, "Result", "Text Not Found");
    }
}

void SearchWindow::Start(
    const QString& url_start,
    ushort threads_count,
    const QString& search_text,
    uint max_urls
)
{
    SetProgressBarMax(max_urls);

    engine_.Start(
        url_start,
        threads_count,
        search_text,
        max_urls
    );
}

// Private

void SearchWindow::ResumeEngine()
{
    ui->resumePushButton->setEnabled(false);
    ui->pausePushButton->setEnabled(true);
    engine_.Resume();
}

void SearchWindow::PauseEngine()
{
    ui->resumePushButton->setEnabled(true);
    ui->pausePushButton->setEnabled(false);
    engine_.Pause();
}

void SearchWindow::StopEngine()
{
    ui->resumePushButton->setEnabled(false);
    ui->pausePushButton->setEnabled(false);
    ui->stopPushButton->setEnabled(false);
    engine_.Stop();
}

void SearchWindow::FinishEngine()
{
    ui->resumePushButton->setEnabled(false);
    ui->pausePushButton->setEnabled(true);
    ui->stopPushButton->setEnabled(true);
    engine_.Stop();
}

void SearchWindow::InitTable()
{
    ui->urlTableWidget->setColumnCount(TABLE_COLUMN_COUNT);
    ui->urlTableWidget->setHorizontalHeaderLabels(TABLE_COLUMN_HEADER);
    ui->urlTableWidget->horizontalHeader()->setSectionResizeMode(TABLE_COLUMN_URL, QHeaderView::Stretch);
    ui->urlTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
}

void SearchWindow::UpdateTable(
    const QString& url,
    Qt::CheckState state,
    const QString& status,
    QColor color
)
{
    int table_id {0};
    bool found {false};

    int rows = ui->urlTableWidget->rowCount();
    for(int i = 0; i < rows; ++i)
    {
        if(ui->urlTableWidget->item(i, TABLE_COLUMN_URL)->text() == url) {
            found = true;
            table_id = i;
            break;
        }
    }

    if(!found)
    {
        table_id = current_records_num_++;
        ui->urlTableWidget->insertRow(table_id);
    }

    // Url column
    QTableWidgetItem *itemUrl = new QTableWidgetItem(url);
    ui->urlTableWidget->setItem(table_id, TABLE_COLUMN_URL, itemUrl);

    // Status column
    QTableWidgetItem *itemStatus = new QTableWidgetItem();
    itemStatus->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    itemStatus->data(Qt::CheckStateRole);
    itemStatus->setCheckState(state);
    ui->urlTableWidget->setItem(table_id, TABLE_COLUMN_STATUS, itemStatus);

    // Result column
    QTableWidgetItem *itemResult = new QTableWidgetItem(status);
    itemResult->setForeground(QBrush(color));
    ui->urlTableWidget->setItem(table_id, TABLE_COLUMN_RESULT, itemResult);

    ui->urlTableWidget->scrollToBottom();
}

void SearchWindow::ResetTable()
{
    current_records_num_ = 0;
    ui->urlTableWidget->clear();
    ui->urlTableWidget->setRowCount(0);
}

void SearchWindow::InitProgressBar()
{
    ui->progressBar->setValue(0);
    SetProgressBarStyle(PROGRESS_BAR_STYLE_PROCESS);
    ui->progressBar->setAlignment(Qt::AlignCenter);
    ui->progressBar->setMinimum(0);
}

void SearchWindow::SetProgressBarMax(int max_value)
{
    ui->progressBar->setMaximum(max_value);
}

void SearchWindow::SetProgressBarStyle(const QString& style)
{
    ui->progressBar->setStyleSheet(style);
}

void SearchWindow::UpdateProgressBar()
{
    auto value = ui->progressBar->value();
    if (++value != ui->progressBar->maximum())
        ui->progressBar->setValue(value);
}

void SearchWindow::UpdateProgressBarToMax()
{
    ui->progressBar->setValue(ui->progressBar->maximum());
}

void SearchWindow::ResetProgressBar()
{
    ui->progressBar->setValue(0);
    SetProgressBarStyle(PROGRESS_BAR_STYLE_PROCESS);
    ui->progressBar->reset();
}
