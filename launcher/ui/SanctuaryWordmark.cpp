// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryWordmark.h"
#include <QPainter>

static void initializeWordmarkResources() { Q_INIT_RESOURCE(sanctuary); }

SanctuaryWordmark::SanctuaryWordmark(QWidget* parent) : QLabel(parent)
{
    initializeWordmarkResources();
    const QPixmap source(":/sanctuary/wordmark.png");
    setPixmap(source);
    setFixedSize(180, source.isNull() ? 30 : qRound(180.0 * source.height() / source.width()));
    setAccessibleName("Sanctuary");
}

void SanctuaryWordmark::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(contentsRect(), pixmap());
}
