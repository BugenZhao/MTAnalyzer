//
// Created by Bugen Zhao on 2019/12/7.
//

#ifndef FINALPROJECT_BDATETIME_H
#define FINALPROJECT_BDATETIME_H

#include <QString>
#include <QDateTime>
#include <ctime>

#if _WIN32
#include <windows.h>
#endif

class BDateTime {
public:
    static int bToLocalTimestamp(const QString &s);
};


#endif //FINALPROJECT_BDATETIME_H
