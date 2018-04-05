#include <QIcon>
#include <QMimeDatabase>

#include "archivefilemodel.h"
#include "common/utility.h"


ArchiveFileModel::ArchiveFileModel(QObject * parent) : QAbstractTableModel(parent) {}

ArchiveFileModel::~ArchiveFileModel() {}


void ArchiveFileModel::addFile(const QFileInfo& file_info) {
    if (this->m_files.indexOf(file_info) >= 0)
        return;

    int nb_rows = this->m_files.size();
    this->beginInsertRows(QModelIndex(), nb_rows, nb_rows);
    this->m_files << file_info;
    this->endInsertRows();

    if (this->m_compute != nullptr)
        return;

    this->m_compute = new ArchiveFileComputeSize(*this, this);
    connect(this->m_compute, SIGNAL(completed(quint64, quint64, quint64)), SLOT(sizeCompute(quint64, quint64, quint64)));

    this->m_compute->start(QThread::LowestPriority);

    emit this->startComputeSize();
}

ArchiveFile& ArchiveFileModel::archiveFile(int index) {
    return this->m_files[index];
}

int ArchiveFileModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    else
        return 3;
}

QVariant ArchiveFileModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() or index.row() < 0)
        return QVariant();

    static QMimeDatabase db_mime_type;
    if (index.row() < this->m_files.length()) {
        const ArchiveFile& file = this->m_files[index.row()];
        switch (index.column()) {
            case 0:
                switch (role) {
                    case Qt::DecorationRole:
                        QMimeType type = db_mime_type.mimeTypeForFile(file.info(), QMimeDatabase::MatchExtension);
                        return QIcon::fromTheme(type.iconName());
                }
                break;

            case 1: {
                switch (role) {
                    case Qt::DisplayRole:
                        return file.info().fileName();
                }
                break;
            }

            case 2:
                switch (role) {
                    case Qt::DisplayRole:
                        return OCC::Utility::octetsToString(file.size());
                }
                break;
        }
    }

    return QVariant();
}

QVariant ArchiveFileModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole or orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case 1:
            return tr("Name");

        case 2:
            return tr("Size");

        default:
            return QVariant();
    }
}

int ArchiveFileModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    else
        return this->m_files.length();
}

void ArchiveFileModel::sizeCompute(quint64 size, quint64 nb_files, quint64 nb_directories) {
    this->m_compute->deleteLater();
    this->m_compute = nullptr;

    emit this->sizeComputed(size, nb_files, nb_directories);
}
