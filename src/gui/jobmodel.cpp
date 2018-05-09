#include <algorithm>

#include <QCollator>
#include <QLocale>
#include <QJsonArray>

#include "jobmodel.h"

JobModel::JobModel(QObject * parent) : QAbstractTableModel(parent) {}


int JobModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    else
        return 7;
}

QVariant JobModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() < this->m_jobs.size()) {
        int current_key = this->m_display_order.at(index.row());
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
                        return job.type();

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;

            case 2:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.name();
                }
                break;

            case 3:
                switch (role) {
                    case Qt::DisplayRole:
                        return QLocale::system().toString(job.startTime(), QLocale::ShortFormat);

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;

            case 4:
                switch (role) {
                    case Qt::DisplayRole:
                        if (job.done() > 0)
                            return job.done();
                        else
                            return -1;
                }
                break;

            case 5:
                switch (role) {
                    case Qt::DisplayRole:
                        return job.etaToString();

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;

            case 6:
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
            return tr("Type");

        case 2:
            return tr("Name");

        case 3:
            return tr("Start time");

        case 4:
            return tr("Progress");

        case 5:
            return tr("ETA");

        case 6:
            return tr("Status");

        default:
            return QVariant();
    }
}

QModelIndex JobModel::index(int row, int column, const QModelIndex& parent) const {
    if (not parent.isValid() and row < this->m_jobs.size()) {
        int current_key = this->m_display_order.at(row);
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
        int pos = this->m_display_order.indexOf(job.id());
        this->m_jobs[job.id()] = job;
        this->dataChanged(this->index(pos, 0), this->index(pos, 5));
    } else {
        int row = this->m_jobs.size();
        this->beginInsertRows(QModelIndex(), row, row);
        this->m_display_order.append(job.id());
        this->m_jobs[job.id()] = job;
        this->endInsertRows();
    }
}

void JobModel::setJobList(const QList<int>& jobs) {
    for (int pos = 0; pos < this->m_display_order.length(); pos++) {
        int key = this->m_display_order[pos];
        if (not jobs.contains(key)) {
            this->beginRemoveRows(QModelIndex(), pos, pos);
            this->m_jobs.remove(key);
            this->m_display_order.removeAt(pos);
            this->endRemoveRows();

            pos--;
        }
    }
}

void JobModel::sort(int column, Qt::SortOrder order) {
    emit this->layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

    switch (column) {
        // sort by id
        case 0:
            if (order == Qt::AscendingOrder)
                std::sort(this->m_display_order.begin(), this->m_display_order.end());
            else
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), std::greater<int>());
            break;

        // sort by name
        case 1: {
            QCollator col;
            col.setCaseSensitivity(Qt::CaseInsensitive);
            col.setNumericMode(true);

            if (order == Qt::AscendingOrder)
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this, &col](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    int diff = col.compare(job_a.type(), job_b.type());
                    if (diff == 0)
                        return a < b;
                    else
                        return diff < 0;
                });
            else
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this, &col](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    int diff = col.compare(job_a.type(), job_b.type());
                    if (diff == 0)
                        return a > b;
                    else
                        return diff > 0;
                });
            break;
        }

        // sort by name
        case 2: {
            QCollator col;
            col.setCaseSensitivity(Qt::CaseInsensitive);
            col.setNumericMode(true);

            if (order == Qt::AscendingOrder)
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this, &col](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    int diff = col.compare(job_a.name(), job_b.name());
                    if (diff == 0)
                        return a < b;
                    else
                        return diff < 0;
                });
            else
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this, &col](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    int diff = col.compare(job_a.name(), job_b.name());
                    if (diff == 0)
                        return a > b;
                    else
                        return diff > 0;
                });
            break;
        }

        // sort by start time
        case 3:
            if (order == Qt::AscendingOrder)
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.startTime() == job_b.startTime())
                        return a < b;
                    else
                        return job_a.startTime() < job_b.startTime();
                });
            else
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.startTime() == job_b.startTime())
                        return a > b;
                    else
                        return job_a.startTime() > job_b.startTime();
                });
            break;

        // sort by progression
        case 4:
            if (order == Qt::AscendingOrder)
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.done() == job_b.done())
                        return a < b;
                    else
                        return job_a.done() < job_b.done();
                });
            else
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.done() == job_b.done())
                        return a > b;
                    else
                        return job_a.done() > job_b.done();
                });
            break;

        // sort by eta
        case 5:
            if (order == Qt::AscendingOrder)
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.eta() == job_b.eta())
                        return a < b;
                    else
                        return job_a.eta() < job_b.eta();
                });
            else
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.eta() == job_b.eta())
                        return a > b;
                    else
                        return job_a.eta() > job_b.eta();
                });
            break;

        // sort by status
        case 6:
            if (order == Qt::AscendingOrder)
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.status() == job_b.status())
                        return a < b;
                    else
                        return job_a.status() < job_b.status();
                });
            else
                std::sort(this->m_display_order.begin(), this->m_display_order.end(), [this](int a, int b) {
                    const Job& job_a = this->m_jobs[a];
                    const Job& job_b = this->m_jobs[b];

                    if (job_a.status() == job_b.status())
                        return a > b;
                    else
                        return job_a.status() > job_b.status();
                });
            break;
    }

    emit this->layoutChanged();
}
