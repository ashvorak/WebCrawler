#include "search_engine.h"

#include <QThread>
#include <QDebug>

#include <unordered_map>

#include "search_worker.h"

static const std::unordered_map<WorkerResult, UrlSearchStatus> gStatusValues
{
    {WorkerResult::kProcess,                        UrlSearchStatus::kProcess},
    {WorkerResult::kFound,                          UrlSearchStatus::kFound},
    {WorkerResult::kNotFound,                       UrlSearchStatus::kNotFound},
    {WorkerResult::kErrorTimeout,                   UrlSearchStatus::kErrorTimeout},
    {WorkerResult::kErrorConnectionRefused,         UrlSearchStatus::kErrorConnectionRefused},
    {WorkerResult::kErrorRemoteHostClosed,          UrlSearchStatus::kErrorRemoteHostClosed},
    {WorkerResult::kErrorHostNotFound,              UrlSearchStatus::kErrorHostNotFound},
    {WorkerResult::kErrorOperationCanceled,         UrlSearchStatus::kErrorOperationCanceled},
    {WorkerResult::kErrorSslHandshakeFailed,        UrlSearchStatus::kErrorSslHandshakeFailed},
    {WorkerResult::kErrorTemporaryNetworkFailure,   UrlSearchStatus::kErrorTemporaryNetworkFailure},
    {WorkerResult::kErrorNetworkSessionFailed,      UrlSearchStatus::kErrorNetworkSessionFailed},
    {WorkerResult::kErrorUnknownNetwork,            UrlSearchStatus::kErrorUnknownNetwork},
    {WorkerResult::kErrorProtocolUnknown,           UrlSearchStatus::kErrorProtocolUnknown},
    {WorkerResult::kErrorUnknown,                   UrlSearchStatus::kErrorUnknown}
};

template <typename T>
static void clear(std::queue<T> &q )
{
   std::queue<T> empty;
   std::swap( q, empty );
}

SearchEngine::SearchEngine() = default;

EngineStatus SearchEngine::GetStatus() const
{
    return status_;
}

void SearchEngine::Start(
    const QString& url_start,
    ushort threads_count,
    const QString& search_text,
    uint max_urls
)
{
    status_ = EngineStatus::kProcess;

    auto setSearchStatus {[this](auto url, auto status) {
        QMutexLocker locker(&status_mutex_);

        auto statusIt {gStatusValues.find(status)};
        if (statusIt != gStatusValues.end()) {
            UpdateSearchStatus(url, statusIt->second);
        }
        else {
            UpdateSearchStatus(url, UrlSearchStatus::kErrorUnknown);
        }
    }};

    auto getSearchUrl {[this]() -> QString {
        QMutexLocker locker(&queue_mutex_);

        if (!urls_queue_.empty()) {
            auto url = urls_queue_.front();
            urls_queue_.pop();
            return url;
        }

        return nullptr;
    }};

    auto addSearchUrl {[this, max_urls](auto url) {
        QMutexLocker locker(&queue_mutex_);

        if (checked_urls_.find(url) == checked_urls_.end()
                && checked_urls_.size() < max_urls && status_ != EngineStatus::kStop) {
            urls_queue_.push(url);
            checked_urls_.insert(url);
        }
    }};

    addSearchUrl(url_start);
    workers_.reserve(threads_count);
    for (int i = 0; i < threads_count; ++i)
    {
        SearchWorker* worker = new SearchWorker(search_text,
                                                setSearchStatus,
                                                getSearchUrl,
                                                addSearchUrl);
        QThread* thread = new QThread();
        worker->moveToThread(thread);

        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));

        connect(thread, SIGNAL(started()), worker, SLOT(Start()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        thread->start();
        workers_.push_back(worker);
    }
}

void SearchEngine::Pause()
{
    QMutexLocker locker(&workers_mutex_);

    status_ = EngineStatus::kPause;
    for (auto worker : workers_) {
        worker->Pause();
    }
}

void SearchEngine::Resume()
{
    QMutexLocker locker(&workers_mutex_);

    status_ = EngineStatus::kProcess;
    for (auto worker : workers_) {
        worker->Resume();
    }
}

void SearchEngine::Stop()
{
    QMutexLocker locker(&workers_mutex_);

    status_ = EngineStatus::kStop;
    for (auto worker : workers_) {
        worker->Stop();
    }
    workers_.clear();

    Reset();
}

void SearchEngine::Reset()
{
    QMutexLocker locker(&queue_mutex_);

    clear(urls_queue_);
    checked_urls_.clear();
}

bool SearchEngine::IsWorkersProcessed()
{
    QMutexLocker locker(&workers_mutex_);

    for (auto& worker : workers_) {
        if (worker->IsProcessed()) {
             return true;
        }
    }

    return false;
}

void SearchEngine::UpdateSearchStatus(
    const QString& url,
    UrlSearchStatus status
)
{
    if (status_ == EngineStatus::kStop) {
        return;
    }

    emit update_url_status(url, status);

    if (status == UrlSearchStatus::kFound) {
        emit search_result(SearchResult::kFound);
    }
    else if (status_ != EngineStatus::kStop && status != UrlSearchStatus::kProcess) {
        if (!IsWorkersProcessed() && urls_queue_.empty()) {
            emit search_result(SearchResult::kNotFound);
        }
    }
}
