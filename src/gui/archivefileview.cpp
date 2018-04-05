#include <QDragEnterEvent>
#include <QMimeData>

#include "archivefilemodel.h"
#include "archivefileview.h"
#include "folderman.h"

ArchiveFileView::ArchiveFileView(QWidget * parent) : QTableView(parent) {
    this->setAcceptDrops(true);
}


void ArchiveFileView::dragEnterEvent(QDragEnterEvent * event) {
    const QMimeData * mime_data = event->mimeData();
    if (mime_data->hasFormat("text/plain")) {
        OCC::FolderMan * fm = OCC::FolderMan::instance();
        QStringList files = mime_data->text().split('\n', QString::SkipEmptyParts);
        bool ok = true;
        foreach (QUrl url, files) {
            if (fm->folderForPath(url.path()) == nullptr)
                ok = false;
        }

        if (ok)
            event->acceptProposedAction();
    }
}

void ArchiveFileView::dragLeaveEvent(QDragLeaveEvent * event) {
    event->accept();
}

void ArchiveFileView::dragMoveEvent(QDragMoveEvent * event) {
    event->acceptProposedAction();
}

void ArchiveFileView::dropEvent(QDropEvent * event) {
    const QMimeData * mime_data = event->mimeData();
    if (mime_data->hasFormat("text/plain")) {
        OCC::FolderMan * fm = OCC::FolderMan::instance();
        QStringList files = mime_data->text().split('\n', QString::SkipEmptyParts);
        bool ok = false;
        foreach (QUrl url, files) {
            if (fm->folderForPath(url.path()) != nullptr) {
                this->m_model->addFile(url.path());
                ok = true;
            }
        }

        if (ok)
            event->acceptProposedAction();
    }
}

void ArchiveFileView::setModel(ArchiveFileModel * model) {
    this->m_model = model;
    QTableView::setModel(model);
}
