#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QMap>
#include <QVariantMap>
#include <QtWebKitWidgets/QWebPage>
#include <QtWebKitWidgets/QWebFrame>
#include <QObject>


class WebPage : public QObject, public QWebFrame::PrintCallback
{
    Q_OBJECT
   /* Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString frameTitle READ frameTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)
    Q_PROPERTY(QString offlineStoragePath READ offlineStoragePath)
    Q_PROPERTY(int offlineStorageQuota READ offlineStorageQuota)
    Q_PROPERTY(QVariantMap viewportSize READ viewportSize WRITE setViewportSize)
    Q_PROPERTY(QVariantMap paperSize READ paperSize WRITE setPaperSize)
    Q_PROPERTY(QVariantMap clipRect READ clipRect WRITE setClipRect)
    Q_PROPERTY(QString windowName READ windowName)*/

public:
    WebPage(const QUrl &baseUrl = QUrl());
    virtual ~WebPage();

    QWebFrame *mainFrame();

    QString content() const;
    QString frameContent() const;
    QString plainText() const;
    QString framePlainText() const;
    void setContent(const QString &content);
    void setFrameContent(const QString &content);
    QString title() const;
    QString frameTitle() const;

    void setViewportSize(const QVariantMap &size);
    QVariantMap viewportSize() const;

    void setClipRect(const QVariantMap &size);
    QVariantMap clipRect() const;

    void setPaperSize(const QVariantMap &size);
    QVariantMap paperSize() const;

    QString footer(int page, int numPages);
    qreal footerHeight() const;
    QString header(int page, int numPages);
    qreal headerHeight() const;

    void setZoomFactor(qreal zoom);
    qreal zoomFactor() const;

signals:
     void closing(QObject *page);

public slots:
     bool render(const QString &fileName, const QVariantMap &map = QVariantMap());
    void release();
    void close();

    QString renderBase64(const QByteArray &format = "png");
    void setContent(const QString &content, const QString &baseUrl);

private:
    QImage renderImage();
    bool renderPdf(const QString &fileName);

private:
    QWebPage *m_customWebPage;
    QWebFrame *m_mainFrame;
    QWebFrame *m_currentFrame;
    QPoint m_scrollPosition;
    QRect m_clipRect;
    QVariantMap m_paperSize; // For PDF output via render()
};

#endif // WEBPAGE_H
