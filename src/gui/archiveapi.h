#ifndef ARCHIVEAPI_H
#define ARCHIVEAPI_H

#include <QJsonObject>

#include "abstractnetworkjob.h"

class ArchiveFileModel;
class Job;

namespace OCC {
    class ArchivalAuthJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalAuthJob(const AccountPtr& account, const QString& password, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void connectionFailure();
        void connectionSuccess(int user_id);

    private:
        QString m_password;
    };

    class ArchivalCheckConnectionJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalCheckConnectionJob(const AccountPtr& account, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void authenticationRequire();
        void connectionAlive(int user_id);
    };

    class ArchivalCreateArchiveJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalCreateArchiveJob(const QString& archive_name, const QString& user_home_directory, ArchiveFileModel * model, int pool_id, bool do_auth_on_failure, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void archiveCreated();
        void creationFailure();
        void notConnected();

    private:
        QString m_archive_name;
        ArchiveFileModel * m_model;
        int m_pool_id;
        QString m_user_home_directory;
        bool m_do_auth_on_failure;
    };

    class ArchivalJobInfoJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalJobInfoJob(const AccountPtr& account, int job_id, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void fetchFailure();
        void jobInfo(const Job& job);
        void notConnected();

    private:
        int m_job_id;
    };

    class ArchivalJobsJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalJobsJob(const AccountPtr& account, int user_id, int limit = 5, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void fetchFailure();
        void jobsFetch(const QList<int>& jobs);
        void notConnected();

    private:
        int m_user_id;
        int m_limit;
    };

    class ArchivalSearchPoolJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalSearchPoolJob(const AccountPtr& account, int pool_group, bool do_auth_on_failure, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void notConnected();
        void poolFound(int pool_id);
        void poolNotFound();
        void searchFailure();

    private:
        int m_pool_group;
        bool m_do_auth_on_failure;
};

    class ArchivalStopJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalStopJob(const AccountPtr& account, int job_id, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void failure();
        void jobKilled();
        void notConnected();

    private:
        int m_job_id;
    };

    class ArchivalUserInfoJob : public AbstractNetworkJob {
        Q_OBJECT

    public:
        explicit ArchivalUserInfoJob(const AccountPtr& account, int user_id, bool do_auth_on_failure, QObject * parent = nullptr);

    public slots:
        void start() Q_DECL_OVERRIDE;

    protected:
        bool finished() Q_DECL_OVERRIDE;

    signals:
        void notConnected();
        void userInfoComplete(QJsonObject user_info);
        void userInfoFailure();

    private:
        int m_user_id;
        bool m_do_auth_on_failure;
};
}

#endif // ARCHIVEAPI_H
