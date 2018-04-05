#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include <QWidget>

namespace Ui {
    class ArchiveWidget;
}

class ArchiveFileModel;

namespace OCC {
    class ArchiveWidget : public QWidget {
            Q_OBJECT

        public:
            explicit ArchiveWidget(QWidget * parent = nullptr);
            ~ArchiveWidget();

        private slots:
            void addFiles();
            void startUpdatingSize();
            void sizeUpdated(quint64 size, quint64 nb_files, quint64 nb_directories);

        private:
            ::Ui::ArchiveWidget * ui;
            ArchiveFileModel * model;
    };
}

#endif // ARCHIVEWIDGET_H
