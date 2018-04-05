#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include <QFileInfo>

class ArchiveFile {
    public:
        ArchiveFile(const QFileInfo& info);

        inline const QFileInfo& info() const {
            return this->m_file_info;
        }
        quint64 size() const;

        inline bool operator ==(const ArchiveFile& file) const {
            return this->m_file_info == file.m_file_info;
        }

    private:
        QFileInfo m_file_info;
        quint64 m_file_size = 0;

        bool m_computing = false;
        bool m_computed = false;
        quint64 m_nb_files = 0;
        quint64 m_nb_directories = 0;

        friend class ArchiveFileComputeSize;
};

#endif // ARCHIVEFILE_H
