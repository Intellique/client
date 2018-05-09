#ifndef JOBMODEL_H
#define JOBMODEL_H

#include <QAbstractTableModel>
#include <QMap>

#include "job.h"

class JobModel : public QAbstractTableModel {
        Q_OBJECT

    public:
        explicit JobModel(QObject * parent = nullptr);

        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        void setJob(const Job& job);
        void setJobList(const QList<int>& jobs);
        void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    private:
        QList<int> m_display_order;
        mutable QMap<int, Job> m_jobs;
};

#endif // JOBMODEL_H
