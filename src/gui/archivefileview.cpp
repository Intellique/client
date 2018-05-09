#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>

#include "archivefilemodel.h"
#include "archivefileview.h"
#include "folderman.h"

ArchiveFileView::ArchiveFileView(QWidget * parent) : QTableView(parent), m_logo(":/client/resources/upload-folder-button.png") {
    this->setAcceptDrops(true);
}


void ArchiveFileView::dragEnterEvent(QDragEnterEvent * event) {
    const QMimeData * mime_data = event->mimeData();
    if (mime_data == nullptr) {
        event->ignore();
        return;
    }

    if (mime_data->hasUrls()) {
        OCC::FolderMan * fm = OCC::FolderMan::instance();
        QList<QUrl> urls = mime_data->urls();
        bool ok = true;
        foreach (const QUrl url, urls) {
            QString path = url.path();
            if (path.at(2) == ':')
                path = path.mid(1);
            if (fm->folderForPath(path) == nullptr)
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
    if (mime_data == nullptr)
        return;

    if (mime_data->hasUrls()) {
        OCC::FolderMan * fm = OCC::FolderMan::instance();
        QList<QUrl> urls = mime_data->urls();
        bool ok = false;
        foreach (const QUrl url, urls) {
            QString path = url.path();
            if (path.at(2) == ':')
                path = path.mid(1);

            static QRegExp trailingSlash("[\\/]+$");
            if (path.endsWith('/') or path.endsWith('\\'))
                path.remove(trailingSlash);

            if (fm->folderForPath(path) != nullptr) {
                this->m_model->addFile(path);
                ok = true;
            }
        }

        if (ok) {
            event->acceptProposedAction();
            emit this->filesChanged();
        }
    }
}

void ArchiveFileView::setModel(ArchiveFileModel * model) {
    this->m_model = model;
    QTableView::setModel(model);
}
