#ifndef LOGTABLEVIEW_H
#define LOGTABLEVIEW_H

#include <QTableView>
#include "logtablemodel.h"

class LogTableView : public QTableView
{
    Q_OBJECT

public:
    LogTableView(QWidget *parent);
    void setModel(LogTableModel *model);
    LogTableModel *model() const;
    void setIndexToDiff(bool forward);

signals:
    void showNumFrames(int, float);

protected:
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected);
};

#endif
