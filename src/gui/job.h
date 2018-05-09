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
            Scheduled,
            Waiting,
            Error,
            Pause,
            Stopped,
            Running,
            Disable,
            Finished,

            Unknown
        };

        QDateTime endTime() const;
        QDateTime eta() const;
        QString etaToString() const;
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
        inline const QString& type() const {
            return this->m_type;
        }

        Job& operator =(const QJsonObject& job);
        Job& operator =(const Job& job);

    private:
        bool parseJson(const QJsonObject& job);

        int m_id = -1;
        QString m_type;
        QString m_name;
        QDateTime m_start_time;
        QDateTime m_end_time;
        double m_done = -1;
        Status m_status = Unknown;
        bool m_is_null = true;
};

#endif // JOB_H
