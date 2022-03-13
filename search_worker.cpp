#include "search_worker.h"

#include <QRegularExpression>

static constexpr auto CONNECTION_TIMEOUT = 5000; // ms

SearchWorker::SearchWorker(
        QString search_text,
        std::function<void(const QString&, WorkerResult)>   SetSearchStatus,
        std::function<QString()>                            GetSearchedUrl,
        std::function<void(const QString&)>                 AddSearchedUrl
) : QObject {nullptr},
    search_text_ {search_text},
    SetSearchStatus_{SetSearchStatus},
    GetSearchedUrl_{GetSearchedUrl},
    AddSearchedUrl_{AddSearchedUrl}
{

}

SearchWorker::~SearchWorker() = default;

void SearchWorker::Start()
{
    while (state_ != State::kStopped) {

        if (state_ == State::kPaused) {
            continue;
        }

        auto url {GetSearchedUrl_()};
        if (url != nullptr) {
            is_proccessed_ = true;
            SetSearchStatus_(url, WorkerResult::kProcess);

            QTimer timer;
            timer.setSingleShot(true);
            QNetworkAccessManager manager;
            QEventLoop event;

            auto reply = manager.get(QNetworkRequest(QUrl(url)));

            connect(&timer, SIGNAL(timeout()), &event, SLOT(quit()));
            connect(reply, SIGNAL(finished()), &event, SLOT(quit()));
            connect(reply, SIGNAL(finished()), &event, SLOT(deleteLater()));

            timer.start(CONNECTION_TIMEOUT);
            event.exec();

            if(timer.isActive()) {
                timer.stop();
                auto error = reply->error();
                if (error == QNetworkReply::NoError) {
                    QString data = reply->readAll();
                    ProcessReply(url, data);
                }
                else {
                    ProcessError(url, error);
                }
            }
            else {
               // timeout
               is_proccessed_ = false;
               SetSearchStatus_(url, WorkerResult::kErrorTimeout);
               disconnect(reply, SIGNAL(finished()), &event, SLOT(quit()));
               reply->abort();
            }
        }
    }

    emit finished();
}

void SearchWorker::Pause()
{
    state_ = State::kPaused;
}

void SearchWorker::Resume()
{
    state_ = State::kRunning;
}

void SearchWorker::Stop()
{
    state_ = State::kStopped;
}

bool SearchWorker::IsProcessed() const
{
    return is_proccessed_;
}

// Private

void SearchWorker::ProcessReply(const QString& url, const QString& data)
{
    if (data.contains(search_text_)) {
        is_proccessed_ = false;
        SetSearchStatus_(url, WorkerResult::kFound);
    }
    else {
        ParseUrls(data);
        is_proccessed_ = false;
        SetSearchStatus_(url, WorkerResult::kNotFound);
    }
}

void SearchWorker::ProcessError(const QString& url, QNetworkReply::NetworkError error)
{

    WorkerResult status;

    switch (error) {
    case QNetworkReply::NetworkError::ConnectionRefusedError:
        status = WorkerResult::kErrorConnectionRefused;
        break;
    case QNetworkReply::NetworkError::RemoteHostClosedError:
        status = WorkerResult::kErrorRemoteHostClosed;
        break;
    case QNetworkReply::NetworkError::HostNotFoundError:
        status = WorkerResult::kErrorHostNotFound;
        break;
    case QNetworkReply::NetworkError::TimeoutError:
        status = WorkerResult::kErrorTimeout;
        break;
    case QNetworkReply::NetworkError::OperationCanceledError:
        status = WorkerResult::kErrorOperationCanceled;
        break;
    case QNetworkReply::NetworkError::SslHandshakeFailedError:
        status = WorkerResult::kErrorSslHandshakeFailed;
        break;
    case QNetworkReply::NetworkError::TemporaryNetworkFailureError:
        status = WorkerResult::kErrorTemporaryNetworkFailure;
        break;
    case QNetworkReply::NetworkError::NetworkSessionFailedError:
        status = WorkerResult::kErrorNetworkSessionFailed;
        break;
    case QNetworkReply::NetworkError::UnknownNetworkError:
        status = WorkerResult::kErrorUnknownNetwork;
        break;
    case QNetworkReply::NetworkError::ProtocolUnknownError:
        status = WorkerResult::kErrorProtocolUnknown;
        break;
    default:
        status = WorkerResult::kErrorUnknown;
        break;

    }

    is_proccessed_ = false;
    SetSearchStatus_(url, status);
}

void SearchWorker::ParseUrls(const QString& data)
{
    QRegularExpression re(
                "https?:\\/\\/(www\\.)?"\
                "[-a-zA-Z0-9@:%._\\+~#=]{1,256}"\
                "\\.[a-zA-Z0-9()]{1,6}\\b([-a-z"\
                "A-Z0-9()@:%_\\+.~#?&//=]*)", QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator it = re.globalMatch(data);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString url = match.captured(0);
        AddSearchedUrl_(url);
    }
}
