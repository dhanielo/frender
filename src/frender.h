#ifndef FRENDER_H
#define FRENDER_H

#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkReply>
#include <webpage.h>

class Frender : public QObject
{
    Q_OBJECT

public:
    Frender();
    virtual ~Frender();

    void convert(const QUrl &url, const QString &out_path);
    bool execute();
    static Frender *instance();

public slots:
    void processPage();
    void close();

signals:
     void closing(QObject *obj);
     void loadFinished(const QString &status);
     void finished();

private:
     WebPage *page;
     bool ended;
};

#endif // FRENDER_H
