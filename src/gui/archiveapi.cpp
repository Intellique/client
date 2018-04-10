#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>

#include "account.h"
#include "archiveapi.h"
#include "archivefilemodel.h"
#include "creds/abstractcredentials.h"

using OCC::AbstractNetworkJob;
using OCC::ArchivalAuthJob;
using OCC::ArchivalCheckConnectionJob;
using OCC::ArchivalCreateArchiveJob;
using OCC::ArchivalSearchPoolJob;
using OCC::ArchivalUserInfoJob;

ArchivalAuthJob::ArchivalAuthJob(const AccountPtr& account, const QString& password, QObject * parent) : AbstractNetworkJob(account, "api/v1/auth/index.php", parent), m_password(password) {}


bool ArchivalAuthJob::finished() {
    QNetworkReply * serverReply = reply();
    QVariant code = serverReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    switch (code.toInt()) {
        case 201: {
            QJsonDocument doc = QJsonDocument::fromJson(serverReply->readAll());
            QJsonObject obj = doc.object();
            emit this->connectionSuccess(obj["user_id"].toInt());
            return true;
        }

        default:
            emit this->connectionFailure();
            return true;
    }
}

void ArchivalAuthJob::start() {
    QUrl url = account()->archivalUrl();

    OCC::AbstractCredentials * credential = account()->credentials();
    QJsonObject body_obj = {
        { "login", credential->user() },
        { "password", this->m_password },
        { "apikey", account()->archivalApiKey() }
    };
    QJsonDocument body;
    body.setObject(body_obj);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QBuffer * body_buffer = new QBuffer;
    body_buffer->setData(body.toJson());

    sendRequest("POST", Utility::concatUrlPath(url, path()), request, body_buffer);
    AbstractNetworkJob::start();
}



ArchivalCheckConnectionJob::ArchivalCheckConnectionJob(const AccountPtr& account, QObject * parent) : AbstractNetworkJob(account, "api/v1/auth/index.php", parent) {}


bool ArchivalCheckConnectionJob::finished() {
    QNetworkReply * serverReply = reply();
    QVariant code = serverReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    switch (code.toInt()) {
        case 200: {
            QJsonDocument doc = QJsonDocument::fromJson(serverReply->readAll());
            QJsonObject obj = doc.object();
            emit this->connectionAlive(obj["user_id"].toInt());
            return true;
        }

        default:
            emit this->authenticationRequire();
            return true;
    }
}

void ArchivalCheckConnectionJob::start() {
    QUrl url = account()->archivalUrl();

    sendRequest("GET", Utility::concatUrlPath(url, path()));
    AbstractNetworkJob::start();
}


ArchivalCreateArchiveJob::ArchivalCreateArchiveJob(const QString& archive_name, const QString& user_home_directory, ArchiveFileModel * model, int pool_id, bool do_auth_on_failure, QObject * parent) : AbstractNetworkJob(model->account(), "api/v1/archive/index.php", parent), m_archive_name(archive_name), m_model(model), m_pool_id(pool_id), m_user_home_directory(user_home_directory), m_do_auth_on_failure(do_auth_on_failure) {}


bool ArchivalCreateArchiveJob::finished() {
    QNetworkReply * serverReply = reply();
    QVariant code = serverReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    switch (code.toInt()) {
        case 201:
            emit this->archiveCreated();
            return true;

        default:
            emit this->creationFailure();
            return true;
    }
}

void ArchivalCreateArchiveJob::start() {
    QUrl url = account()->archivalUrl();

    QJsonObject body_obj = {
        { "name", this->m_archive_name },
        { "files", this->m_model->files(this->m_user_home_directory) },
        { "pool", this->m_pool_id }
    };
    QJsonDocument body;
    body.setObject(body_obj);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QBuffer * body_buffer = new QBuffer;
    body_buffer->setData(body.toJson());

    sendRequest("POST", Utility::concatUrlPath(url, path()), request, body_buffer);
    AbstractNetworkJob::start();
}



ArchivalSearchPoolJob::ArchivalSearchPoolJob(const AccountPtr& account, int pool_group, bool do_auth_on_failure, QObject * parent) : AbstractNetworkJob(account, "api/v1/pool/search/index.php", parent), m_pool_group(pool_group), m_do_auth_on_failure(do_auth_on_failure) {}


bool ArchivalSearchPoolJob::finished() {
    QNetworkReply * serverReply = reply();
    QVariant code = serverReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    switch (code.toInt()) {
        case 200: {
            QJsonDocument doc = QJsonDocument::fromJson(serverReply->readAll());
            QJsonObject obj = doc.object();
            QJsonArray pools = obj["pools"].toArray();
            emit this->poolFound(pools[0].toInt());
            return true;
        }

        case 401:
            if (this->m_do_auth_on_failure)
                emit this->notConnected();
            else
                emit this->searchFailure();
            return true;

        case 404:
            emit this->poolNotFound();
            return true;

        default:
            emit this->searchFailure();
            return true;
    }
}

void ArchivalSearchPoolJob::start() {
    QUrl url = Utility::concatUrlPath(account()->archivalUrl(), path());

    QList<QPair<QString, QString>> params;
    params << qMakePair(QString::fromLatin1("poolgroup"), QString::number(this->m_pool_group));
    url.setQueryItems(params);

    sendRequest("GET", url);
    AbstractNetworkJob::start();
}


ArchivalUserInfoJob::ArchivalUserInfoJob(const AccountPtr& account, int user_id, bool do_auth_on_failure, QObject * parent) : AbstractNetworkJob(account, "api/v1/user/index.php", parent), m_user_id(user_id), m_do_auth_on_failure(do_auth_on_failure) {}


bool ArchivalUserInfoJob::finished() {
    QNetworkReply * serverReply = reply();
    QVariant code = serverReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    switch (code.toInt()) {
        case 200: {
            QJsonDocument doc = QJsonDocument::fromJson(serverReply->readAll());
            QJsonObject obj = doc.object();
            emit this->userInfoComplete(obj["user"].toObject());
            return true;
        }

        case 401:
            if (this->m_do_auth_on_failure)
                emit this->notConnected();
            else
                emit this->userInfoFailure();
            return true;

        default:
            emit this->userInfoFailure();
            return true;
    }
}

void ArchivalUserInfoJob::start() {
    QUrl url = Utility::concatUrlPath(account()->archivalUrl(), path());

    QList<QPair<QString, QString>> params;
    params << qMakePair(QString::fromLatin1("id"), QString::number(this->m_user_id));
    url.setQueryItems(params);

    sendRequest("GET", url);
    AbstractNetworkJob::start();
}
