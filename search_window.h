#ifndef SEARCH_WINDOW_H
#define SEARCH_WINDOW_H

#include <QWidget>

#include "search_engine.h"

namespace Ui {
class SearchWindow;
}

class SearchWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWindow(QWidget *parent = nullptr);
    ~SearchWindow();

    void Start(
        const QString& url_start,
        ushort threads_count,
        const QString& search_text,
        uint max_urls
    );

signals:
    void finish_button_clicked();

private slots:
    void on_resumePushButton_clicked();

    void on_pausePushButton_clicked();

    void on_stopPushButton_clicked();

    void on_finishPushButton_clicked();

    void on_tableUpdate(const QString& url, UrlSearchStatus url_status);

    void on_searchResult(SearchResult result);

private:
    Ui::SearchWindow *ui;

    unsigned int current_records_num_ = 0;

    SearchEngine engine_ {};

    void ResumeEngine();
    void PauseEngine();
    void StopEngine();
    void FinishEngine();

    void InitTable();
    void UpdateTable(const QString& url,
                     Qt::CheckState state,
                     const QString& status,
                     QColor color);
    void ResetTable();

    void InitProgressBar();
    void SetProgressBarMax(int max_value);
    void SetProgressBarStyle(const QString& style);
    void UpdateProgressBar();
    void UpdateProgressBarToMax();
    void ResetProgressBar();
};

#endif // SEARCH_WINDOW_H
