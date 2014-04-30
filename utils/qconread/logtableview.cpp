#include <QHeaderView>
#include "logtableview.h"
#include "logtablemodel.h"

LogTableView::LogTableView(QWidget *parent)
    : QTableView(parent)
{
    setCornerButtonEnabled(false);
    setMouseTracking(true);

    QHeaderView *horizHead = horizontalHeader();
    horizHead->setSectionsMovable(true);
    horizHead->setSectionsClickable(false);

    QHeaderView *vertHead = verticalHeader();
    vertHead->setDefaultSectionSize(vertHead->minimumSectionSize());
    vertHead->setSectionResizeMode(QHeaderView::Fixed);
}

void LogTableView::setModel(LogTableModel *model)
{
    QTableView::setModel(model);
    QHeaderView *horizHead = horizontalHeader();
    int hMinSize = horizHead->minimumSectionSize();
    for (int i : {LogTableModel::HEAD_MSEC, LogTableModel::HEAD_OG,
                LogTableModel::HEAD_DST, LogTableModel::HEAD_DUCK,
                LogTableModel::HEAD_JUMP, LogTableModel::HEAD_FMOVE,
                LogTableModel::HEAD_SMOVE, LogTableModel::HEAD_UMOVE,
                LogTableModel::HEAD_USE, LogTableModel::HEAD_ATTACK,
                LogTableModel::HEAD_ATTACK2, LogTableModel::HEAD_RELOAD,
                LogTableModel::HEAD_WLVL, LogTableModel::HEAD_LADDER}) {
        horizHead->resizeSection(i, hMinSize);
        horizHead->setSectionResizeMode(i, QHeaderView::Fixed);
    }
    for (int i : {LogTableModel::HEAD_HP, LogTableModel::HEAD_AP}) {
        horizHead->resizeSection(i, hMinSize * 2);
    }
    for (int i : {LogTableModel::HEAD_FRATE, LogTableModel::HEAD_YAW,
                LogTableModel::HEAD_PITCH, LogTableModel::HEAD_POSX,
                LogTableModel::HEAD_POSY, LogTableModel::HEAD_POSZ}) {
        horizHead->resizeSection(i, hMinSize * 3);
    }
}

LogTableModel *LogTableView::model() const
{
    return (LogTableModel *)QTableView::model();
}

void LogTableView::setIndexToDiff(bool forward)
{
    QModelIndex newIndex = model()->findDiff(currentIndex(), forward);
    if (newIndex.isValid())
        setCurrentIndex(newIndex);
}
