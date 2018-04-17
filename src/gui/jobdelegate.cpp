#include <QApplication>
#include <QPainter>
#include <QStyle>

#include "jobdelegate.h"

JobDelegate::JobDelegate(QObject * parent) : QItemDelegate(parent) {}


void JobDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;

    int progress = 100 * index.data().toFloat();

    progressBarOption.state = option.state;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    if (progress >= 0) {
        progressBarOption.progress = progress;
        progressBarOption.text = QString("%1%").arg(progress);
    } else {
        progressBarOption.progress = -1;
    }

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
