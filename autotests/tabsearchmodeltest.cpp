/* ============================================================
* Falkon - Qt web browser
* Copyright (C) 2026  Bunga Mungil <bungamungil@icloud.com>
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
#include "tabsearchmodeltest.h"
#include "tabsearchmodel.h"

#include <QTest>

void TabSearchModelTest::matching()
{
    QVERIFY(tabSearchMatches(QSL("KDE Konsole"), QSL("https://invent.kde.org/konsole"), QString()));
    QVERIFY(tabSearchMatches(QSL("KDE Konsole"), QSL("https://invent.kde.org/konsole"), QSL("konsole")));
    QVERIFY(tabSearchMatches(QSL("KDE Konsole"), QSL("https://invent.kde.org/konsole"), QSL("kde invent")));
    QVERIFY(!tabSearchMatches(QSL("KDE Konsole"), QSL("https://invent.kde.org/konsole"), QSL("firefox")));

    // Regex metacharacters in the filter must be escaped (literal '.', not "any character")
    QVERIFY(tabSearchMatches(QSL("Test"), QSL("https://kde.org/page"), QSL("kde.org")));
    QVERIFY(!tabSearchMatches(QSL("Test"), QSL("https://kdeXorg/page"), QSL("kde.org")));

    // Case-insensitive matching
    QVERIFY(tabSearchMatches(QSL("KDE Konsole"), QSL("https://invent.kde.org/konsole"), QSL("KONSOLE")));

    // Match via URL alone, when the title doesn't contain the filter term at all
    QVERIFY(tabSearchMatches(QSL("Home Page"), QSL("https://phabricator.kde.org"), QSL("phabricator")));

    // Whitespace-only filter matches everything (trims to empty)
    QVERIFY(tabSearchMatches(QSL("Anything"), QSL("https://example.com"), QSL("   ")));
}

QTEST_GUILESS_MAIN(TabSearchModelTest)
