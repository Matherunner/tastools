#include <QApplication>
#include "qcreadwin.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCReadWin qcreadWin;
    qcreadWin.show();
    return app.exec();
}
