#ifndef JOBLOGWIDGET_H
#define JOBLOGWIDGET_H

#include <QList>
#include <QWidget>

#include "account.h"

namespace Ui {
    class JobLogWidget;
}

class Job;
class JobLogModel;

class JobLogWidget : public QWidget {
        Q_OBJECT

    public:
        explicit JobLogWidget(QWidget *parent = nullptr);
        ~JobLogWidget();

    private slots:
        void doAuthForUpdate();
        void fetchJobs(int user_id);
        void job(const Job& job);
        void jobs(const QList<int>& jobs);
        void updateModel();

    private:
        void fetchNextJob();

        Ui::JobLogWidget * ui;
        JobLogModel * model;
        OCC::AccountPtr account;
        QList<int> jobs_remain;
};

#endif // JOBLOGWIDGET_H
