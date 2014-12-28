#include <QBrush>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
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
        case HEAD_FRATE:
            return logTableData[index.row()].frate;
        case HEAD_MSEC:
            return logTableData[index.row()].msec;
        case HEAD_HP:
            return logTableData[index.row()].hp;
        case HEAD_AP:
            return logTableData[index.row()].ap;
        case HEAD_YAW:
            return logTableData[index.row()].yaw;
        case HEAD_PITCH:
            return logTableData[index.row()].pitch;
        case HEAD_POSX:
            return logTableData[index.row()].posx;
        case HEAD_POSY:
            return logTableData[index.row()].posy;
        case HEAD_POSZ:
            return logTableData[index.row()].posz;
        case HEAD_HSPD:
            if (hbasevels.contains(index.row()))
                return '*' + QString::number(logTableData[index.row()].hspd);
            else
                return logTableData[index.row()].hspd;
        case HEAD_ANG:
            if (hbasevels.contains(index.row()))
                return '*' + QString::number(logTableData[index.row()].ang);
            else
                return logTableData[index.row()].ang;
        case HEAD_VSPD:
            if (vbasevels.contains(index.row()))
                return '*' + QString::number(logTableData[index.row()].vspd);
            else
                return logTableData[index.row()].vspd;
        }
        return QVariant();

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
            duckState = logTableData[index.row()].dst;
            if (duckState == 2)
                return brushBlack;
            else if (duckState == 1)
                return brushDckGray;
            break;
        case HEAD_DUCK:
            if (logTableData[index.row()].buttons & IN_DUCK)
                return brushMagenta;
            break;
        case HEAD_JUMP:
            if (logTableData[index.row()].buttons & IN_JUMP)
                return brushCyan;
            break;
        case HEAD_USE:
            if (logTableData[index.row()].buttons & IN_USE)
                return brushYellow;
            break;
        case HEAD_ATTACK:
            if (logTableData[index.row()].buttons & IN_ATTACK)
                return brushYellow;
            break;
        case HEAD_ATTACK2:
            if (logTableData[index.row()].buttons & IN_ATTACK2)
                return brushYellow;
            break;
        case HEAD_RELOAD:
            if (logTableData[index.row()].buttons & IN_RELOAD)
                return brushYellow;
            break;
        case HEAD_FMOVE:
            moveVal = logTableData[index.row()].fmove;
            if (moveVal > 0)
                return brushMoveBlue;
            else if (moveVal < 0)
                return brushMoveRed;
            break;
        case HEAD_SMOVE:
            moveVal = logTableData[index.row()].smove;
            if (moveVal > 0)
                return brushMoveBlue;
            else if (moveVal < 0)
                return brushMoveRed;
            break;
        case HEAD_UMOVE:
            moveVal = logTableData[index.row()].umove;
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
        case HEAD_HP:
        case HEAD_AP:
            if (damages.contains(index.row()))
                return brushRed;
            break;
        case HEAD_OG:
            if (logTableData[index.row()].og)
                return brushOgGreen;
            break;
        case HEAD_WLVL:
            waterLevel = logTableData[index.row()].wlvl;
            if (waterLevel >= 2)
                return brushBlue;
            else if (waterLevel == 1)
                return brushDimBlue;
            break;
        case HEAD_LADDER:
            if (logTableData[index.row()].ladder)
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
#define STARTTOK(n) tok = strtok_r(lineptr + (n), " ", &saveptr);
#define NEXTTOK tok = strtok_r(NULL, " ", &saveptr);

    FILE *logFile = std::fopen(logFileName.toUtf8().data(), "r");
    if (!logFile)
        return false;

    float basevel[3] = {0, 0, 0};
    int readState = 0;
    LogEntry logEntry;
    size_t n = 1024;
    char *lineptr = (char *)std::malloc(n);

    while (getline(&lineptr, &n, logFile) != -1) {
        char *saveptr;
        char *tok;

        switch (readState) {
        case 0:
            if (strncmp(lineptr, "prethink", 8) == 0) {
                STARTTOK(8);
                frameNums.append(std::atoi(tok));
                logTableData.append(logEntry);
                NEXTTOK;
                logTableData.last().frate = 1 / std::atof(tok);
                readState = 1;
                continue;
            } else if (strncmp(lineptr, "dmg", 3) == 0) {
                STARTTOK(3);
                float dmg = std::atof(tok);
                NEXTTOK;
                unsigned long bits = std::strtoul(tok, NULL, 10);
                damages[frameNums.length() - 1] = qMakePair(dmg, bits);
                continue;
            } else if (strncmp(lineptr, "obj", 3) == 0) {
                STARTTOK(3);
                bool push = *tok != '0';
                NEXTTOK;
                float velx = std::atof(tok);
                NEXTTOK;
                float vely = std::atof(tok);
                objmoves[frameNums.length() - 1] =
                    std::make_tuple(push, velx, vely);
                continue;
            } else if (strncmp(lineptr, "expld", 5) == 0) {
                float start[3];
                STARTTOK(5);
                start[0] = std::atof(tok);
                NEXTTOK;
                start[1] = std::atof(tok);
                NEXTTOK;
                start[2] = std::atof(tok);

                float end[3];
                NEXTTOK;
                NEXTTOK;
                NEXTTOK;
                NEXTTOK;
                end[0] = std::atof(tok);
                NEXTTOK;
                end[1] = std::atof(tok);
                NEXTTOK;
                end[2] = std::atof(tok);

                float disp[3] = {end[0] - start[0], end[1] - start[1],
                                 end[2] - start[2]};
                explddists[frameNums.length() - 1] = sqrt(
                    disp[0] * disp[0] + disp[1] * disp[1] + disp[2] * disp[2]);
                continue;
            }
            break;
        case 1:
            if (strncmp(lineptr, "health", 6) != 0)
                break;
            STARTTOK(6);
            logTableData.last().hp = std::atof(tok);
            NEXTTOK;
            logTableData.last().ap = std::atof(tok);
            readState = 2;
            continue;
        case 2:
            if (strncmp(lineptr, "usercmd", 7) != 0)
                break;
            STARTTOK(7);
            logTableData.last().msec = std::atoi(tok);
            NEXTTOK;
            logTableData.last().buttons = std::strtoul(tok, NULL, 10);
            NEXTTOK;
            logTableData.last().pitch = std::atof(tok);
            NEXTTOK;
            logTableData.last().yaw = std::atof(tok);
            readState = 3;
            continue;
        case 3:
            if (strncmp(lineptr, "fsu", 3) != 0)
                break;
            STARTTOK(3);
            logTableData.last().fmove = std::atoi(tok);
            NEXTTOK;
            logTableData.last().smove = std::atoi(tok);
            NEXTTOK;
            logTableData.last().umove = std::atoi(tok);
            readState = 4;
            continue;
        case 4:
            if (strncmp(lineptr, "fg", 2) != 0)
                break;
            readState = 5;
            continue;
        case 5: {
            if (strncmp(lineptr, "pa", 2) != 0)
                break;
            float tmppangs[2];
            STARTTOK(2);
            tmppangs[0] = std::atof(tok);
            NEXTTOK;
            tmppangs[1] = std::atof(tok);
            if (tmppangs[0] || tmppangs[1])
                punchangles[frameNums.length() - 1] =
                    qMakePair(tmppangs[0], tmppangs[1]);
            readState = 6;
            continue;
        }
        case 6:
            if (strncmp(lineptr, "pmove", 5) != 0)
                break;
            STARTTOK(5);
            if (*tok != '1')
                break;
            NEXTTOK;
            NEXTTOK;
            NEXTTOK;
            NEXTTOK;
            basevel[0] = std::atof(tok);
            NEXTTOK;
            basevel[1] = std::atof(tok);
            NEXTTOK;
            basevel[2] = std::atof(tok);
            if (basevel[2])
                vbasevels[frameNums.length() - 1] = basevel[2];
            readState = 7;
            continue;
        case 7:
            if (strncmp(lineptr, "ntl", 3) != 0)
                break;
            STARTTOK(3);
            if (*tok != '0')
                numtouches.insert(frameNums.length() - 1);
            NEXTTOK;
            logTableData.last().ladder = *tok != '0';
            readState = 8;
            continue;
        case 8:
            if (strncmp(lineptr, "pos", 3) != 0)
                break;
            STARTTOK(3);
            if (*tok != '2')
                break;
            NEXTTOK;
            logTableData.last().posx = std::atof(tok);
            NEXTTOK;
            logTableData.last().posy = std::atof(tok);
            NEXTTOK;
            logTableData.last().posz = std::atof(tok);
            readState = 9;
            continue;
        case 9: {
            if (strncmp(lineptr, "pmove", 5) != 0)
                break;
            STARTTOK(5);
            if (*tok != '2')
                break;

            float vel[3];
            NEXTTOK;
            vel[0] = std::atof(tok);
            NEXTTOK;
            vel[1] = std::atof(tok);
            NEXTTOK;
            vel[2] = std::atof(tok);
            logTableData.last().hspd = hypotf(vel[0], vel[1]);
            logTableData.last().ang = atan2f(vel[1], vel[0]) *
                M_RAD2DEG;
            logTableData.last().vspd = vel[2];

            NEXTTOK;
            NEXTTOK;
            NEXTTOK;
            NEXTTOK;
            bool bInDuck = *tok != '0';
            NEXTTOK;
            bool ducking = std::strtoul(tok, NULL, 10) & FL_DUCKING;
            if (ducking)
                logTableData.last().dst = 2;
            else if (bInDuck)
                logTableData.last().dst = 1;
            else
                logTableData.last().dst = 0;

            NEXTTOK;
            logTableData.last().og = std::atoi(tok) != -1;
            NEXTTOK;
            logTableData.last().wlvl = std::atoi(tok);
            if (basevel[0] || basevel[1]) {
                vel[0] += basevel[0];
                vel[1] += basevel[1];
                hbasevels[frameNums.length() - 1] = qMakePair(
                    hypotf(vel[0], vel[1]),
                    atan2f(vel[1], vel[0]) * M_RAD2DEG);
            }
            readState = 0;
            continue;
        }
        }

        if (strncmp(lineptr, "pos", 3) == 0 ||
            strncmp(lineptr, "cl_yawspeed", 11) == 0 ||
            strncmp(lineptr, "execing", 7) == 0)
            continue;

        extralines[frameNums.length() - 1].append(lineptr);
    }

    std::free(lineptr);
    std::fclose(logFile);

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
    logTableData.clear();
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

    // auto searchList = logTableData[curIndex.column()];
    // const QVariant &curVal = searchList[curIndex.row()];
    // if (forward) {
    //     for (int row = curIndex.row() + 1; row < searchList.length(); row++) {
    //         if (searchList[row] != curVal)
    //             return createIndex(row, curIndex.column());
    //     }
    // } else {
    //     for (int row = curIndex.row() - 1; row >= 0; row--) {
    //         if (searchList[row] != curVal)
    //             return createIndex(row, curIndex.column());
    //     }
    // }

    return QModelIndex();
}

float LogTableModel::sumDuration(int startRow, int endRow) const
{
    double duration = 0;
    for (int i = startRow; i <= endRow; i++) {
        duration += 1 / (double)logTableData[i].frate;
    }
    return duration;
}
