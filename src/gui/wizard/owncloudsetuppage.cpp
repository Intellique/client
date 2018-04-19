/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 * Copyright (C) by Krzesimir Nowak <krzesimir@endocode.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include <QDir>
#include <QFileDialog>
#include <QUrl>
#include <QTimer>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QSsl>
#include <QSslCertificate>
#include <QNetworkAccessManager>
#include <QUuid>

#include "QProgressIndicator.h"

#include "wizard/owncloudwizardcommon.h"
#include "wizard/owncloudsetuppage.h"
#include "wizard/owncloudconnectionmethoddialog.h"
#include "theme.h"
#include "account.h"

namespace OCC {

OwncloudSetupPage::OwncloudSetupPage(QWidget *parent)
    : QWizardPage()
    , _ui()
    , _oCUrl()
    , _ocUser()
    , _authTypeKnown(false)
    , _checking(false)
    , _authType(DetermineAuthTypeJob::Basic)
    , _progressIndi(new QProgressIndicator(this))
{
    _ui.setupUi(this);
    _ocWizard = qobject_cast<OwncloudWizard *>(parent);

    Theme *theme = Theme::instance();
    setTitle(WizardCommon::titleTemplate().arg(tr("Connect to %1").arg(theme->appNameGUI())));
    setSubTitle(WizardCommon::subTitleTemplate().arg(tr("Setup %1 server").arg(theme->appNameGUI())));

    if (theme->overrideServerUrl().isEmpty()) {
        _ui.leUrlStorage->setPostfix(theme->wizardUrlPostfix());
        _ui.leUrlStorage->setPlaceholderText(theme->wizardUrlHint());

        _ui.leUrlArchival->setPostfix(theme->wizardUrlPostfix());
        _ui.leUrlArchival->setPlaceholderText(theme->wizardUrlHint());
    } else {
        _ui.leUrlStorage->setEnabled(false);
        _ui.leUrlArchival->setEnabled(false);
        _ui.lnEdtAPI->setEnabled(false);
    }

    if (this->loadSettings()) {
        connect(_ui.rdBttnCloudIntellique, &QRadioButton::clicked, this, &OwncloudSetupPage::slotDisableForm);
        connect(_ui.rdBttnCustom, &QRadioButton::clicked, this, &OwncloudSetupPage::slotEnableForm);
        slotDisableForm();

        registerField(QLatin1String("OCUrl*"), _ui.leUrlStorage);
        registerField(QLatin1String("IntelliqueUrl*"), _ui.leUrlArchival);
        registerField(QLatin1String("IntelliqueAPI*"), _ui.lnEdtAPI);
    } else {
        _ui.rdBttnCloudIntellique->setChecked(false);
        _ui.rdBttnCloudIntellique->setEnabled(false);
        _ui.rdBttnCustom->setChecked(true);
        slotEnableForm();
    }

    _ui.resultLayout->addWidget(_progressIndi);
    stopSpinner();

    setupCustomization();

    slotUrlArchivalChanged(QLatin1String("")); // don't jitter UI
    slotUrlStorageChanged(QLatin1String("")); // don't jitter UI
    connect(_ui.leUrlArchival, &QLineEdit::textChanged, this, &OwncloudSetupPage::slotUrlArchivalChanged);
    connect(_ui.leUrlStorage, &QLineEdit::textChanged, this, &OwncloudSetupPage::slotUrlStorageChanged);
    connect(_ui.lnEdtAPI, &QLineEdit::textChanged, this, &OwncloudSetupPage::completeChanged);
    connect(_ui.leUrlArchival, &QLineEdit::editingFinished, this, &OwncloudSetupPage::slotArchivalUrlEditFinished);
    connect(_ui.leUrlStorage, &QLineEdit::editingFinished, this, &OwncloudSetupPage::slotStorageUrlEditFinished);

    addCertDial = new AddCertificateDialog(this);
}

void OwncloudSetupPage::setServerUrl(const QString &newUrl)
{
    _oCUrl = newUrl;
    if (_oCUrl.isEmpty()) {
        _ui.leUrlStorage->clear();
        return;
    }

    _ui.leUrlStorage->setText(_oCUrl);
}

void OwncloudSetupPage::setupCustomization()
{
    // set defaults for the customize labels.
    _ui.topLabel->hide();
    _ui.bottomLabel->hide();

    Theme *theme = Theme::instance();
    QVariant variant = theme->customMedia(Theme::oCSetupTop);
    if (!variant.isNull()) {
        WizardCommon::setupCustomMedia(variant, _ui.topLabel);
    }

    variant = theme->customMedia(Theme::oCSetupBottom);
    WizardCommon::setupCustomMedia(variant, _ui.bottomLabel);
}

// slot hit from textChanged of the url entry field.
void OwncloudSetupPage::slotUrlArchivalChanged(const QString &url)
{
    if (!url.startsWith(QLatin1String("https://"))) {
        _ui.urlLblArchival->setPixmap(QPixmap(Theme::hidpiFileName(":/client/resources/lock-http.png")));
        _ui.urlLblArchival->setToolTip(tr("This url is NOT secure as it is not encrypted.\n"
                                    "It is not advisable to use it."));
    } else {
        _ui.urlLblArchival->setPixmap(QPixmap(Theme::hidpiFileName(":/client/resources/lock-https.png")));
        _ui.urlLblArchival->setToolTip(tr("This url is secure. You can use it."));
    }

    emit completeChanged();
}

// slot hit from textChanged of the url entry field.
void OwncloudSetupPage::slotUrlStorageChanged(const QString &url)
{
    _authTypeKnown = false;

    QString newUrl = url;
    if (url.endsWith("index.php")) {
        newUrl.chop(9);
    }
    if (_ocWizard && _ocWizard->account()) {
        QString webDavPath = _ocWizard->account()->davPath();
        if (url.endsWith(webDavPath)) {
            newUrl.chop(webDavPath.length());
        }
        if (webDavPath.endsWith(QLatin1Char('/'))) {
            webDavPath.chop(1); // cut off the slash
            if (url.endsWith(webDavPath)) {
                newUrl.chop(webDavPath.length());
            }
        }
    }
    if (newUrl != url) {
        _ui.leUrlStorage->setText(newUrl);
    }

    if (!url.startsWith(QLatin1String("https://"))) {
        _ui.urlLblStorage->setPixmap(QPixmap(Theme::hidpiFileName(":/client/resources/lock-http.png")));
        _ui.urlLblStorage->setToolTip(tr("This url is NOT secure as it is not encrypted.\n"
                                    "It is not advisable to use it."));
    } else {
        _ui.urlLblStorage->setPixmap(QPixmap(Theme::hidpiFileName(":/client/resources/lock-https.png")));
        _ui.urlLblStorage->setToolTip(tr("This url is secure. You can use it."));
    }

    emit completeChanged();
}

void OwncloudSetupPage::slotArchivalUrlEditFinished()
{
    QString url = _ui.leUrlArchival->fullText();
    if (QUrl(url).isRelative() && !url.isEmpty()) {
        // no scheme defined, set one
        url.prepend("https://");
    }
    _ui.leUrlArchival->setFullText(url);
}

void OwncloudSetupPage::slotStorageUrlEditFinished()
{
    QString url = _ui.leUrlStorage->fullText();
    if (QUrl(url).isRelative() && !url.isEmpty()) {
        // no scheme defined, set one
        url.prepend("https://");
    }
    _ui.leUrlStorage->setFullText(url);
}

bool OwncloudSetupPage::isComplete() const {
    if (_ui.rdBttnCloudIntellique->isChecked())
        return true;

    QUuid uuid(_ui.lnEdtAPI->text());
    if (uuid.isNull())
        return false;

    return !_ui.leUrlStorage->text().isEmpty() and not _ui.leUrlArchival->text().isEmpty() and !_checking;
}

void OwncloudSetupPage::initializePage()
{
    WizardCommon::initErrorLabel(_ui.errorLabel);

    _authTypeKnown = false;
    _checking = false;

    QAbstractButton *nextButton = wizard()->button(QWizard::NextButton);
    QPushButton *pushButton = qobject_cast<QPushButton *>(nextButton);
    if (pushButton)
        pushButton->setDefault(true);

    // If url is overriden by theme, it's already set and
    // we just check the server type and switch to second page
    // immediately.
    if (Theme::instance()->overrideServerUrl().isEmpty()) {
        _ui.leUrlStorage->setFocus();
    } else {
        setCommitPage(true);
        // Hack: setCommitPage() changes caption, but after an error this page could still be visible
        setButtonText(QWizard::CommitButton, tr("&Next >"));
        validatePage();
        setVisible(false);
    }
}

bool OwncloudSetupPage::urlHasChanged()
{
    bool change = false;
    const QChar slash('/');

    QUrl currentUrl(storageUrl());
    QUrl initialUrl(_oCUrl);

    QString currentPath = currentUrl.path();
    QString initialPath = initialUrl.path();

    // add a trailing slash.
    if (!currentPath.endsWith(slash))
        currentPath += slash;
    if (!initialPath.endsWith(slash))
        initialPath += slash;

    if (currentUrl.host() != initialUrl.host() || currentUrl.port() != initialUrl.port() || currentPath != initialPath) {
        change = true;
    }

    return change;
}

int OwncloudSetupPage::nextId() const
{
    switch (_authType) {
    case DetermineAuthTypeJob::Basic:
        return WizardCommon::Page_HttpCreds;
    case DetermineAuthTypeJob::OAuth:
        return WizardCommon::Page_OAuthCreds;
    case DetermineAuthTypeJob::Shibboleth:
        return WizardCommon::Page_ShibbolethCreds;
    }
    return WizardCommon::Page_HttpCreds;
}

QString OwncloudSetupPage::archivalApiKey() const {
    if (_ui.rdBttnCloudIntellique->isChecked())
        return _settings.value("Archival/api_key").toString();
    else
        return _ui.lnEdtAPI->text();
}

QString OwncloudSetupPage::archivalUrl() const {
    if (_ui.rdBttnCloudIntellique->isChecked())
        return _settings.value("Archival/url").toString();
    else
        return _ui.leUrlArchival->fullText().simplified();
}

QString OwncloudSetupPage::storageUrl() const {
    if (_ui.rdBttnCloudIntellique->isChecked()) {
        QString val = _settings.value("Storage/url").toString();
        return val;
    } else
        return _ui.leUrlStorage->fullText().simplified();
}

bool OwncloudSetupPage::validatePage()
{
    if (!_authTypeKnown) {
        setErrorString(QString::null, false);
        _checking = true;
        startSpinner();
        emit completeChanged();

        emit determineAuthType(storageUrl());
        return false;
    } else {
        // connecting is running
        stopSpinner();
        _checking = false;
        emit completeChanged();
        return true;
    }
}

void OwncloudSetupPage::setAuthType(DetermineAuthTypeJob::AuthType type)
{
    _authTypeKnown = true;
    _authType = type;
    // stopSpinner();
}

void OwncloudSetupPage::setErrorString(const QString &err, bool retryHTTPonly)
{
    if (err.isEmpty()) {
        _ui.errorLabel->setVisible(false);
    } else {
        if (retryHTTPonly) {
            QUrl url(_ui.leUrlStorage->fullText());
            if (url.scheme() == "https") {
                // Ask the user how to proceed when connecting to a https:// URL fails.
                // It is possible that the server is secured with client-side TLS certificates,
                // but that it has no way of informing the owncloud client that this is the case.

                OwncloudConnectionMethodDialog dialog;
                dialog.setUrl(url);
                // FIXME: Synchronous dialogs are not so nice because of event loop recursion
                int retVal = dialog.exec();

                switch (retVal) {
                case OwncloudConnectionMethodDialog::No_TLS: {
                    url.setScheme("http");
                    _ui.leUrlStorage->setFullText(url.toString());
                    // skip ahead to next page, since the user would expect us to retry automatically
                    wizard()->next();
                } break;
                case OwncloudConnectionMethodDialog::Client_Side_TLS:
                    addCertDial->show();
                    connect(addCertDial, &QDialog::accepted, this, &OwncloudSetupPage::slotCertificateAccepted);
                    break;
                case OwncloudConnectionMethodDialog::Closed:
                case OwncloudConnectionMethodDialog::Back:
                default:
                    // No-op.
                    break;
                }
            }
        }

        _ui.errorLabel->setVisible(true);
        _ui.errorLabel->setText(err);
    }
    _checking = false;
    emit completeChanged();
    stopSpinner();
}

void OwncloudSetupPage::startSpinner()
{
    _ui.resultLayout->setEnabled(true);
    _progressIndi->setVisible(true);
    _progressIndi->startAnimation();
}

void OwncloudSetupPage::stopSpinner()
{
    _ui.resultLayout->setEnabled(false);
    _progressIndi->setVisible(false);
    _progressIndi->stopAnimation();
}

QString subjectInfoHelper(const QSslCertificate &cert, const QByteArray &qa)
{
    return cert.subjectInfo(qa).join(QLatin1Char('/'));
}

//called during the validation of the client certificate.
void OwncloudSetupPage::slotCertificateAccepted()
{
    QList<QSslCertificate> clientCaCertificates;
    QFile certFile(addCertDial->getCertificatePath());
    certFile.open(QFile::ReadOnly);
    if (QSslCertificate::importPkcs12(&certFile,
            &_ocWizard->_clientSslKey, &_ocWizard->_clientSslCertificate,
            &clientCaCertificates,
            addCertDial->getCertificatePasswd().toLocal8Bit())) {
        AccountPtr acc = _ocWizard->account();

        // to re-create the session ticket because we added a key/cert
        acc->setSslConfiguration(QSslConfiguration());
        QSslConfiguration sslConfiguration = acc->getOrCreateSslConfig();

        // We're stuffing the certificate into the configuration form here. Later the
        // cert will come via the HttpCredentials
        sslConfiguration.setLocalCertificate(_ocWizard->_clientSslCertificate);
        sslConfiguration.setPrivateKey(_ocWizard->_clientSslKey);
        acc->setSslConfiguration(sslConfiguration);

        // Make sure TCP connections get re-established
        acc->networkAccessManager()->clearAccessCache();

        addCertDial->reinit(); // FIXME: Why not just have this only created on use?
        validatePage();
    } else {
        addCertDial->showErrorMessage("Could not load certificate");
        addCertDial->show();
    }
}

void OwncloudSetupPage::slotDisableForm() {
    _ui.leUrlStorage->setEnabled(false);
    _ui.leUrlArchival->setEnabled(false);
    _ui.lnEdtAPI->setEnabled(false);
}

void OwncloudSetupPage::slotEnableForm() {
    _ui.leUrlStorage->setEnabled(true);
    _ui.leUrlArchival->setEnabled(true);
    _ui.lnEdtAPI->setEnabled(true);
}

OwncloudSetupPage::~OwncloudSetupPage()
{
}

void OwncloudSetupPage::startCheckArchivalServer() {
    emit this->checkArchivalServer(archivalUrl(), archivalApiKey());
}

bool OwncloudSetupPage::loadSettings() {
#ifdef Q_OS_MACOS
    QString path = QCoreApplication::applicationDirPath() + "/../Resources/Intellique.ini";
    QSettings mac_settings(path,QSettings::IniFormat);
    if (mac_settings.status() == QSettings::NoError and mac_settings.allKeys().length() > 0) {
        this->loadSettings(mac_settings);
        return true;
    }
#else
    QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "Intellique", "IntelliqueClient");
    if (settings.status() == QSettings::NoError and settings.allKeys().length() > 0) {
        this->loadSettings(settings);
        return true;
    }
#endif

    return false;
}

void OwncloudSetupPage::loadSettings(QSettings& settings) {
    foreach (const QString& key, settings.allKeys())
        this->_settings[key] = settings.value(key);
}

} // namespace OCC
