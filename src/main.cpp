#include <QApplication>
#include <QUrl>

#include <webpage.h>
#include <frender.h>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    QString path = "/home/daniel/teste.pdf";
    QString google = "http://www.google.com.br";

    Frender* engine = new Frender();
    QString contents = engine->getWebContents(QUrl("http://headdev.com.br/index.php"));

    WebPage* page = new WebPage();
    page->setContent(contents);
    page->render(path, QVariantMap());

    return app.exec();
}
