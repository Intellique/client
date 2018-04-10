#include <QDir>
#include <QIcon>
#include <QItemSelection>
#include <QMimeDatabase>

#include "accountstate.h"
#include "archivefilemodel.h"
#include "common/utility.h"
#include "folderman.h"


ArchiveFileModel::ArchiveFileModel(QObject * parent) : QAbstractTableModel(parent) {}

ArchiveFileModel::~ArchiveFileModel() {}


void ArchiveFileModel::addFile(const QFileInfo& file_info) {
    if (this->m_files.indexOf(file_info) >= 0)
        return;

    OCC::Folder * folder = OCC::FolderMan::instance()->folderForPath(file_info.absoluteFilePath());
    if (folder == nullptr)
        return;

    OCC::AccountPtr other_account = folder->accountState()->account();
    if (this->m_account != nullptr and this->m_account != other_account)
        return;

    if (this->m_account == nullptr)
        this->m_account = other_account;

    for (int i = 0; i < this->m_files.length(); i++) {
        const QFileInfo& file = this->m_files[i].info();

        if (file_info.absoluteFilePath().startsWith(file.absoluteFilePath()))
            return;

        if (file.absoluteFilePath().startsWith(file_info.absoluteFilePath())) {
            this->beginRemoveRows(QModelIndex(), i, i);
            this->m_files.removeAt(i);
            this->endRemoveRows();
            i--;
        }
    }

    int nb_rows = this->m_files.size();
    this->beginInsertRows(QModelIndex(), nb_rows, nb_rows);
    this->m_files << file_info;
    this->endInsertRows();

    this->recomputeSize();
}

ArchiveFile& ArchiveFileModel::archiveFile(int index) {
    return this->m_files[index];
}

bool ArchiveFileModel::canCreateArchive() {
    OCC::FolderMan * monitor = OCC::FolderMan::instance();
    foreach (const ArchiveFile& file, this->m_files) {
        OCC::Folder * folder = monitor->folderForPath(file.info().absoluteFilePath());
        if (folder->syncResult().status() != OCC::SyncResult::Success)
            return false;
    }

    return true;
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

QJsonArray ArchiveFileModel::files(const QString& remote_dir) const {
    OCC::FolderMan * monitor = OCC::FolderMan::instance();
    QJsonArray files;
    foreach (const ArchiveFile& file, this->m_files) {
        OCC::Folder * folder = monitor->folderForPath(file.info().absoluteFilePath());

        QDir dir(remote_dir + folder->remotePath());
        files << dir.filePath(file.info().absoluteFilePath().mid(folder->path().length()));
    }
    return files;
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

void ArchiveFileModel::recomputeSize() {
    if (this->m_compute != nullptr)
        return;

    this->m_compute = new ArchiveFileComputeSize(*this, this);
    connect(this->m_compute, SIGNAL(completed(quint64, quint64, quint64)), SLOT(sizeCompute(quint64, quint64, quint64)));

    this->m_compute->start(QThread::LowestPriority);

    emit this->startComputeSize();
}

void ArchiveFileModel::removeSelection(const QItemSelection& selection) {
    if (selection.length() == 0)
        return;

    QModelIndex parent;
    foreach (const QItemSelectionRange range, selection) {
        this->beginRemoveRows(parent, range.top(), range.bottom());
        this->m_files.erase(this->m_files.begin() + range.top(), this->m_files.begin() + range.bottom() + 1);
        this->endRemoveRows();
    }

    this->recomputeSize();
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
