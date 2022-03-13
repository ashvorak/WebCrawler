#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QtNetwork>

enum class WorkerResult
{
    kProcess,
    kFound,
    kNotFound,
    kErrorTimeout,
    kErrorConnectionRefused,
    kErrorRemoteHostClosed,
    kErrorHostNotFound,
    kErrorOperationCanceled,
    kErrorSslHandshakeFailed,
    kErrorTemporaryNetworkFailure,
    kErrorNetworkSessionFailed,
    kErrorUnknownNetwork,
    kErrorProtocolUnknown,
    kErrorUnknown
};

class SearchWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchWorker(
        QString search_text = nullptr,
        std::function<void(const QString&, WorkerResult)>   SetSearchStatus = nullptr,
        std::function<QString()>                            GetSearchedUrl = nullptr,
        std::function<void(const QString&)>                 AddSearchedUrl = nullptr
    );

    ~SearchWorker();

public slots:
    void Start();

public:
    void Pause();

    void Resume();

    void Stop();

    bool IsProcessed() const;

signals:

    void finished();

private:

    enum class State
    {
        kRunning,
        kPaused,
        kStopped,
    };

    QString search_text_;

    bool    is_proccessed_ = false;
    State   state_ = State::kRunning;

    std::function<void(const QString&, WorkerResult)>   SetSearchStatus_;
    std::function<QString()>                            GetSearchedUrl_;
    std::function<void(const QString&)>                 AddSearchedUrl_;

    void ProcessReply(const QString& url, const QString& data);

    void ProcessError(const QString& url, QNetworkReply::NetworkError error);

    void ParseUrls(const QString& data);
};

#endif // SEARCHWORKER_H
