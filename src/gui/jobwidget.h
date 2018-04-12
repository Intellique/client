#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include <QList>
#include <QWidget>

#include "account.h"

namespace QKeychain {
    class Job;
}

namespace OCC {
    class AbstractCredentials;
}

namespace Ui {
    class JobWidget;
}

class Job;
class JobModel;

class JobWidget : public QWidget {
    Q_OBJECT

public:
    explicit JobWidget(QWidget * parent = nullptr);
    ~JobWidget();

private slots:
    void credentialAsked(OCC::AbstractCredentials * credentials);
    void credentialFetched(OCC::AbstractCredentials * credentials);
    void doAuthentication();
    void fetchJobs(int user_id);
    void job(const Job& job);
    void jobs(const QList<int>& jobs);
    void updateModel();

private:
    void fetchNextJob();

    Ui::JobWidget * ui;
    JobModel * model;
    OCC::AccountPtr account;
    QList<int> jobs_remain;
};

#endif // JOBWIDGET_H
