#include <QFileDialog>
#include <QStringList>

#include "archivefilemodel.h"
#include "archivewidget.h"
#include "folderman.h"
#include "settingsdialog.h"
#include "ui_archivewidget.h"

using OCC::ArchiveWidget;

ArchiveWidget::ArchiveWidget(QWidget *parent) : QWidget(parent), ui(new ::Ui::ArchiveWidget) {
    ui->setupUi(this);

    this->model = new ArchiveFileModel(this);
    this->ui->tblVwArchiveFile->setModel(this->model);

    connect(this->model, SIGNAL(sizeComputed(quint64, quint64, quint64)), SLOT(sizeUpdated(quint64, quint64, quint64)));
    connect(this->model, SIGNAL(startComputeSize()), SLOT(startUpdatingSize()));
}

ArchiveWidget::~ArchiveWidget() {
    delete ui;
}


void ArchiveWidget::addFiles() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec() == 0)
        return;

    QStringList files = dialog.selectedFiles();
    for (int i = 0; i < files.length(); i++) {
        Folder * folder = FolderMan::instance()->folderForPath(files[i]);
        if (folder == nullptr)
            continue;

        const QFileInfo file_info = files[i];
        this->model->addFile(file_info);
    }
}


void ArchiveWidget::startUpdatingSize() {
    this->ui->lblSize->setText(tr("Computing size..."));
}

void ArchiveWidget::sizeUpdated(quint64 size, quint64 nb_files, quint64 nb_directories) {
    QString text = tr("There is %1 and %2 for %3")
        .arg(tr("%n file(s)", "", nb_files), tr("%n directory(ies)", "", nb_directories), Utility::octetsToString(size)
    );
    this->ui->lblSize->setText(text);
}
