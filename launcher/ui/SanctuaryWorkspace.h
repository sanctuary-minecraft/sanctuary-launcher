// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QWidget>
#include <QList>
#include <QHash>
#include <QSet>
#include "ui/pages/BasePageContainer.h"

class BasePage;
class QLabel;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;

/** The native Prism pages, hosted in the Sanctuary window rather than InstanceWindow dialogs. */
class SanctuaryWorkspace : public QWidget, public BasePageContainer {
    Q_OBJECT
public:
    explicit SanctuaryWorkspace(QWidget* parent = nullptr);
    ~SanctuaryWorkspace() override;
    void setPages(const QList<BasePage*>& pages, bool global = false);
    void appendPages(const QList<BasePage*>& pages);
    void setGameBusy(bool busy);
    void setFrench(bool french);
    bool saveAll();
    bool selectPage(QString pageId) override;
    BasePage* selectedPage() const override { return m_current; }
    BasePage* getPage(QString pageId) override;
    void refreshContainer() override;
    bool requestClose() override;
signals:
    void homeRequested();
    void settingsApplied();
private:
    QString name(BasePage* page) const;
    QList<BasePage*> m_pages;
    QSet<BasePage*> m_visited;
    QHash<QString, QPushButton*> m_navigation;
    BasePage* m_current = nullptr;
    QStackedWidget* m_stack;
    QVBoxLayout* m_navLayout;
    QLabel *m_title, *m_feedback;
    QWidget* m_toolbar;
    QPushButton *m_home, *m_save;
    bool m_french = true;
    bool m_global = false;
};
