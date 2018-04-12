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

/*
void ArchiveFileView::paintEvent(QPaintEvent * event) {
    // QTableView::paintEvent(event);

    QSize image_size = this->m_logo.size();
    QSize this_size = this->size();
    QPoint pos((this_size.width() - image_size.width()) / 2, (this_size.height() - image_size.height()) / 2);
    QRect target(pos, image_size);

    QPainter painter(this);

    QTableView::render(&painter);



    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setOpacity(0.25);
    painter.drawImage(event->rect(), this->m_logo, event->rect());
    painter.restore();
}
*/

void ArchiveFileView::setModel(ArchiveFileModel * model) {
    this->m_model = model;
    QTableView::setModel(model);
}
