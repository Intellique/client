#ifndef ARCHIVEFILEVIEW_H
#define ARCHIVEFILEVIEW_H

#include <QTableView>

class ArchiveFileModel;

class ArchiveFileView : public QTableView {
    public:
        ArchiveFileView(QWidget * parent = nullptr);

        void setModel(ArchiveFileModel * model);

    protected:
        void dragEnterEvent(QDragEnterEvent * event) override;
        void dragLeaveEvent(QDragLeaveEvent * event) override;
        void dragMoveEvent(QDragMoveEvent * event) override;
        void dropEvent(QDropEvent * event) override;

    private:
        ArchiveFileModel * m_model = nullptr;
};

#endif // ARCHIVEFILEVIEW_H
