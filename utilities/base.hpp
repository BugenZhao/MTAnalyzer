//
// Created by Bugen Zhao on 2019/12/4.
//

#ifndef FINALPROJECT_BASE_HPP
#define FINALPROJECT_BASE_HPP

#include <QVector>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QPair>

using Adj=QVector<QVector<bool>>;

using Record=QString;

using BData=QPair<QPointF, QString>;
using BDataList=QList<BData>;
using BDataTable=QList<BDataList>;

using FilterData=QPair<QString, QMap<QString, bool>>;
using FilterDataList=QList<FilterData>;

namespace BugenZhao {
    const QString DATE_TIME_FORMAT = "yyyy-MM-dd hh:mm:ss";
    const QString DATE_TIME_FORMAT_NO_SEC = "yyyy-MM-dd hh:mm";
    const QString TIME_FORMAT_NO_SEC = "hh:mm";
    const QString DATE_FORMAT = "yyyy-MM-dd";

    template<typename BSequence>
    auto bAverage(const BSequence &sequence) {
        using V=typename BSequence::value_type;
        V sum = V();
        for (const auto &item:sequence) sum += item;
        return sum / sequence.size();
    }

    const bool BZ_DEBUG = false;
}


#endif //FINALPROJECT_BASE_HPP
