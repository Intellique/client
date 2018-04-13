#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QStringList>

#include "account.h"
#include "archiveapi.h"
#include "archivefilemodel.h"
#include "archivewidget.h"
#include "creds/httpcredentialsgui.h"
#include "folderman.h"
#include "settingsdialog.h"
#include "ui_archivewidget.h"

using OCC::ArchivalAuthJob;
using OCC::ArchivalCreateArchiveJob;
using OCC::ArchivalCheckConnectionJob;
using OCC::ArchivalUserInfoJob;
using OCC::ArchiveWidget;

ArchiveWidget::ArchiveWidget(QWidget *parent) : QWidget(parent), ui(new ::Ui::ArchiveWidget) {
    ui->setupUi(this);

    this->model = new ArchiveFileModel(this);
    this->ui->tblVwArchiveFile->setModel(this->model);

    connect(this->model, SIGNAL(sizeComputed(quint64, quint64, quint64)), SLOT(sizeUpdated(quint64, quint64, quint64)));
    connect(this->model, SIGNAL(startComputeSize()), SLOT(startUpdatingSize()));
    connect(this->ui->pshBttnRemoveFiles, SIGNAL(clicked()), SLOT(removeFiles()));
    connect(this->ui->bttnCreateArchive, SIGNAL(clicked()), SLOT(createArchive()));
    connect(this->ui->lnEdtArchiveName, SIGNAL(textChanged(const QString&)), SLOT(checkCanArchive()));
    connect(this->ui->tblVwArchiveFile->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));

    // add support for DEL keystroke
    QShortcut * del = new QShortcut(QKeySequence::Delete, this);
    connect(del, SIGNAL(activated()), SLOT(removeFiles()));

    // resize header
    QHeaderView * header = this->ui->tblVwArchiveFile->horizontalHeader();
    header->setSectionResizeMode(1, QHeaderView::Stretch);
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

    this->checkCanArchive();
}

void ArchiveWidget::archiveCreated() {
    QMessageBox message;
    message.setIcon(QMessageBox::Information);
    message.setText(tr("Your archival task \"%1\" has been created.").arg(this->ui->lnEdtArchiveName->text()));
    message.setInformativeText(tr("The task will start soon."));
    message.setStandardButtons(QMessageBox::Ok);
    message.exec();

    this->ui->lnEdtArchiveName->clear();
    this->model->clear();

    emit this->newArchive();
}

void ArchiveWidget::archiveCreationFailure() {
    QMessageBox message;
    message.setIcon(QMessageBox::Critical);
    message.setText(tr("Failed to create your archival task"));
    message.setInformativeText(tr("Please contact Intellique for further information"));
    message.setStandardButtons(QMessageBox::Ok);
    message.exec();
}

void ArchiveWidget::authenticationFailure() {
    QMessageBox message;
    message.setIcon(QMessageBox::Critical);
    message.setText(tr("Failed to login into archival backend."));
    message.setInformativeText(tr("Please contact Intellique for further information"));
    message.setStandardButtons(QMessageBox::Ok);
    message.exec();
}

void ArchiveWidget::checkCanArchive() {
    bool ok = true;
    if (this->ui->lnEdtArchiveName->text().simplified().length() == 0 or this->model->rowCount() == 0 or not this->model->canCreateArchive())
        ok = false;
    this->ui->bttnCreateArchive->setEnabled(ok);
}

void ArchiveWidget::createArchive() {
    this->redo_auth = true;

    ArchivalCheckConnectionJob * job = new ArchivalCheckConnectionJob(this->model->account(), this);
    connect(job, SIGNAL(authenticationRequire()), SLOT(doAuthentication()));
    connect(job, SIGNAL(connectionAlive(int)), SLOT(getUserInfo(int)));
    job->start();
}

void ArchiveWidget::doAuthentication() {
    this->redo_auth = false;

    OCC::AbstractCredentials * cred = this->model->account()->credentials();
    HttpCredentials * httpCreds = qobject_cast<HttpCredentials *>(cred);

    if (httpCreds != nullptr) {
        if (not httpCreds->wasFetched()) {
            connect(this->model->account().data(), SIGNAL(credentialsFetched(AbstractCredentials *)), SLOT(doAuthentication()));
            httpCreds->fetchFromKeychain();
            return;
        } else {
            ArchivalAuthJob * job = new ArchivalAuthJob(this->model->account(), httpCreds->password(), this);
            connect(job, SIGNAL(connectionFailure()), SLOT(authenticationFailure()));
            connect(job, SIGNAL(connectionSuccess(int)), SLOT(getUserInfo(int)));
            job->start();
        }
    }
}

void ArchiveWidget::doCreateArchive(int pool_id) {
    ArchivalCreateArchiveJob * job = new ArchivalCreateArchiveJob(this->ui->lnEdtArchiveName->text(), this->user_home_directory, this->model, pool_id, this->redo_auth, this);
    connect(job, SIGNAL(archiveCreated()), SLOT(archiveCreated()));
    connect(job, SIGNAL(creationFailure()), SLOT(archiveCreationFailure()));
    connect(job, SIGNAL(notConnected()), SLOT(doAuthentication()));
    job->start();
}

void ArchiveWidget::getUserInfo(int user_id) {
    ArchivalUserInfoJob * job = new ArchivalUserInfoJob(this->model->account(), user_id, this->redo_auth, this);
    connect(job, SIGNAL(notConnected()), SLOT(doAuthentication()));
    connect(job, SIGNAL(userInfoComplete(QJsonObject)), SLOT(searchPool(QJsonObject)));
    connect(job, SIGNAL(userInfoFailure()), SLOT(userInfoFailure()));
    job->start();
}

void ArchiveWidget::noPoolFound() {
    QMessageBox message;
    message.setIcon(QMessageBox::Critical);
    message.setText(tr("There is no pool available."));
    message.setInformativeText(tr("Please contact Intellique for further information"));
    message.setStandardButtons(QMessageBox::Ok);
    message.exec();
}

void ArchiveWidget::removeFiles() {
    this->model->removeSelection(this->ui->tblVwArchiveFile->selectionModel()->selection());
}

void ArchiveWidget::searchPool(QJsonObject user_info) {
    this->user_home_directory = user_info["homedirectory"].toString();

    ArchivalSearchPoolJob * job = new ArchivalSearchPoolJob(this->model->account(), user_info["poolgroup"].toInt(), this->redo_auth, this);
    connect(job, SIGNAL(noConnected()), SLOT(doAuthentication()));
    connect(job, SIGNAL(poolFound(int)), SLOT(doCreateArchive(int)));
    connect(job, SIGNAL(poolNotFound()), SLOT(noPoolFound()));
    connect(job, SIGNAL(searchPoolFailure()), SLOT(searchPoolFailure()));
    job->start();
}

void ArchiveWidget::searchPoolFailure() {
    QMessageBox message;
    message.setIcon(QMessageBox::Critical);
    message.setText(tr("An error occured while creating your archival task."));
    message.setInformativeText(tr("Please contact Intellique for further information"));
    message.setStandardButtons(QMessageBox::Ok);
    message.exec();
}

void ArchiveWidget::selectionChanged(const QItemSelection& selected, const QItemSelection&) {
    this->ui->pshBttnRemoveFiles->setEnabled(selected.indexes().length() != 0);
}

void ArchiveWidget::sizeUpdated(quint64 size, quint64 nb_files, quint64 nb_directories) {
    QString text = tr("There is %1 and %2 for %3")
        .arg(tr("%n file(s)", "", nb_files), tr("%n directory(ies)", "", nb_directories), Utility::octetsToString(size)
    );
    this->ui->lblSize->setText(text);
}

void ArchiveWidget::startUpdatingSize() {
    this->ui->lblSize->setText(tr("Computing size..."));
}

void ArchiveWidget::userInfoFailure() {
    QMessageBox message;
    message.setIcon(QMessageBox::Critical);
    message.setText(tr("An error occured while creating your archival task."));
    message.setInformativeText(tr("Please contact Intellique for further information"));
    message.setStandardButtons(QMessageBox::Ok);
    message.exec();
}
