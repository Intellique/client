#ifndef ARCHIVEFILEMODEL_H
#define ARCHIVEFILEMODEL_H

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QList>

#include "account.h"
#include "archivefile.h"
#include "archivefilecomputesize.h"

class QItemSelection;

class ArchiveFileModel : public QAbstractTableModel {
    Q_OBJECT

    public:
        explicit ArchiveFileModel(QObject * parent = nullptr);
        virtual ~ArchiveFileModel();

        inline OCC::AccountPtr& account() {
            return this->m_account;
        }
        inline const OCC::AccountPtr& account() const {
            return this->m_account;
        }
        void addFile(const QFileInfo& file_info);
        bool canCreateArchive();
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QJsonArray files(const QString& remote_dir) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        void removeSelection(const QItemSelection& selection);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    signals:
        void sizeComputed(quint64 size, quint64 nb_files, quint64 nb_dictories);
        void startComputeSize();

    private slots:
        void sizeCompute(quint64 size, quint64 nb_files, quint64 nb_directories);

    private:
        ArchiveFile& archiveFile(int index);

        void recomputeSize();

        QList<ArchiveFile> m_files;
        ArchiveFileComputeSize * m_compute = nullptr;

        OCC::AccountPtr m_account;

        friend class ArchiveFileComputeSize;
};

#endif // ARCHIVEFILEMODEL_H
