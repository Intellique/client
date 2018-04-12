#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include <QWidget>

namespace Ui {
    class ArchiveWidget;
}

class ArchiveFileModel;
class QItemSelection;

namespace OCC {
    class ArchiveWidget : public QWidget {
            Q_OBJECT

        public:
            explicit ArchiveWidget(QWidget * parent = nullptr);
            ~ArchiveWidget();

        private slots:
            void addFiles();
            void archiveCreated();
            void archiveCreationFailure();
            void authenticationFailure();
            void checkCanArchive();
            void createArchive();
            void doAuthentication();
            void doCreateArchive(int pool_id);
            void getUserInfo(int user_id);
            void noPoolFound();
            void removeFiles();
            void searchPool(QJsonObject user_info);
            void searchPoolFailure();
            void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
            void startUpdatingSize();
            void sizeUpdated(quint64 size, quint64 nb_files, quint64 nb_directories);
            void userInfoFailure();

        private:
            ::Ui::ArchiveWidget * ui;
            ArchiveFileModel * model;
            bool redo_auth = true;
            QString user_home_directory;
    };
}

#endif // ARCHIVEWIDGET_H
