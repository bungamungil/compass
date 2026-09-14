/* ============================================================
* Compass
* Copyright (C) 2026 Bunga Mungil <bungamungil@icloud.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */

#include "securedns.h"
#include "settings.h"

#include <QUrl>
#include <QDebug>

#include <QtWebEngineCore/qtwebenginecoreversion.h>
#if QTWEBENGINECORE_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QWebEngineGlobalSettings>
#endif

bool SecureDns::isSupported()
{
#if QTWEBENGINECORE_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    return true;
#else
    return false;
#endif
}

bool SecureDns::isValidCustomUrl(const QString &url)
{
    QString trimmed = url.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    // Strip a trailing RFC 6570 "{?dns}" GET-template expression before parsing.
    if (trimmed.endsWith(QL1S("{?dns}"))) {
        trimmed.chop(6);
    }

    // Any remaining template braces are not a valid endpoint URL.
    if (trimmed.contains(QL1C('{')) || trimmed.contains(QL1C('}'))) {
        return false;
    }

    const QUrl parsed(trimmed, QUrl::StrictMode);
    if (!parsed.isValid()) {
        return false;
    }
    if (parsed.scheme().compare(QSL("https"), Qt::CaseInsensitive) != 0) {
        return false;
    }
    if (parsed.host().isEmpty()) {
        return false;
    }
    if (!parsed.userInfo().isEmpty()) {
        return false;
    }
    return true;
}

QString SecureDns::providerTemplate(Provider provider, const QString &customUrl)
{
    switch (provider) {
    case Cloudflare:
        return QSL("https://cloudflare-dns.com/dns-query{?dns}");
    case Google:
        return QSL("https://dns.google/dns-query{?dns}");
    case Custom:
        return customUrl.trimmed();
    }
    return QString();
}

QStringList SecureDns::serverTemplates(const Config &config)
{
    if (!config.enabled) {
        return {};
    }
    if (config.provider == Custom && !isValidCustomUrl(config.customUrl)) {
        return {};
    }
    const QString tmpl = providerTemplate(config.provider, config.customUrl);
    if (tmpl.isEmpty()) {
        return {};
    }
    return { tmpl };
}

SecureDns::Config SecureDns::loadConfig()
{
    Config config;
    Settings settings;
    settings.beginGroup(QSL("Secure-DNS"));
    config.enabled = settings.value(QSL("Enabled"), false).toBool();
    config.provider = static_cast<Provider>(settings.value(QSL("Provider"), 0).toInt());
    config.customUrl = settings.value(QSL("CustomUrl"), QString()).toString();
    config.fallbackToSystem = settings.value(QSL("FallbackToSystem"), true).toBool();
    settings.endGroup();
    return config;
}

bool SecureDns::apply(const Config &config)
{
#if QTWEBENGINECORE_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    using namespace QWebEngineGlobalSettings;

    if (!config.enabled || !isSupported()) {
        setDnsMode(DnsMode{SecureDnsMode::SystemOnly, {}});
        return true;
    }

    DnsMode mode;
    mode.secureMode = config.fallbackToSystem ? SecureDnsMode::SecureWithFallback
                                              : SecureDnsMode::SecureOnly;
    mode.serverTemplates = serverTemplates(config);

    const bool ok = setDnsMode(mode);
    if (!ok) {
        qWarning() << "SecureDns: setDnsMode failed; reverting to system DNS";
        // Chromium keeps the previous mode on failure — force a clean system resolver.
        setDnsMode(DnsMode{SecureDnsMode::SystemOnly, {}});
    }
    return ok;
#else
    if (config.enabled) {
        qWarning() << "SecureDns: Qt WebEngine < 6.6 has no DNS-over-HTTPS support";
        return false;
    }
    return true;
#endif
}
