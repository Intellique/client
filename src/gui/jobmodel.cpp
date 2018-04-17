#include <algorithm>

#include <QLocale>
#include <QJsonArray>

#include "jobmodel.h"

JobModel::JobModel(QObject * parent) : QAbstractTableModel(parent) {}


int JobModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    else
        return 6;
}

QVariant JobModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() < this->m_jobs.size()) {
        QList<int> current_keys = this->m_jobs.keys();
        std::sort(current_keys.rbegin(), current_keys.rend());
        int current_key = current_keys.at(index.row());
        const Job& job = this->m_jobs[current_key];
        switch (index.column()) {
            case 0:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.id();

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;

            case 1:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.name();
                }
                break;

            case 2:
                switch (role) {
                    case Qt::DisplayRole:
                        return QLocale::system().toString(job.startTime(), QLocale::ShortFormat);

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;

            case 3:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.done();
                }
                break;

            case 4:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.eta();

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;

            case 5:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.statusString();

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;
        }
    }

    return QVariant();
}

QVariant JobModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole or orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case 0:
            return "#";

        case 1:
            return tr("Name");

        case 2:
            return tr("Start time");

        case 3:
            return tr("Progress");

        case 4:
            return tr("ETA");

        case 5:
            return tr("Status");

        default:
            return QVariant();
    }
}

QModelIndex JobModel::index(int row, int column, const QModelIndex& parent) const {
    if (not parent.isValid() and row < this->m_jobs.size()) {
        int current_key = this->m_jobs.keys().at(row);
        return createIndex(row, column, &this->m_jobs[current_key]);
    } else
        return QModelIndex();
}

int JobModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    else
        return this->m_jobs.size();
}

void JobModel::setJob(const Job& job) {
    if (job.is_null())
        return;

    if (this->m_jobs.contains(job.id())) {
        int pos = this->m_jobs.keys().indexOf(job.id());
        this->m_jobs[job.id()] = job;
        this->dataChanged(this->index(pos, 0), this->index(pos, 5));
    } else {
        int row = this->m_jobs.size();
        this->beginInsertRows(QModelIndex(), row, row);
        this->m_jobs[job.id()] = job;
        this->endInsertRows();
    }
}

void JobModel::setJobList(const QList<int> &jobs) {
    QList<int> current_keys = this->m_jobs.keys();
    std::sort(current_keys.rbegin(), current_keys.rend());
    for (int pos = 0; pos < current_keys.length(); pos++) {
        int key = current_keys[pos];
        if (not jobs.contains(key)) {
            this->beginRemoveRows(QModelIndex(), pos, pos);
            this->m_jobs.remove(key);
            this->endRemoveRows();
        }
    }
}
