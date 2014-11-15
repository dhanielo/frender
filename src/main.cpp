#include <QApplication>
#include <QWebPage>
#include <QUrl>
#include <webpage.h>

class WebPage;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //Thumbnailer* thumb = new Thumbnailer(QUrl("file:///home/daniel/index.html"));

    WebPage* page = new WebPage(QUrl("file:///home/daniel/index.html"));
    page->render(QString('/home/daniel/teste.pdf'), QVariantMap());

    return app.exec();
}
