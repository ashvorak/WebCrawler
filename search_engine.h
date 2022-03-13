#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>
#include <QMutex>

#include <set>
#include <queue>

enum class UrlSearchStatus
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

enum class EngineStatus
{
    kProcess,
    kStop,
    kPause
};

enum class SearchResult
{
    kFound,
    kNotFound
};

class SearchWorker;

class SearchEngine : public QObject
{
    Q_OBJECT

public:
    SearchEngine();

    EngineStatus GetStatus() const;

    void Start(
        const QString& url_start,
        ushort threads_count,
        const QString& search_text,
        uint max_urls
    );

    void Pause();

    void Resume();

    void Stop();

signals:
    void update_url_status(const QString& url, UrlSearchStatus status);

    void search_result(SearchResult result);

private:

    EngineStatus status_ = EngineStatus::kStop;

    std::queue<QString> urls_queue_ {};
    std::set<QString>   checked_urls_ {};

    std::vector<SearchWorker*>  workers_ {};

    QMutex queue_mutex_;
    QMutex workers_mutex_;
    QMutex status_mutex_;

    void Reset();

    bool IsWorkersProcessed();

    void UpdateSearchStatus(const QString& url, UrlSearchStatus status);

};

#endif // SEARCHENGINE_H
