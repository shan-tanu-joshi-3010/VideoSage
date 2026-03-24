#include "NetworkManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDateTime>
#include <QDebug>

static const QString BASE = "http://127.0.0.1:18080";

/* =========================================================
 *  CONSTRUCTOR
 * ========================================================= */

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent)
{}

/* =========================================================
 *  STREAM & THUMB URLS
 * ========================================================= */

QString NetworkManager::streamUrl(QString id)
{
    return BASE + "/videos/" + id + "/stream";
}

QString NetworkManager::thumbUrl(QString thumbName)
{
    return BASE + "/thumb/" + thumbName;
}

/* =========================================================
 *  FETCH VIDEOS
 * ========================================================= */

void NetworkManager::fetchVideos()
{
    QNetworkRequest req{ QUrl(BASE + "/videos") };
    QNetworkReply* rep = nam.get(req);

    connect(rep, &QNetworkReply::finished, this, [this, rep]() {

        QByteArray data = rep->readAll();
        rep->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray()) {
            qDebug() << "[UI] Invalid video list response:" << data;
            return;
        }

        m_videos.clear();
        for (const QJsonValue& v : doc.array()) {
            m_videos.append(v.toObject().toVariantMap());
        }

        emit videosChanged();
    });
}

/* =========================================================
 *  UPLOAD VIDEO
 * ========================================================= */

void NetworkManager::upload(QString filePath, QString title)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        emit uploadFinished("File open error");
        return;
    }

    QByteArray body = f.readAll();
    f.close();

    QString id = QString::number(QDateTime::currentSecsSinceEpoch());
    QString url = BASE + "/upload/" + id +
                  "?title=" + title +
                  "&ext=.mp4";

    QNetworkRequest req{ QUrl(url) };
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/octet-stream");

    QNetworkReply* rep = nam.put(req, body);

    connect(rep, &QNetworkReply::finished, this, [this, rep]() {

        if (rep->error() == QNetworkReply::NoError)
            emit uploadFinished("Upload success");
        else
            emit uploadFinished(rep->errorString());

        rep->deleteLater();
        fetchVideos();
    });
}

/* =========================================================
 *  🔥 SUMMARIZE VIDEO
 * ========================================================= */

void NetworkManager::summarizeVideo(QString videoId)
{
    if (videoId.isEmpty()) {
        emit summaryReady("Invalid video ID");
        return;
    }

    qDebug() << "[UI] Requesting summarization for:" << videoId;

    QString url = BASE + "/videos/" + videoId + "/summarize";

    QNetworkRequest req{ QUrl(url) };
    req.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/json");

    // POST with empty body (server uses videoId from URL)
    QNetworkReply* rep = nam.post(req, QByteArray());

    emit summaryReady("Processing… please wait");

    connect(rep, &QNetworkReply::finished, this, [this, rep]() {

        QByteArray raw = rep->readAll();
        rep->deleteLater();

        QJsonDocument doc = QJsonDocument::fromJson(raw);
        if (!doc.isObject()) {
            emit summaryReady("Invalid server response:\n" + raw);
            return;
        }

        QString summary = doc.object().value("summary").toString();
        if (summary.isEmpty())
            summary = "Server returned empty summary";

        emit summaryReady(summary);
    });
}
