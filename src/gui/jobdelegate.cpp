#include <QApplication>
#include <QStyle>

#include "jobdelegate.h"

JobDelegate::JobDelegate(QObject * parent) : QItemDelegate(parent) {}


void JobDelegate::paint(QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;

    int progress = 100 * index.model()->data(index).toFloat();

    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.progress = progress;
    progressBarOption.text = QString("%1%").arg(progress);
    progressBarOption.textVisible = true;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
