//
// Created by Bugen Zhao on 2019/12/4.
//

#ifndef FINALPROJECT_BASE_HPP
#define FINALPROJECT_BASE_HPP

#include <QVector>

using Adj=QVector<QVector<bool>>;
using Record=QString;

namespace BugenZhao {
    const QString TIME_FORMAT = "yyyy-MM-dd hh:mm:ss";

    template<typename BSequence>
    auto average(const BSequence &sequence) {
        using V=typename BSequence::value_type;
        V sum = V();
        for (const auto &item:sequence) sum += item;
        return sum / sequence.size();
    }
}


#endif //FINALPROJECT_BASE_HPP
