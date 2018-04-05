#include <QDirIterator>
#include <QFileInfo>

#include "archivefilecomputesize.h"
#include "archivefilemodel.h"

ArchiveFileComputeSize::ArchiveFileComputeSize(ArchiveFileModel& model, QObject * parent) : QThread(parent), m_model(model) { }

ArchiveFileComputeSize::~ArchiveFileComputeSize() {
    this->m_lock.lock();
    if (not this->m_stop)
        this->quit();
    this->m_lock.unlock();
}


void ArchiveFileComputeSize::run() {
    quint64 total_size = 0, total_nb_files = 0, total_nb_directories = 0;

    for (int i = 0; i < this->m_model.rowCount(); i++) {
        this->m_lock.lock();
        bool should_exit = this->m_stop;
        this->m_lock.unlock();

        if (should_exit)
            return;

        ArchiveFile& af = this->m_model.archiveFile(i);

        if (af.m_computed) {
            total_size += af.m_file_size;
            total_nb_files += af.m_nb_files;
            total_nb_directories += af.m_nb_directories;
            continue;
        }

        quint64 size = 0, nb_files = 0, nb_directories = 0;
        const QFileInfo& file_info = af.info();
        if (file_info.isFile()) {
            nb_files++;
            size += file_info.size();
        } else if (file_info.isDir()) {
            nb_directories++;

            QDirIterator iter(file_info.absoluteFilePath(), QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                QFileInfo info = iter.next();
                if (info.isFile()) {
                    nb_files++;
                    size += info.size();
                } else if (info.isDir())
                    nb_directories++;

                this->m_lock.lock();
                should_exit = this->m_stop;
                this->m_lock.unlock();

                if (should_exit)
                    return;
            }
        }

        af.m_computed = true;
        total_size += af.m_file_size = size;
        total_nb_files += af.m_nb_files = nb_files;
        total_nb_directories += af.m_nb_directories = nb_directories;
    }

    this->m_stop = true;
    emit this->completed(total_size, total_nb_files, total_nb_directories);
}
