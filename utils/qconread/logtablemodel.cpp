#include <QBrush>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <cmath>
#include "logtablemodel.h"

using std::hypot;
using std::atan2;
using std::sqrt;

static const QHash<int, QString> DMG_STRING = {
    {1, "crush"}, {1 << 1, "bullet"}, {1 << 2, "slash"}, {1 << 3, "burn"},
    {1 << 4, "freeze"}, {1 << 5, "fall"}, {1 << 6, "blast"}, {1 << 7, "club"},
    {1 << 8, "shock"}, {1 << 9, "sonic"}, {1 << 10, "energybeam"},
    {1 << 14, "drown"}, {1 << 18, "radiation"}, {1 << 20, "acid"}
};

static const float M_RAD2DEG = 180 / M_PI;

static const unsigned int IN_ATTACK = 1 << 0;
static const unsigned int IN_JUMP = 1 << 1;
static const unsigned int IN_DUCK = 1 << 2;
static const unsigned int IN_FORWARD = 1 << 3;
static const unsigned int IN_BACK = 1 << 4;
static const unsigned int IN_USE = 1 << 5;
static const unsigned int IN_MOVELEFT = 1 << 9;
static const unsigned int IN_MOVERIGHT = 1 << 10;
static const unsigned int IN_ATTACK2 = 1 << 11;
static const unsigned int IN_RELOAD = 1 << 13;
static const unsigned int FL_DUCKING = 1 << 14;

static const QBrush brushBlack = QBrush(Qt::black);
static const QBrush brushWhite = QBrush(Qt::white);
static const QBrush brushRed = QBrush(Qt::red);
static const QBrush brushMoveRed = QBrush(QColor(255, 100, 100));
static const QBrush brushLRed = QBrush(QColor(255, 230, 230));
static const QBrush brushBlue = QBrush(Qt::blue);
static const QBrush brushMoveBlue = QBrush(QColor(100, 100, 255));
static const QBrush brushDimBlue = QBrush(QColor(200, 200, 255));
static const QBrush brushOgGreen = QBrush(QColor(150, 255, 150));
static const QBrush brushDGreen = QBrush(QColor(0, 120, 0));
static const QBrush brushFrGray = QBrush(Qt::darkGray);
static const QBrush brushDckGray = QBrush(Qt::gray);
static const QBrush brushMagenta = QBrush(QColor(255, 130, 230));
static const QBrush brushLMagenta = QBrush(QColor(230, 200, 255));
static const QBrush brushCyan = QBrush(QColor(80, 255, 255));
static const QBrush brushYellow = QBrush(QColor(255, 230, 100));
static const QBrush brushBrown = QBrush(QColor(180, 100, 0));

LogTableModel::LogTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    italicFont.setItalic(true);
    boldFont.setBold(true);
}

int LogTableModel::columnCount(const QModelIndex &) const
{
    return HEAD_LENGTH;
}

int LogTableModel::rowCount(const QModelIndex &) const
{
    return frameNums.length();
}

QVariant LogTableModel::data(const QModelIndex &index, int role) const
{
    QString basevelStr;
    QString objmoveStr;
    int duckState;
    int waterLevel;
    int moveVal;

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case HEAD_OG:
        case HEAD_DST:
        case HEAD_DUCK:
        case HEAD_JUMP:
        case HEAD_FMOVE:
        case HEAD_SMOVE:
        case HEAD_UMOVE:
        case HEAD_USE:
        case HEAD_ATTACK:
        case HEAD_ATTACK2:
        case HEAD_RELOAD:
        case HEAD_WLVL:
        case HEAD_LADDER:
            return QVariant();
        case HEAD_HSPD:
        case HEAD_ANG:
            if (!hbasevels.contains(index.row()))
                break;
            return '*' + QString::number(logTableData[index.column()]
                                         [index.row()].toFloat());
        case HEAD_VSPD:
            if (!vbasevels.contains(index.row()))
                break;
            return '*' + QString::number(logTableData[index.column()]
                                         [index.row()].toFloat());
        }
        return logTableData[index.column()][index.row()];

    case Qt::ForegroundRole:
        switch (index.column()) {
        case HEAD_HP:
            return damages.contains(index.row()) ? brushWhite : brushRed;
        case HEAD_AP:
            return damages.contains(index.row()) ? brushWhite : brushBlue;
        case HEAD_FRATE:
        case HEAD_MSEC:
            return brushFrGray;
        }
        break;

    case Qt::BackgroundRole:
        switch (index.column()) {
        case HEAD_HSPD:
        case HEAD_ANG:
        case HEAD_VSPD:
            if (numtouches.contains(index.row()))
                return brushLRed;
            break;
        case HEAD_DST:
            duckState = logTableData[HEAD_DST][index.row()].toInt();
            if (duckState == 2)
                return brushBlack;
            else if (duckState == 1)
                return brushDckGray;
            break;
        case HEAD_DUCK:
            if (logTableData[HEAD_DUCK][index.row()].toBool())
                return brushMagenta;
            break;
        case HEAD_JUMP:
            if (logTableData[HEAD_JUMP][index.row()].toBool())
                return brushCyan;
            break;
        case HEAD_FMOVE:
        case HEAD_SMOVE:
        case HEAD_UMOVE:
            moveVal = logTableData[index.column()][index.row()].toInt();
            if (moveVal > 0)
                return brushMoveBlue;
            else if (moveVal < 0)
                return brushMoveRed;
            break;
        case HEAD_PITCH:
            if (punchangles.contains(index.row()) &&
                punchangles[index.row()].first)
                return brushLMagenta;
            break;
        case HEAD_YAW:
            if (punchangles.contains(index.row()) &&
                punchangles[index.row()].second)
                return brushLMagenta;
            break;
        case HEAD_USE:
        case HEAD_ATTACK:
        case HEAD_ATTACK2:
        case HEAD_RELOAD:
            if (logTableData[index.column()][index.row()].toBool())
                return brushYellow;
            break;
        case HEAD_HP:
        case HEAD_AP:
            if (damages.contains(index.row()))
                return brushRed;
            break;
        case HEAD_OG:
            if (logTableData[HEAD_OG][index.row()].toBool())
                return brushOgGreen;
            break;
        case HEAD_WLVL:
            waterLevel = logTableData[HEAD_WLVL][index.row()].toInt();
            if (waterLevel >= 2)
                return brushBlue;
            else if (waterLevel == 1)
                return brushDimBlue;
            break;
        case HEAD_LADDER:
            if (logTableData[HEAD_LADDER][index.row()].toBool())
                return brushBrown;
        }
        break;

    case Qt::FontRole:
        switch (index.column()) {
        case HEAD_HSPD:
        case HEAD_ANG:
            if (objmoves.contains(index.row()))
                return boldFont;
            break;
        case HEAD_FMOVE:
        case HEAD_SMOVE:
        case HEAD_UMOVE:
            return boldFont;
        case HEAD_HP:
        case HEAD_AP:
            if (damages.contains(index.row()))
                return boldFont;
            break;
        }
        break;

    case Qt::StatusTipRole:
        switch (index.column()) {
        case HEAD_HSPD:
        case HEAD_ANG:
            if (hbasevels.contains(index.row())) {
                auto entry = hbasevels[index.row()];
                basevelStr = QString("(with basevel) hspd = %1, ang = %2")
                    .arg(entry.first).arg(entry.second);
            }
            if (objmoves.contains(index.row())) {
                auto entry = objmoves[index.row()];
                objmoveStr = QString("push = %1, objhspd = %2, objang = %3")
                    .arg(std::get<0>(entry) ? "yes" : "no")
                    .arg(hypotf(std::get<1>(entry), std::get<2>(entry)))
                    .arg(atan2f(std::get<2>(entry), std::get<1>(entry))
                         * M_RAD2DEG);
            }
            if (!basevelStr.isNull() && objmoveStr.isNull())
                return basevelStr;
            else if (basevelStr.isNull() && !objmoveStr.isNull())
                return objmoveStr;
            else if (!basevelStr.isNull() && !objmoveStr.isNull())
                return basevelStr + QString(" | ") + objmoveStr;
            break;
        case HEAD_VSPD:
            if (vbasevels.contains(index.row()))
                return QString("vertical basevel = %1")
                    .arg(vbasevels[index.row()]);
            break;
        case HEAD_PITCH:
            if (punchangles.contains(index.row()))
                return QString("punchpitch = %1")
                    .arg(punchangles[index.row()].first);
            break;
        case HEAD_YAW:
            if (punchangles.contains(index.row()))
                return QString("punchyaw = %1")
                    .arg(punchangles[index.row()].second);
            break;
        case HEAD_HP:
        case HEAD_AP:
            if (damages.contains(index.row())) {
                auto entry = damages[index.row()];
                QString blastDistStr;
                QStringList dmgStrList;
                for (int flag : DMG_STRING.uniqueKeys()) {
                    if (entry.second & flag)
                        dmgStrList.append(DMG_STRING[flag]);
                }
                if (dmgStrList.isEmpty())
                    dmgStrList.append(entry.second ? "other" : "generic");
                else if (entry.second & (1 << 6))
                    blastDistStr = QString(" dist = %1")
                        .arg(explddists[index.row()]);
                return QString("damage = %1 (%2)%3").arg(entry.first)
                    .arg(dmgStrList.join(", ")).arg(blastDistStr);
            }
            break;
        }
        break;
    }

    return QVariant();
}

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (role == Qt::FontRole && orientation == Qt::Vertical) {
        if (extralines.contains(section))
            return boldFont;
        else
            return italicFont;
    }

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            if (extralines.contains(section))
                return '*' + QString::number(frameNums[section]);
            else
                return frameNums[section];
        } else {
            return HEAD_LABELS[section];
        }
    }

    if (role == Qt::UserRole && orientation == Qt::Vertical &&
        extralines.contains(section)) {
        return extralines[section].join('\n');
    }

    return QVariant();
}

bool LogTableModel::parseLogFile(const QString &logFileName)
{
    QFile logFile(logFileName);
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    float velocity[3];
    float basevel[3] = {0, 0, 0};
    float tmppangs[2];
    unsigned int flags;
    int readState = 0;
    QStringList tokens;
    QTextStream textStream(&logFile);
    for (;;) {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        switch (readState) {
        case 0:
            if (line.startsWith("prethink ")) {
                tokens = line.split(' ');
                frameNums.append(tokens[1].toUInt());
                logTableData[HEAD_FRATE].append(1 / tokens[2].toFloat());
                readState = 1;
                continue;
            } else if (line.startsWith("dmg ")) {
                tokens = line.split(' ');
                damages[frameNums.length() - 1] = qMakePair(
                    tokens[1].toFloat(), tokens[2].toUInt());
                continue;
            } else if (line.startsWith("obj ")) {
                tokens = line.split(' ');
                objmoves[frameNums.length() - 1] = std::make_tuple(
                    tokens[1][0] != '0', tokens[2].toFloat(),
                    tokens[3].toFloat());
                continue;
            } else if (line.startsWith("expld ")) {
                tokens = line.split(' ');
                float disp[3] = {tokens[7].toFloat() - tokens[1].toFloat(),
                                 tokens[8].toFloat() - tokens[2].toFloat(),
                                 tokens[9].toFloat() - tokens[3].toFloat()};
                explddists[frameNums.length() - 1] = sqrt(disp[0] * disp[0] +
                                                          disp[1] * disp[1] +
                                                          disp[2] * disp[2]);
                continue;
            }
            break;
        case 1:
            if (!line.startsWith("health "))
                break;
            tokens = line.split(' ');
            logTableData[HEAD_HP].append(tokens[1].toFloat());
            logTableData[HEAD_AP].append(tokens[2].toFloat());
            readState = 2;
            continue;
        case 2:
            if (!line.startsWith("usercmd "))
                break;
            tokens = line.split(' ');
            logTableData[HEAD_MSEC].append(tokens[1].toShort());
            flags = tokens[2].toUInt();
            logTableData[HEAD_DUCK].append((flags & IN_DUCK) != 0);
            logTableData[HEAD_JUMP].append((flags & IN_JUMP) != 0);
            logTableData[HEAD_USE].append((flags & IN_USE) != 0);
            logTableData[HEAD_ATTACK].append((flags & IN_ATTACK) != 0);
            logTableData[HEAD_ATTACK2].append((flags & IN_ATTACK2) != 0);
            logTableData[HEAD_RELOAD].append((flags & IN_RELOAD) != 0);
            logTableData[HEAD_PITCH].append(tokens[3].toFloat());
            logTableData[HEAD_YAW].append(tokens[4].toFloat());
            readState = 3;
            continue;
        case 3:
            if (!line.startsWith("fsu "))
                break;
            tokens = line.split(' ');
            logTableData[HEAD_FMOVE].append(tokens[1].toInt());
            logTableData[HEAD_SMOVE].append(tokens[2].toInt());
            logTableData[HEAD_UMOVE].append(tokens[3].toInt());
            readState = 4;
            continue;
        case 4:
            if (!line.startsWith("fg "))
                break;
            readState = 5;
            continue;
        case 5:
            if (!line.startsWith("pa "))
                break;
            tokens = line.split(' ');
            tmppangs[0] = tokens[1].toFloat();
            tmppangs[1] = tokens[2].toFloat();
            if (tmppangs[0] || tmppangs[1])
                punchangles[frameNums.length() - 1] =
                    qMakePair(tmppangs[0], tmppangs[1]);
            readState = 6;
            continue;
        case 6:
            if (!line.startsWith("pmove 1 "))
                break;
            tokens = line.split(' ');
            basevel[0] = tokens[5].toFloat();
            basevel[1] = tokens[6].toFloat();
            basevel[2] = tokens[7].toFloat();
            if (basevel[2])
                vbasevels[frameNums.length() - 1] = basevel[2];
            readState = 7;
            continue;
        case 7:
            if (!line.startsWith("ntl "))
                break;
            tokens = line.split(' ');
            logTableData[HEAD_LADDER].append(tokens[2] != "0");
            if (tokens[1] != "0")
                numtouches.insert(frameNums.length() - 1);
            readState = 8;
            continue;
        case 8:
            if (!line.startsWith("pos 2 "))
                break;
            tokens = line.split(' ');
            logTableData[HEAD_POSX].append(tokens[2].toFloat());
            logTableData[HEAD_POSY].append(tokens[3].toFloat());
            logTableData[HEAD_POSZ].append(tokens[4].toFloat());
            readState = 9;
            continue;
        case 9:
            if (!line.startsWith("pmove 2 "))
                break;
            tokens = line.split(' ');
            velocity[0] = tokens[2].toFloat();
            velocity[1] = tokens[3].toFloat();
            velocity[2] = tokens[4].toFloat();
            logTableData[HEAD_HSPD].append(hypotf(velocity[0], velocity[1]));
            logTableData[HEAD_ANG].append(atan2f(velocity[1], velocity[0])
                                          * M_RAD2DEG);
            logTableData[HEAD_VSPD].append(velocity[2]);
            logTableData[HEAD_OG].append(tokens[10] != "-1");
            if (tokens[9].toUInt() & FL_DUCKING)
                logTableData[HEAD_DST].append(2);
            else if (tokens[8][0] != '0')
                logTableData[HEAD_DST].append(1);
            else
                logTableData[HEAD_DST].append(0);
            logTableData[HEAD_WLVL].append(tokens[11].toShort());
            if (basevel[0] || basevel[1]) {
                velocity[0] += basevel[0];
                velocity[1] += basevel[1];
                hbasevels[frameNums.length() - 1] = qMakePair(
                    hypotf(velocity[0], velocity[1]),
                    atan2f(velocity[1], velocity[0]) * M_RAD2DEG);
            }
            readState = 0;
            continue;
        }

        if (line.startsWith("pos"))
            continue;

        extralines[frameNums.length() - 1].append(line);
    }

    if (extralines.contains(-1)){
        extralines[-1].append(extralines.value(0, QStringList()));
        extralines[0].swap(extralines[-1]);
        extralines.remove(-1);
    }

    beginInsertRows(QModelIndex(), 0, frameNums.length() - 1);
    endInsertRows();

    return true;
}

void LogTableModel::clearAllRows()
{
    beginRemoveRows(QModelIndex(), 0, frameNums.length() - 1);
    for (int i = 0; i < HEAD_LENGTH; i++) {
        logTableData[i].clear();
    }
    frameNums.clear();
    damages.clear();
    punchangles.clear();
    hbasevels.clear();
    vbasevels.clear();
    objmoves.clear();
    numtouches.clear();
    extralines.clear();
    endRemoveRows();
}

QModelIndex LogTableModel::findDiff(const QModelIndex &curIndex,
                                    bool forward) const
{
    if (!curIndex.isValid())
        return QModelIndex();

    auto searchList = logTableData[curIndex.column()];
    const QVariant &curVal = searchList[curIndex.row()];
    if (forward) {
        for (int row = curIndex.row() + 1; row < searchList.length(); row++) {
            if (searchList[row] != curVal)
                return createIndex(row, curIndex.column());
        }
    } else {
        for (int row = curIndex.row() - 1; row >= 0; row--) {
            if (searchList[row] != curVal)
                return createIndex(row, curIndex.column());
        }
    }

    return QModelIndex();
}
