#ifndef QCREADWIN_H
#define QCREADWIN_H

#include <QDockWidget>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QLabel>
#include "logtableview.h"

class QCReadWin : public QMainWindow
{
    Q_OBJECT

public:
    QCReadWin();

private slots:
    void findNextDiff();
    void findPrevDiff();
    void openLogFile();
    void reloadLogFile();
    void showAbout();
    void showExtraLines(int);
    void showNumFrames(int, float);

private:
    QString logFileName;
    LogTableView *logTableView;
    QDockWidget *extraLinesDock;
    QPlainTextEdit *extraLinesEdit;
    QLabel *lblNumFrames;
};

#endif
