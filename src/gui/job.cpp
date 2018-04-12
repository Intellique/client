#include <QJsonArray>
#include <QJsonObject>
#include <QHash>

#include "job.h"
#include "common/utility.h"

Job::Job(const QJsonObject& job) {
    this->m_is_null = not this->parseJson(job);
}

Job::Job(const Job& job) : m_id(job.m_id), m_name(job.m_name), m_start_time(job.m_start_time), m_end_time(job.m_end_time), m_done(job.m_done), m_status(job.m_status), m_is_null(job.m_is_null) {}


QDateTime Job::endTime() const {
    return this->m_end_time;
}

QString Job::eta() const {
    if (this->m_end_time.isValid())
        return "";

    QDateTime now = QDateTime::currentDateTime();
    qint64 msec = this->m_start_time.msecsTo(now);
    QDateTime eta = this->m_start_time.addMSecs(msec / this->m_done);

    return OCC::Utility::durationToDescriptiveString1(now.msecsTo(eta));
}

bool Job::parseJson(const QJsonObject& job) {
    QJsonValue elt = job["id"];
    if (elt.isDouble())
        this->m_id = elt.toInt();
    else
        this->m_is_null = true;

    elt = job["name"];
    if (elt.isString())
        this->m_name = elt.toString();
    else
        this->m_is_null = true;

    elt = job["runs"];
    if (not elt.isArray()) {
        this->m_is_null = true;
        return false;
    }

    QJsonArray runs = elt.toArray();
    if (runs.size() > 0) {
        QJsonValue run = runs.last();
        if (not run.isObject())
            return false;

        QJsonObject job_run = run.toObject();
        elt = job_run["starttime"];
        if (elt.isString())
            this->m_start_time = QDateTime::fromString(elt.toString());
        else if (elt.isObject()) {
            QJsonObject date = elt.toObject();
            const QString str_date = date["date"].toString() + date["timezone"].toString();
            this->m_start_time = QDateTime::fromString(str_date, Qt::ISODate);
        }

        elt = job_run["endtime"];
        if (elt.isString())
            this->m_end_time = QDateTime::fromString(elt.toString());
        else if (elt.isObject()) {
            QJsonObject date = elt.toObject();
            const QString str_date = date["date"].toString() + date["timezone"].toString();
            this->m_end_time = QDateTime::fromString(str_date, Qt::ISODate);
        }

        elt = job_run["done"];
        if (elt.isDouble())
            this->m_done = elt.toDouble();

        static const QHash<QString, Job::Status> string2status = {
            { "disable",   Job::Disable },
            { "error",     Job::Error },
            { "finished",  Job::Finished },
            { "running",   Job::Running },
            { "scheduled", Job::Scheduled },
            { "stopped",   Job::Stopped },
            { "unknown",   Job::Unknown },
            { "waiting",   Job::Waiting}
        };

        QString status = job_run["status"].toString();
        if (string2status.contains(status))
            this->m_status = string2status[status];
        else
            this->m_status = Job::Unknown;
    }

    return true;
}

QString Job::statusString() const {
    switch (this->m_status) {
        case Job::Disable:
            return "Disable";

        case Job::Error:
            return "Error";

        case Job::Finished:
            return "Finished";

        case Job::Pause:
            return "Pause";

        case Job::Running:
            return "Running";

        case Job::Scheduled:
            return "Scheduled";

        case Job::Stopped:
            return "Stopped";

        default:
            return "Unkown";
    }
}


Job& Job::operator=(const QJsonObject& job) {
    this->parseJson(job);
    return *this;
}

Job& Job::operator=(const Job& job) {
    if (this == &job)
        return *this;

    this->m_id = job.m_id;
    this->m_name = job.m_name;
    this->m_start_time = job.m_start_time;
    this->m_end_time = job.m_end_time;
    this->m_done = job.m_done;
    this->m_status = job.m_status;
    this->m_is_null = job.m_is_null;

    return *this;
}
