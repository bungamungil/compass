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

#include "securednstest.h"
#include "securedns.h"
#include "settings.h"

#include <QTest>
#include <QTemporaryDir>

void SecureDnsTest::initTestCase()
{
    // Initialize a temporary settings file so loadConfig/saveConfig can be tested.
    m_tempDir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_tempDir->isValid());
    Settings::createSettings(m_tempDir->filePath(QSL("settings.ini")));
}

void SecureDnsTest::cleanupTestCase()
{
    m_tempDir.reset();
}

void SecureDnsTest::validCustomUrl_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("valid");

    QTest::newRow("plain")                 << QSL("https://dns.example/dns-query") << true;
    QTest::newRow("template")              << QSL("https://dns.example/dns-query{?dns}") << true;
    QTest::newRow("ip")                    << QSL("https://1.1.1.1/dns-query") << true;
    QTest::newRow("uppercase-scheme")      << QSL("HTTPS://x.y/z") << true;
    QTest::newRow("surrounding-space")     << QSL("  https://dns.example/dns-query  ") << true;
    QTest::newRow("port-path-query")       << QSL("https://dns.example:8443/dns-query?ct=application/dns-message") << true;

    QTest::newRow("empty")                 << QString() << false;
    QTest::newRow("whitespace-only")       << QSL("   ") << false;
    QTest::newRow("http")                  << QSL("http://dns.example/dns-query") << false;
    QTest::newRow("ftp")                   << QSL("ftp://dns.example/dns-query") << false;
    QTest::newRow("no-scheme")             << QSL("dns.example/dns-query") << false;
    QTest::newRow("no-host")               << QSL("https://") << false;
    QTest::newRow("userinfo")              << QSL("https://user:pw@host/") << false;
    QTest::newRow("empty-host-with-path")  << QSL("https:///path") << false;
    QTest::newRow("not-a-url")             << QSL("not a url") << false;
    QTest::newRow("malformed-brace")       << QSL("https://host/{") << false;
    QTest::newRow("non-dns-template")      << QSL("https://host/dns-query{?foo}") << false;
}

void SecureDnsTest::validCustomUrl()
{
    QFETCH(QString, url);
    QFETCH(bool, valid);
    QCOMPARE(SecureDns::isValidCustomUrl(url), valid);
}

void SecureDnsTest::providerTemplates()
{
    QCOMPARE(SecureDns::providerTemplate(SecureDns::Cloudflare, QString()),
             QSL("https://cloudflare-dns.com/dns-query{?dns}"));
    QCOMPARE(SecureDns::providerTemplate(SecureDns::Google, QString()),
             QSL("https://dns.google/dns-query{?dns}"));
    QCOMPARE(SecureDns::providerTemplate(SecureDns::Custom, QSL("  https://dns.example/dns-query  ")),
             QSL("https://dns.example/dns-query"));
}

void SecureDnsTest::serverTemplatesForConfig()
{
    SecureDns::Config disabled;
    disabled.enabled = false;
    QVERIFY(SecureDns::serverTemplates(disabled).isEmpty());

    SecureDns::Config invalidCustom;
    invalidCustom.enabled = true;
    invalidCustom.provider = SecureDns::Custom;
    invalidCustom.customUrl = QSL("http://not-https");
    QVERIFY(SecureDns::serverTemplates(invalidCustom).isEmpty());

    SecureDns::Config cloudflare;
    cloudflare.enabled = true;
    cloudflare.provider = SecureDns::Cloudflare;
    QCOMPARE(SecureDns::serverTemplates(cloudflare).size(), 1);
}

void SecureDnsTest::customUrlTemplatePassthrough()
{
    // A custom URL with {?dns} suffix must survive into serverTemplates unchanged.
    SecureDns::Config cfg;
    cfg.enabled = true;
    cfg.provider = SecureDns::Custom;
    cfg.customUrl = QSL("https://dns.example/dns-query{?dns}");
    const QStringList tmpl = SecureDns::serverTemplates(cfg);
    QCOMPARE(tmpl.size(), 1);
    QCOMPARE(tmpl.first(), QSL("https://dns.example/dns-query{?dns}"));
}

void SecureDnsTest::cloudflareTemplateContent()
{
    SecureDns::Config cfg;
    cfg.enabled = true;
    cfg.provider = SecureDns::Cloudflare;
    const QStringList tmpl = SecureDns::serverTemplates(cfg);
    QCOMPARE(tmpl.size(), 1);
    QCOMPARE(tmpl.first(), QSL("https://cloudflare-dns.com/dns-query{?dns}"));
}

void SecureDnsTest::fallbackModeMapping()
{
    // serverTemplates returns the template regardless of fallbackToSystem;
    // the mode flag is consumed by apply(), not serverTemplates().
    SecureDns::Config withFallback;
    withFallback.enabled = true;
    withFallback.provider = SecureDns::Google;
    withFallback.fallbackToSystem = true;
    QCOMPARE(SecureDns::serverTemplates(withFallback).size(), 1);

    SecureDns::Config noFallback = withFallback;
    noFallback.fallbackToSystem = false;
    QCOMPARE(SecureDns::serverTemplates(noFallback).size(), 1);
}

void SecureDnsTest::loadConfigRoundTrip()
{
    // Write a config via saveConfig(), read it back via loadConfig().
    SecureDns::Config original;
    original.enabled = true;
    original.provider = SecureDns::Google;
    original.customUrl = QSL("https://dns.example/dns-query");
    original.fallbackToSystem = false;
    SecureDns::saveConfig(original);

    const SecureDns::Config loaded = SecureDns::loadConfig();
    QCOMPARE(loaded.enabled, original.enabled);
    QCOMPARE(static_cast<int>(loaded.provider), static_cast<int>(original.provider));
    QCOMPARE(loaded.customUrl, original.customUrl);
    QCOMPARE(loaded.fallbackToSystem, original.fallbackToSystem);
}

QTEST_GUILESS_MAIN(SecureDnsTest)
