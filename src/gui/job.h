#ifndef JOB_H
#define JOB_H

#include <QString>
#include <QDateTime>

class QJsonObject;

class Job {
    public:
        Job() = default;
        explicit Job(const QJsonObject& job);
        Job(const Job& job);
        ~Job() = default;

        enum Status {
            Disable,
            Error,
            Finished,
            Pause,
            Running,
            Scheduled,
            Stopped,
            Waiting,

            Unknown
        };

        QDateTime endTime() const;
        QString eta() const;
        inline double done() const {
            return this->m_done;
        }
        inline int id() const {
            return this->m_id;
        }
        inline bool is_null() const {
            return this->m_is_null;
        }
        inline const QString& name() const {
            return this->m_name;
        }
        inline const QDateTime& startTime() const {
            return this->m_start_time;
        }
        inline Status status() const {
            return this->m_status;
        }
        QString statusString() const;

        Job& operator =(const QJsonObject& job);
        Job& operator =(const Job& job);

    private:
        bool parseJson(const QJsonObject& job);

        int m_id = -1;
        QString m_name;
        QDateTime m_start_time;
        QDateTime m_end_time;
        double m_done = -1;
        Status m_status = Unknown;
        bool m_is_null = true;
};

#endif // JOB_H
