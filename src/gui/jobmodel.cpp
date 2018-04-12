#include <QLocale>
#include <QJsonArray>

#include "jobmodel.h"

JobModel::JobModel(QObject * parent) : QAbstractTableModel(parent) {}


int JobModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    else
        return 5;
}

QVariant JobModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() < this->m_jobs.size()) {
        int current_key = this->m_jobs.keys().at(index.row());
        const Job& job = this->m_jobs[current_key];
        switch (index.column()) {
            case 0:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.name();
                }
                break;

            case 1:
                switch (role) {
                    case Qt::DisplayRole:
                        return QLocale::system().toString(job.startTime(), QLocale::ShortFormat);
                }
                break;

            case 2:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.done();
                }
                break;

            case 3:
                switch (role) {
                    case Qt::DisplayRole:
                        return QLocale::system().toString(job.endTime(), QLocale::ShortFormat);
                }
                break;

            case 4:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.statusString();
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
            return tr("Name");

        case 1:
            return tr("Start time");

        case 2:
            return tr("Progress");

        case 3:
            return tr("ETA");

        case 4:
            return tr("Status");

        default:
            return QVariant();
    }
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
    for (int pos = 0; pos < current_keys.length(); pos++) {
        int key = current_keys[pos];
        if (not jobs.contains(key)) {
            this->beginRemoveRows(QModelIndex(), pos, pos);
            this->m_jobs.remove(key);
            this->endRemoveRows();
            pos--;
        }
    }
}
