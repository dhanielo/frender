#include <QApplication>
#include <QUrl>

#include <frender.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Frender *engine = Frender::instance();

   if (engine->execute()) {
       app.exec();
   }

   return 0;
}
