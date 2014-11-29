#include "frender.h"

#include <QApplication>
#include <QStringList>
#include <QUrl>
#include <QString>
#include <QDebug>
#include <webpage.h>

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
        QUrl url = arg;
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
    page->setOutputPath(out_path);
    connect(page->mainFrame(),SIGNAL(loadFinished(bool)), this, SLOT(processPage()));
    page->mainFrame()->load(url);
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
