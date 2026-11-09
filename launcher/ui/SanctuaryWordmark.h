// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QLabel>

/** Draw the original PNG directly at the screen's resolution. */
class SanctuaryWordmark : public QLabel {
public:
    explicit SanctuaryWordmark(QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* event) override;
};
