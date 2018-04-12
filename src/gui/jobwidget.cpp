#include <algorithm>

#include <QTimer>

#include "accountmanager.h"
#include "archiveapi.h"
#include "configfile.h"
#include "creds/httpcredentialsgui.h"
#include "jobdelegate.h"
#include "jobmodel.h"
#include "jobwidget.h"
#include "theme.h"
#include "ui_jobwidget.h"

using OCC::AbstractCredentials;
using OCC::AccountManager;
using OCC::AccountStatePtr;
using OCC::ArchivalAuthJob;
using OCC::ArchivalCheckConnectionJob;
using OCC::ArchivalJobInfoJob;
using OCC::ArchivalJobsJob;
using OCC::HttpCredentials;

JobWidget::JobWidget(QWidget *parent) : QWidget(parent), ui(new Ui::JobWidget) {
    this->ui->setupUi(this);

    this->model = new JobModel(this);
    this->ui->tblVwJobs->setItemDelegateForColumn(2, new JobDelegate(this));
    this->ui->tblVwJobs->setModel(this->model);

    // resize header
    QHeaderView * header = this->ui->tblVwJobs->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    QTimer::singleShot(5000, this, SLOT(updateModel()));
}

JobWidget::~JobWidget() {
    delete this->ui;
    delete this->model;
}


void JobWidget::credentialAsked(AbstractCredentials * cred) {
    HttpCredentials * httpCreds = qobject_cast<HttpCredentials *>(cred);

    if (httpCreds != nullptr and httpCreds->ready())
        this->doAuthentication();
}

void JobWidget::credentialFetched(AbstractCredentials * cred) {
    HttpCredentials * httpCreds = qobject_cast<HttpCredentials *>(cred);

    if (httpCreds != nullptr and httpCreds->ready())
        this->doAuthentication();
}

void JobWidget::doAuthentication() {
    OCC::AbstractCredentials * cred = this->account->credentials();
    HttpCredentials * httpCreds = qobject_cast<HttpCredentials *>(cred);

    if (httpCreds != nullptr) {
        if (not httpCreds->wasFetched()) {
            connect(this->account.data(), SIGNAL(credentialsFetched(AbstractCredentials *)), SLOT(doAuthentication()));
            httpCreds->fetchFromKeychain();
            return;
        } else {
            ArchivalAuthJob * job = new ArchivalAuthJob(this->account, httpCreds->password(), this);
            // connect(job, SIGNAL(connectionFailure()), SLOT(authenticationFailure()));
            connect(job, SIGNAL(connectionSuccess(int)), SLOT(fetchJobs(int)));
            job->start();
        }
    }
}

void JobWidget::fetchJobs(int user_id) {
    ArchivalJobsJob * job = new ArchivalJobsJob(this->account, user_id, 5, this);
    // connect(job, SIGNAL(fetchFailure()), SLOT(???));
    connect(job, SIGNAL(jobsFetch(const QList<int>&)), SLOT(jobs(const QList<int>&)));
    connect(job, SIGNAL(notConnected()), SLOT(doAuthentication()));
    job->start();
}

void JobWidget::fetchNextJob() {
    if (this->jobs_remain.size() > 0) {
        int job_id = this->jobs_remain.first();
        this->jobs_remain.removeFirst();

        ArchivalJobInfoJob * job = new ArchivalJobInfoJob(this->account, job_id, this);
        // connect(job, SIGNAL(fetchFailure()), SLOT(???));
        connect(job, SIGNAL(jobInfo(const Job&)), SLOT(job(const Job&)));
        connect(job, SIGNAL(notConnected()), SLOT(doAuthentication()));
        job->start();
    } else
        QTimer::singleShot(5000, this, SLOT(updateModel()));
}

void JobWidget::job(const Job& job) {
    this->model->setJob(job);
    this->fetchNextJob();
}

void JobWidget::jobs(const QList<int>& jobs) {
    this->jobs_remain = jobs;
    std::sort(this->jobs_remain.begin(), this->jobs_remain.end());
    this->model->setJobList(this->jobs_remain);
    this->fetchNextJob();
}

void JobWidget::updateModel() {
    OCC::AccountStatePtr ptr = AccountManager::instance()->accounts().first();
    this->account = ptr->account();

    ArchivalCheckConnectionJob * job = new ArchivalCheckConnectionJob(this->account, this);
    connect(job, SIGNAL(authenticationRequire()), SLOT(doAuthentication()));
    connect(job, SIGNAL(connectionAlive(int)), SLOT(fetchJobs(int)));
    job->start();
}
