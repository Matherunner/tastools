#ifndef LOGTABLEVIEW_H
#define LOGTABLEVIEW_H

#include <QTableView>
#include "logtablemodel.h"

class LogTableView : public QTableView
{
public:
    LogTableView(QWidget *parent);
    void setModel(LogTableModel *model);
    LogTableModel *model() const;
    void setIndexToDiff(bool forward);
};

#endif
