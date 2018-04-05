#ifndef ARCHIVEFILEMODEL_H
#define ARCHIVEFILEMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include "archivefile.h"
#include "archivefilecomputesize.h"

class ArchiveFileModel : public QAbstractTableModel {
    Q_OBJECT

    public:
        explicit ArchiveFileModel(QObject * parent = nullptr);
        virtual ~ArchiveFileModel();

        void addFile(const QFileInfo& file_info);
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    signals:
        void sizeComputed(quint64 size, quint64 nb_files, quint64 nb_dictories);
        void startComputeSize();

    private slots:
        void sizeCompute(quint64 size, quint64 nb_files, quint64 nb_directories);

    private:
        ArchiveFile& archiveFile(int index);

        QList<ArchiveFile> m_files;
        ArchiveFileComputeSize * m_compute = nullptr;

        friend class ArchiveFileComputeSize;
};

#endif // ARCHIVEFILEMODEL_H
