#include "frender.h"

#include <QUrl>
#include <QString>
#include <QtCUrl.h>
#include <QDebug>

#include <iostream>

#define BLANK_HTML  "<html><head></head><body></body></html>"

Frender::Frender()
{
}

Frender::~Frender()
{
    emit closing(this);
}

QString Frender::getWebContents(const QUrl &url)
{
    QtCUrl cUrl;
    cUrl.setTextCodec("utf-8");
    QtCUrl::Options opt;
    opt[CURLOPT_URL] = url;
    opt[CURLOPT_FOLLOWLOCATION] = true;
    opt[CURLOPT_FAILONERROR] = true;
    QStringList headers;
    headers
        << "Accept  text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"
        << "Accept-Encoding gzip,deflate,sdch"
        << "Accept-Language pt-BR,pt;q=0.8,en-US;q=0.6,en;q=0.4,it;q=0.2,gl;q=0.2,und;q=0.2,fr;q=0.2"
        << "User-Agent  Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1985.143 Safari/537.36";
    opt[CURLOPT_HTTPHEADER] = headers;
    QString result = cUrl.exec(opt);

    if (!cUrl.lastError().isOk()) {
        qDebug() << QString("Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer());
    }

    return result;
}
