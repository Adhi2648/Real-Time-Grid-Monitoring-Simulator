#include <QApplication>

#include "../common/Env.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // the app loads local settings first so the dashboard can pick up the server address
    Env::loadDotEnv();

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}
