#ifndef LOGTABLEMODEL_H
#define LOGTABLEMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include <QSet>
#include <tuple>

class LogTableModel : public QAbstractTableModel
{
public:
    // IMPORTANT: Make sure the labels in HEAD_LABELS matches that of
    // HeaderIndex.  If you modify HeaderIndex, remember to modify HEAD_LABELS
    // and vice versa!
    enum HeaderIndex
    {
        HEAD_FRATE,
        HEAD_MSEC,
        HEAD_HP,
        HEAD_AP,
        HEAD_HSPD,
        HEAD_ANG,
        HEAD_VSPD,
        HEAD_OG,
        HEAD_DST,
        HEAD_DUCK,
        HEAD_JUMP,
        HEAD_FMOVE,
        HEAD_SMOVE,
        HEAD_UMOVE,
        HEAD_YAW,
        HEAD_PITCH,
        HEAD_USE,
        HEAD_ATTACK,
        HEAD_ATTACK2,
        HEAD_RELOAD,
        HEAD_WLVL,
        HEAD_LADDER,
        HEAD_POSX,
        HEAD_POSY,
        HEAD_POSZ,

        // This stays at the end
        HEAD_LENGTH,
    };
    const QString HEAD_LABELS[HEAD_LENGTH] = {
        "fr", "ms", "hp", "ap", "hspd", "ang", "vspd", "og", "ds", "d", "j",
        "fm", "sm", "um", "yaw", "pitch", "u", "a", "a2", "rl", "wl", "ld",
        "px", "py", "pz"
    };

    LogTableModel(QObject *parent);
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;
    bool parseLogFile(const QString &logFileName);
    void clearAllRows();
    QModelIndex findDiff(const QModelIndex &curIndex, bool forward) const;
    float sumDuration(int startRow, int endRow) const;

private:
    // We will be assuming that every QVector in this array has equal length.
    QVector<QVariant> logTableData[HEAD_LENGTH];
    QVector<unsigned int> frameNums;
    QHash<unsigned int, QPair<float, unsigned int>> damages;
    QHash<unsigned int, QPair<float, float>> punchangles;
    QHash<unsigned int, QPair<float, float>> hbasevels;
    QHash<unsigned int, float> vbasevels;
    QHash<unsigned int, std::tuple<bool, float, float>> objmoves;
    QHash<unsigned int, QStringList> extralines;
    QHash<unsigned int, float> explddists;
    QSet<unsigned int> numtouches;
    QFont italicFont;
    QFont boldFont;
};

#endif
