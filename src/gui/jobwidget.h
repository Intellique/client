#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include <QList>
#include <QWidget>

#include "account.h"

namespace Ui {
    class JobWidget;
}

class Job;
class JobModel;
class QItemSelection;

class JobWidget : public QWidget {
    Q_OBJECT

public:
    explicit JobWidget(QWidget * parent = nullptr);
    ~JobWidget();

private slots:
    void doAuthForUpdate();
    void doStopTask(int user_id);
    void fetchJobs(int user_id);
    void job(const Job& job);
    void jobKilled();
    void jobKillFailure();
    void jobs(const QList<int>& jobs);
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void stopTask();
    void updateModel();

    void getToken();
    void gotToken(const QString& token);
    void noToken();

private:
    void fetchNextJob();

    Ui::JobWidget * ui;
    JobModel * model;
    OCC::AccountPtr account;
    QList<int> jobs_remain;
    int selected_job = -1;
};

#endif // JOBWIDGET_H
