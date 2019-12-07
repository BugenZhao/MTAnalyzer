//
// Created by Bugen Zhao on 2019/12/7.
//

#include "BDateTime.h"

int BDateTime::bToLocalTimestamp(const QString &s) {
    if (!s.isEmpty()) {
        std::tm tm{};

        tm.tm_year = s.mid(0, 4).toInt() - 1900;
        tm.tm_mon = s.mid(5, 2).toInt() - 1;
        tm.tm_mday = s.mid(8, 2).toInt();
        tm.tm_hour = s.mid(11, 2).toInt();
        tm.tm_min = s.mid(14, 2).toInt();
        tm.tm_sec = s.mid(17, 2).toInt();

        return std::mktime(&tm);
    }
    return 0;
}
