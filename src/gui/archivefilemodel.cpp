#include <algorithm>

#include <QBrush>
#include <QCollator>
#include <QDir>
#include <QFileIconProvider>
#include <QIcon>
#include <QItemSelection>
#include <QMimeDatabase>

#include "accountstate.h"
#include "archivefilemodel.h"
#include "common/utility.h"
#include "folderman.h"
#include "syncengine.h"

using OCC::AccountPtr;
using OCC::Folder;
using OCC::FolderMan;
using OCC::SyncResult;


ArchiveFileModel::ArchiveFileModel(QObject * parent) : QAbstractTableModel(parent) {}

ArchiveFileModel::~ArchiveFileModel() {}


void ArchiveFileModel::addFile(const QFileInfo& file_info) {
    if (this->m_files.indexOf(file_info) >= 0)
        return;

    Folder * folder = FolderMan::instance()->folderForPath(file_info.absoluteFilePath());
    if (folder == nullptr)
        return;

    AccountPtr other_account = folder->accountState()->account();
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
    foreach (const ArchiveFile& file, this->m_files) {
        OCC::SyncFileStatus::SyncFileStatusTag status = this->fileStatus(file.info().absoluteFilePath());
        switch(status) {
            case OCC::SyncFileStatus::StatusNone:
            case OCC::SyncFileStatus::StatusWarning:
            case OCC::SyncFileStatus::StatusError:
                return false;

            default:
                continue;
        }
    }

    return true;
}

void ArchiveFileModel::clear() {
    this->beginResetModel();
    this->m_files.clear();
    this->m_account.clear();
    this->endResetModel();

    this->recomputeSize();
}

int ArchiveFileModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    else
        return 4;
}

QVariant ArchiveFileModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() or index.row() < 0)
        return QVariant();

    static QMimeDatabase db_mime_type;
    if (index.row() < this->m_files.length()) {
        const ArchiveFile& file = this->m_files[index.row()];
        switch (role) {
            case Qt::BackgroundRole: {
                OCC::SyncFileStatus::SyncFileStatusTag status = this->fileStatus(file.info().absoluteFilePath());
                switch(status) {
                    case OCC::SyncFileStatus::StatusNone:
                    case OCC::SyncFileStatus::StatusWarning:
                    case OCC::SyncFileStatus::StatusError:
                        return QBrush(Qt::red);

                    case OCC::SyncFileStatus::StatusSync:
                        return QBrush(Qt::yellow);

                    default:
                        return QVariant();
                }
            }

            case Qt::ForegroundRole: {
                OCC::SyncFileStatus::SyncFileStatusTag status = this->fileStatus(file.info().absoluteFilePath());
                switch(status) {
                    case OCC::SyncFileStatus::StatusNone:
                    case OCC::SyncFileStatus::StatusWarning:
                    case OCC::SyncFileStatus::StatusError:
                        return QBrush(Qt::white);

                    default:
                        return QVariant();
                }
            }

            default:
                break;
        }

        switch (index.column()) {
            case 0:
                switch (role) {
                    case Qt::DecorationRole: {
                        QMimeType type = db_mime_type.mimeTypeForFile(file.info(), QMimeDatabase::MatchExtension);
                        if (QIcon::hasThemeIcon(type.iconName()))
                            return QIcon::fromTheme(type.iconName());
                        else {
                           QFileIconProvider icons;
                           if (file.info().isDir())
                               return icons.icon(QFileIconProvider::Folder);
                           else
                               return icons.icon(QFileIconProvider::File);
                        }
                    }

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
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

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;

            case 3: {
                switch (role) {
                    case Qt::DisplayRole: {
                        OCC::SyncFileStatus::SyncFileStatusTag status = this->fileStatus(file.info().absoluteFilePath());
                        switch(status) {
                            case OCC::SyncFileStatus::StatusNone:
                                return tr("Not synchonized");

                            case OCC::SyncFileStatus::StatusSync:
                                return tr("Synchonizing");

                            case OCC::SyncFileStatus::StatusWarning:
                                return tr("Warning");

                            case OCC::SyncFileStatus::StatusUpToDate:
                                return tr("Synchonized");

                            default:
                                return tr("Error");
                        }
                    }

                    case Qt::TextAlignmentRole:
                        return Qt::AlignCenter;
                }
                break;
            }
        }
    }

    return QVariant();
}

QJsonArray ArchiveFileModel::files(const QString& remote_dir) const {
    FolderMan * monitor = FolderMan::instance();
    QJsonArray files;
    foreach (const ArchiveFile& file, this->m_files) {
        Folder * folder = monitor->folderForPath(file.info().absoluteFilePath());

        QDir dir(remote_dir + folder->remotePath());
        files << dir.filePath(file.info().absoluteFilePath().mid(folder->path().length()));
    }
    return files;
}

OCC::SyncFileStatus::SyncFileStatusTag ArchiveFileModel::fileStatus(const QString& path) {
    Folder * folder = FolderMan::instance()->folderForPath(path);
    if (folder == nullptr)
        return OCC::SyncFileStatus::StatusError;

    QString systemPath = QDir::cleanPath(path);
    QString relativePath = systemPath.mid(folder->cleanPath().length() + 1);
    return folder->syncEngine().syncFileStatusTracker().fileStatus(relativePath).tag();
}

QVariant ArchiveFileModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole or orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case 0:
            return tr("Type");

        case 1:
            return tr("Name");

        case 2:
            return tr("Size");

        case 3:
            return tr("Status");

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

    if (this->m_files.length() == 0)
        this->m_account.clear();

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

void ArchiveFileModel::sort(int column, Qt::SortOrder order) {
    emit this->layoutAboutToBeChanged();

    switch (column) {
        // sort by type
        case 0: {
            QMimeDatabase db_mime_type;
            if (order == Qt::AscendingOrder)
                std::sort(this->m_files.begin(), this->m_files.end(), [&db_mime_type](const ArchiveFile& a, const ArchiveFile& b) {
                    QMimeType type_a = db_mime_type.mimeTypeForFile(a.info(), QMimeDatabase::MatchExtension);
                    QMimeType type_b = db_mime_type.mimeTypeForFile(b.info(), QMimeDatabase::MatchExtension);
                    return type_a.name() < type_b.name();
                });
            else
                std::sort(this->m_files.begin(), this->m_files.end(), [&db_mime_type](const ArchiveFile& a, const ArchiveFile& b) {
                    QMimeType type_a = db_mime_type.mimeTypeForFile(a.info(), QMimeDatabase::MatchExtension);
                    QMimeType type_b = db_mime_type.mimeTypeForFile(b.info(), QMimeDatabase::MatchExtension);
                    return type_a.name() > type_b.name();
                });
            break;
        }

        // sort by filename
        case 1: {
            QCollator collator;
            if (order == Qt::AscendingOrder)
                std::sort(this->m_files.begin(), this->m_files.end(), [&collator](const ArchiveFile& a, const ArchiveFile& b) { return collator.compare(a.info().fileName(), b.info().fileName()) < 0; });
            else
                std::sort(this->m_files.begin(), this->m_files.end(), [&collator](const ArchiveFile& a, const ArchiveFile& b) { return collator.compare(a.info().fileName(), b.info().fileName()) > 0; });
            break;
        }

        // sort by size
        case 2:
            if (order == Qt::AscendingOrder)
                std::sort(this->m_files.begin(), this->m_files.end(), [](const ArchiveFile& a, const ArchiveFile& b) { return a.size() < b.size(); });
            else
                std::sort(this->m_files.begin(), this->m_files.end(), [](const ArchiveFile& a, const ArchiveFile& b) { return b.size() < a.size(); });
            break;

        case 3: {
            FolderMan * fm = FolderMan::instance();
            if (order == Qt::AscendingOrder)
                std::sort(this->m_files.begin(), this->m_files.end(), [&fm](const ArchiveFile& a, const ArchiveFile& b) {
                    Folder * fa = fm->folderForPath(a.info().absoluteFilePath());
                    Folder * fb = fm->folderForPath(b.info().absoluteFilePath());
                    switch (fa->syncResult().status()) {
                        case SyncResult::NotYetStarted:
                            return fb->syncResult().status() != SyncResult::NotYetStarted;

                        case SyncResult::SyncPrepare:
                        case SyncResult::SyncRunning:
                            switch (fb->syncResult().status()) {
                                case SyncResult::NotYetStarted:
                                case SyncResult::SyncPrepare:
                                case SyncResult::SyncRunning:
                                    return false;

                                default:
                                    return true;
                            }

                        case SyncResult::Success:
                            switch (fb->syncResult().status()) {
                                case SyncResult::NotYetStarted:
                                case SyncResult::SyncPrepare:
                                case SyncResult::SyncRunning:
                                case SyncResult::Success:
                                    return false;

                                default:
                                    return true;
                            }

                        case SyncResult::Paused:
                            switch (fb->syncResult().status()) {
                                case SyncResult::NotYetStarted:
                                case SyncResult::SyncPrepare:
                                case SyncResult::SyncRunning:
                                case SyncResult::Success:
                                case SyncResult::Paused:
                                    return false;

                                default:
                                    return true;
                            }

                        default:
                            return false;
                    }
                });
            }
    }

    emit this->layoutChanged();
}
