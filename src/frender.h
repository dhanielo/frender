#ifndef FRENDER_H
#define FRENDER_H

#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkReply>

class Frender : public QObject
{
    Q_OBJECT

public:
    Frender();
    virtual ~Frender();

    QString getWebContents(const QUrl &url);

signals:
     void closing(QObject *obj);
};

#endif // FRENDER_H
