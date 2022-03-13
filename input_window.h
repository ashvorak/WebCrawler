#ifndef INPUT_WINDOW_H
#define INPUT_WINDOW_H

#include <QWidget>

namespace Ui {
class InputWindow;
}

class InputWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InputWindow(QWidget *parent = nullptr);
    ~InputWindow();

    QString GetStartUrl() const;
    QString GetMaxThreads() const;
    QString GetSearchText() const;
    QString GetMaxUrls() const;

signals:
    void start_button_clicked();
    void quit_button_clicked();

private slots:
    void on_startUrlLineEdit_textEdited(const QString &arg1);

    void on_maxThreadsSpinBox_textChanged(const QString &arg1);

    void on_searchTextLineEdit_textEdited(const QString &arg1);

    void on_maxUrlsLineEdit_textEdited(const QString &arg1);

    void on_startPushButton_clicked();

    void on_quitPushButton_clicked();

private:
    Ui::InputWindow *ui;

    QString start_url_;
    QString max_threads_;
    QString search_text_;
    QString max_urls_;

};

#endif // INPUT_WINDOW_H
