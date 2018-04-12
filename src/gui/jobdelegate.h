#ifndef JOBDELEGATE_H
#define JOBDELEGATE_H

#include <QItemDelegate>

class JobDelegate : public QItemDelegate {
    public:
        JobDelegate(QObject * parent = nullptr);

        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

#endif // JOBDELEGATE_H
