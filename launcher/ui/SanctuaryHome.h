// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QUrl;

/** Sanctuary's dedicated entry point; account secrets and game launching stay in Prism. */
class SanctuaryHome : public QWidget {
    Q_OBJECT
public:
    explicit SanctuaryHome(QWidget* parent = nullptr);
    void setState(bool instanceReady, bool microsoftAvailable, bool french);

signals:
    void installRequested();
    void playRequested();
    void accountsRequested();
    void settingsRequested();
    void advancedRequested();
    void linkRequested(const QUrl& url);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void rebuild();
    QString text(const char* french, const char* english) const;
    bool m_instanceReady = false;
    bool m_microsoftAvailable = false;
    bool m_french = true;
};
