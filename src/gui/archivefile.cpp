#include "archivefile.h"

ArchiveFile::ArchiveFile(const QFileInfo& info) : m_file_info(info) {}


quint64 ArchiveFile::size() const {
    if (this->m_file_info.isFile())
        return this->m_file_info.size();
    else
        return this->m_file_size;
}
