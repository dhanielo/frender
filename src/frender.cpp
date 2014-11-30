#include "frender.h"
#include <webpage.h>

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QFileInfo>
#include <QStringList>
#include <QUrl>
#include <QString>
#include <QDebug>

#include <iostream>

#define BLANK_HTML  "<html><head></head><body></body></html>"

static Frender *frenderInstance  = NULL;

Frender::Frender()
{
    page = new WebPage();
    ended = false;

    connect(this, SIGNAL(finished()), this, SLOT(close()));
}

Frender::~Frender()
{
    emit closing(this);
}

Frender *Frender::instance()
{
    if (NULL == frenderInstance) {
        frenderInstance = new Frender();
    }

    return frenderInstance;
}

bool Frender::execute()
{
    if (ended)
        return false;

    QStringList args = QApplication::arguments();

    if (args.length() >= 2) {

        QString out_path = args[1];
        QString arg = args[2];
        QUrl url = QUrl::fromEncoded(QByteArray(arg.toLatin1()));
        if (url.scheme().isEmpty()) {
            url.setPath(QFileInfo(url.toString()).absoluteFilePath().prepend("/"));
            url.setScheme("file");
        }
        this->convert(url, out_path);
    } else {
        printf("Usage: frender source destination\n");
        printf("Sources: http, file, img\n");
        printf("Destinations: .pdf, .png\n");
        ended = true;
    }

    return !ended;
}

void Frender::convert(const QUrl &url, const QString &out_path)
{
    QNetworkRequest request;
    QNetworkAccessManager::Operation operation = QNetworkAccessManager::UnknownOperation;
    operation = QNetworkAccessManager::GetOperation;

    page->webPage()->triggerAction(QWebPage::Stop);
    page->setOutputPath(out_path);

    connect(page->mainFrame(),SIGNAL(loadFinished(bool)), this, SLOT(processPage()));

    request.setUrl(url);
    page->mainFrame()->load(request, operation);
}


void Frender::processPage()
{
   page->render(page->getOutputPath(), QVariantMap());
   emit finished();
}

void Frender::close()
{
    QApplication::instance()->exit();
}
