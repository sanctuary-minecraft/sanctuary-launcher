// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QWidget>
#include <QPointer>
#include "ui/pages/BasePage.h"

class QLabel;
class QPushButton;
class QTextBrowser;
class QNetworkReply;

/** Read-only view of existing site routes. Private community feeds keep their Discord session requirement. */
class SanctuaryFeed : public QWidget {
    Q_OBJECT
public:
    explicit SanctuaryFeed(const QString& kind, QWidget* parent = nullptr);
    void setFrench(bool french);
    void reload();
    static QString publicNewsArticles(const QString& html);
signals:
    void linkRequested(const QUrl& url);
private:
    QString m_kind;
    bool m_french = true;
    QLabel* m_status;
    QPushButton* m_refresh;
    QTextBrowser* m_content;
    QPointer<QNetworkReply> m_reply;
};

class SanctuaryNewsPage : public QWidget, public BasePage {
    Q_OBJECT
public:
    explicit SanctuaryNewsPage(QWidget* parent = nullptr);
    QString id() const override { return "news"; }
    QString displayName() const override { return "Actualités"; }
    QIcon icon() const override { return QIcon(":/sanctuary/icon.png"); }
    void openedImpl() override;
    void setFrench(bool french);
signals:
    void linkRequested(const QUrl& url);
private:
    SanctuaryFeed* m_feed;
};
