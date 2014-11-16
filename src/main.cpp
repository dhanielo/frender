#include <QApplication>
#include <QUrl>

#include <webpage.h>
#include <frender.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{

    if (argc >= 2) {

        QString out_path = argv[1];
        QString url = argv[2];

        QApplication app(argc, argv);

        Frender* engine = new Frender();
        QString contents = engine->getWebContents(QUrl(url));

        WebPage* page = new WebPage();
        page->setContent(contents);
        page->render(out_path, QVariantMap());

        return app.exec();

    } else {

        printf("Usage: frender source destination\n");
        printf("Sources: http, file, img\n");
        printf("Destinations: .pdf, .png\n");

        exit;
    }
}
