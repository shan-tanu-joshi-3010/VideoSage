#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QVariantList>

class NetworkManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList videos READ videos NOTIFY videosChanged)

public:
    explicit NetworkManager(QObject* parent = nullptr);

    QVariantList videos() const { return m_videos; }

    Q_INVOKABLE void fetchVideos();
    Q_INVOKABLE QString streamUrl(QString id);
    Q_INVOKABLE QString thumbUrl(QString name);
    Q_INVOKABLE void upload(QString filePath, QString title);
    Q_INVOKABLE void summarizeVideo(QString videoId);
    Q_SIGNAL void summaryReady(QString text);

signals:
    void videosChanged();
    void uploadFinished(QString msg);

private:
    QNetworkAccessManager nam;
    QVariantList m_videos;
};
