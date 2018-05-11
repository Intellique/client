#include "accountmanager.h"
#include "archiveapi.h"
#include "creds/httpcredentialsgui.h"
#include "joblogmodel.h"
#include "joblogwidget.h"
#include "ui_joblogwidget.h"

using OCC::AccountManager;
using OCC::ArchivalAuthJob;
using OCC::ArchivalCheckConnectionJob;
using OCC::ArchivalJobInfoJob;
using OCC::ArchivalJobsJob;
using OCC::HttpCredentials;

JobLogWidget::JobLogWidget(QWidget * parent) : QWidget(parent), ui(new Ui::JobLogWidget) {
    ui->setupUi(this);

    this->model = new JobLogModel(this);
    this->ui->tblVwJobs->setModel(this->model);

    // resize header
    QHeaderView * header = this->ui->tblVwJobs->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    QTimer::singleShot(5000, this, SLOT(updateModel()));
}

JobLogWidget::~JobLogWidget() {
    delete this->ui;
    delete this->model;
}


void JobLogWidget::doAuthForUpdate() {
    OCC::AbstractCredentials * cred = this->account->credentials();
    HttpCredentials * httpCreds = qobject_cast<HttpCredentials *>(cred);

    if (httpCreds != nullptr) {
        if (not httpCreds->wasFetched()) {
            connect(this->account.data(), SIGNAL(credentialsFetched(AbstractCredentials *)), SLOT(doAuthForUpdate()));
            httpCreds->fetchFromKeychain();
            return;
        } else {
            ArchivalAuthJob * job = new ArchivalAuthJob(this->account, httpCreds->password(), this);
            connect(job, SIGNAL(connectionFailure()), SLOT(doAuthForUpdate()));
            connect(job, SIGNAL(connectionSuccess(int)), SLOT(fetchJobs(int)));
            job->start();
        }
    }
}

void JobLogWidget::fetchJobs(int user_id) {
    ArchivalJobsJob * job = new ArchivalJobsJob(this->account, user_id, false, this->ui->spnBxMaxJobs->value(), this);
    // connect(job, SIGNAL(fetchFailure()), SLOT(???));
    connect(job, SIGNAL(jobsFetch(const QList<int>&)), SLOT(jobs(const QList<int>&)));
    connect(job, SIGNAL(notConnected()), SLOT(doAuthForUpdate()));
    job->start();
}

void JobLogWidget::fetchNextJob() {
    if (this->jobs_remain.size() > 0) {
        int job_id = this->jobs_remain.first();
        this->jobs_remain.removeFirst();

        ArchivalJobInfoJob * job = new ArchivalJobInfoJob(this->account, job_id, this);
        // connect(job, SIGNAL(fetchFailure()), SLOT(???));
        connect(job, SIGNAL(jobInfo(const Job&)), SLOT(job(const Job&)));
        connect(job, SIGNAL(notConnected()), SLOT(doAuthForUpdate()));
        job->start();
    } else
        QTimer::singleShot(5000, this, SLOT(updateModel()));
}

void JobLogWidget::job(const Job& job) {
    this->model->setJob(job);
    this->fetchNextJob();
    this->ui->tblVwJobs->setSortingEnabled(true);
}

void JobLogWidget::jobs(const QList<int>& jobs) {
    this->jobs_remain = jobs;
    this->model->setJobList(this->jobs_remain);
    this->fetchNextJob();
}

void JobLogWidget::updateModel() {
    AccountManager * mng = AccountManager::instance();
    if (mng->accounts().length() > 0) {
        OCC::AccountStatePtr ptr = mng->accounts().first();
        this->account = ptr->account();

        ArchivalCheckConnectionJob * job = new ArchivalCheckConnectionJob(this->account, this);
        connect(job, SIGNAL(authenticationRequire()), SLOT(doAuthForUpdate()));
        connect(job, SIGNAL(connectionAlive(int)), SLOT(fetchJobs(int)));
        job->start();
    } else
        QTimer::singleShot(5000, this, SLOT(updateModel()));
}
