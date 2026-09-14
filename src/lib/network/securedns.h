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

#ifndef SECUREDNS_H
#define SECUREDNS_H

#include <QString>
#include <QStringList>

#include "qzcommon.h"

namespace SecureDns
{
enum Provider {
    Cloudflare = 0,
    Google = 1,
    Custom = 2
};

struct Config {
    bool enabled = false;
    Provider provider = Cloudflare;
    QString customUrl;
    bool fallbackToSystem = true;
};

// True when built against Qt WebEngine >= 6.6 (setDnsMode available).
FALKON_EXPORT bool isSupported();

// Validates a user-supplied DoH endpoint (see rules in securedns.cpp).
FALKON_EXPORT bool isValidCustomUrl(const QString &url);

// Returns the RFC 6570 GET template for a provider (trimmed customUrl for Custom).
FALKON_EXPORT QString providerTemplate(Provider provider, const QString &customUrl);

// The template list for setDnsMode; empty when disabled or the custom URL is invalid.
FALKON_EXPORT QStringList serverTemplates(const Config &config);

// Reads the "Secure-DNS" Settings group.
FALKON_EXPORT Config loadConfig();

// Applies the config via QWebEngineGlobalSettings::setDnsMode. Returns false on failure.
FALKON_EXPORT bool apply(const Config &config);
}

#endif // SECUREDNS_H
