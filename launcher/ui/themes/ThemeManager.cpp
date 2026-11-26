// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2024 Tayou <git@tayou.org>
 *  Copyright (C) 2023 TheKodeToad <TheKodeToad@proton.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "ThemeManager.h"

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QIcon>
#include <QImageReader>
#include <QStyleHints>
#include "Exception.h"
#include "ui/themes/BrightTheme.h"
#include "ui/themes/CatPack.h"

#include "Application.h"
#include "settings/SettingsObject.h"

ThemeManager::ThemeManager()
{
    QIcon::setFallbackThemeName(QIcon::themeName());
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << m_iconThemeFolder.path());

    initializeIcons();
    initializeCatPacks();
}

ThemeManager::~ThemeManager()
{
    stopSettingNewWindowColorsOnMac();
}

QString ThemeManager::addIconTheme(IconTheme theme)
{
    QString id = theme.id();
    if (m_icons.find(id) == m_icons.end())
        m_icons.emplace(id, std::move(theme));
    else
        themeWarningLog() << "IconTheme(" << id << ") not added to prevent id duplication";
    return id;
}

void ThemeManager::initializeIcons()
{
    // TODO: icon themes and instance icons do not mesh well together. Rearrange and fix discrepancies!
    // set icon theme search path!
    themeDebugLog() << "<> Initializing Icon Themes";

    for (const QString& id : builtinIcons) {
        IconTheme theme(id, QString(":/icons/%1").arg(id));
        if (!theme.load()) {
            themeWarningLog() << "Couldn't load built-in icon theme" << id;
            continue;
        }

        addIconTheme(std::move(theme));
        themeDebugLog() << "Loaded Built-In Icon Theme" << id;
    }

    if (!m_iconThemeFolder.mkpath("."))
        themeWarningLog() << "Couldn't create icon theme folder";
    themeDebugLog() << "Icon Theme Folder Path:" << m_iconThemeFolder.absolutePath();

    QDirIterator directoryIterator(m_iconThemeFolder.path(), QDir::Dirs | QDir::NoDotAndDotDot);
    while (directoryIterator.hasNext()) {
        QDir dir(directoryIterator.next());
        IconTheme theme(dir.dirName(), dir.path());
        if (!theme.load())
            continue;

        addIconTheme(std::move(theme));
        themeDebugLog() << "Loaded Custom Icon Theme from" << dir.path();
    }

    themeDebugLog() << "<> Icon themes initialized.";
}

#ifndef Q_OS_MACOS
void ThemeManager::setTitlebarColorOnMac(WId windowId, QColor color) {}
void ThemeManager::setTitlebarColorOfAllWindowsOnMac(QColor color) {}
void ThemeManager::stopSettingNewWindowColorsOnMac() {}
#endif

QList<IconTheme*> ThemeManager::getValidIconThemes()
{
    QList<IconTheme*> ret;
    ret.reserve(m_icons.size());
    for (auto&& [id, theme] : m_icons) {
        ret.append(&theme);
    }
    return ret;
}

QList<CatPack*> ThemeManager::getValidCatPacks()
{
    QList<CatPack*> ret;
    ret.reserve(m_catPacks.size());
    for (auto&& [id, theme] : m_catPacks) {
        ret.append(theme.get());
    }
    return ret;
}

bool ThemeManager::isValidIconTheme(const QString& id)
{
    return !id.isEmpty() && m_icons.find(id) != m_icons.end();
}

QDir ThemeManager::getIconThemesFolder()
{
    return m_iconThemeFolder;
}

QDir ThemeManager::getCatPacksFolder()
{
    return m_catPacksFolder;
}

void ThemeManager::setIconTheme(const QString& name)
{
    if (m_icons.find(name) == m_icons.end()) {
        themeWarningLog() << "Tried to set invalid icon theme:" << name;
        return;
    }

    QIcon::setThemeName(name);
}

void ThemeManager::applyAppearance(bool initial)
{
    setIconTheme(APPLICATION->settings()->get("IconTheme").toString());
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif
    BrightTheme theme;
    theme.apply(initial);
    setTitlebarColorOfAllWindowsOnMac(qApp->palette().window().color());
    m_logColors = theme.logColorScheme();
    themeDebugLog() << "Applied light theme with window color" << qApp->palette().window().color().name();
}

QString ThemeManager::getCatPack(QString catName)
{
    auto catIter = m_catPacks.find(!catName.isEmpty() ? catName : APPLICATION->settings()->get("BackgroundCat").toString());
    if (catIter != m_catPacks.end()) {
        auto& catPack = catIter->second;
        themeDebugLog() << "applying catpack" << catPack->id();
        return catPack->path();
    } else {
        themeWarningLog() << "Tried to get invalid catPack:" << catName;
    }

    return m_catPacks.begin()->second->path();
}

QString ThemeManager::addCatPack(std::unique_ptr<CatPack> catPack)
{
    QString id = catPack->id();
    if (m_catPacks.find(id) == m_catPacks.end())
        m_catPacks.emplace(id, std::move(catPack));
    else
        themeWarningLog() << "CatPack(" << id << ") not added to prevent id duplication";
    return id;
}

void ThemeManager::initializeCatPacks()
{
    QList<std::pair<QString, QString>> defaultCats{ { "kitteh", QObject::tr("Background Cat (from MultiMC)") },
                                                    { "rory", QObject::tr("Rory ID 11 (drawn by Ashtaka)") },
                                                    { "rory-flat", QObject::tr("Rory ID 11 (flat edition, drawn by Ashtaka)") },
                                                    { "teawie", QObject::tr("Teawie (drawn by SympathyTea)") } };
    for (auto [id, name] : defaultCats) {
        addCatPack(std::unique_ptr<CatPack>(new BasicCatPack(id, name)));
    }
    if (!m_catPacksFolder.mkpath("."))
        themeWarningLog() << "Couldn't create catpacks folder";
    themeDebugLog() << "CatPacks Folder Path:" << m_catPacksFolder.absolutePath();

    QStringList supportedImageFormats;
    for (auto format : QImageReader::supportedImageFormats()) {
        supportedImageFormats.append("*." + format);
    }
    auto loadFiles = [this, supportedImageFormats](QDir dir) {
        // Load image files directly
        QDirIterator ImageFileIterator(dir.absoluteFilePath(""), supportedImageFormats, QDir::Files);
        while (ImageFileIterator.hasNext()) {
            QFile customCatFile(ImageFileIterator.next());
            QFileInfo customCatFileInfo(customCatFile);
            themeDebugLog() << "Loading CatPack from:" << customCatFileInfo.absoluteFilePath();
            addCatPack(std::unique_ptr<CatPack>(new FileCatPack(customCatFileInfo)));
        }
    };

    loadFiles(m_catPacksFolder);

    QDirIterator directoryIterator(m_catPacksFolder.path(), QDir::Dirs | QDir::NoDotAndDotDot);
    while (directoryIterator.hasNext()) {
        QDir dir(directoryIterator.next());
        QFileInfo manifest(dir.absoluteFilePath("catpack.json"));
        if (manifest.isFile()) {
            try {
                // Load background manifest
                themeDebugLog() << "Loading background manifest from:" << manifest.absoluteFilePath();
                addCatPack(std::unique_ptr<CatPack>(new JsonCatPack(manifest)));
            } catch (const Exception& e) {
                themeWarningLog() << "Couldn't load catpack json:" << e.cause();
            }
        } else {
            loadFiles(dir);
        }
    }
}

void ThemeManager::refresh()
{
    m_icons.clear();
    m_catPacks.clear();

    initializeIcons();
    initializeCatPacks();
}
