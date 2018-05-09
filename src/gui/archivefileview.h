#ifndef ARCHIVEFILEVIEW_H
#define ARCHIVEFILEVIEW_H

#include <QImage>
#include <QTableView>

class ArchiveFileModel;

class ArchiveFileView : public QTableView {
    Q_OBJECT

    public:
        ArchiveFileView(QWidget * parent = nullptr);

        void setModel(ArchiveFileModel * model);

    signals:
        void filesChanged();

    protected:
        void dragEnterEvent(QDragEnterEvent * event) override;
        void dragLeaveEvent(QDragLeaveEvent * event) override;
        void dragMoveEvent(QDragMoveEvent * event) override;
        void dropEvent(QDropEvent * event) override;

    private:
        ArchiveFileModel * m_model = nullptr;
        QImage m_logo;
};

#endif // ARCHIVEFILEVIEW_H
