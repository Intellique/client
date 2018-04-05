#ifndef ARCHIVEFILECOMPUTESIZE_H
#define ARCHIVEFILECOMPUTESIZE_H

#include <QList>
#include <QThread>
#include <QMutex>

class ArchiveFile;
class ArchiveFileModel;

class ArchiveFileComputeSize : public QThread {
    Q_OBJECT

    public:
        ArchiveFileComputeSize(ArchiveFileModel& model, QObject * parent = nullptr);
        virtual ~ArchiveFileComputeSize();

    signals:
        void completed(quint64 size, quint64 nb_files, quint64 nb_directories);

    protected:
        void run() override;

    private:
        ArchiveFileModel& m_model;
        QMutex m_lock;
        volatile bool m_stop = false;
};

#endif // ARCHIVEFILECOMPUTESIZE_H
