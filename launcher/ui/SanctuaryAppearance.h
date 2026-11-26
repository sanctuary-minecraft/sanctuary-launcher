// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QApplication>
#include <QEvent>
#include <QPalette>
#include <QWidget>
#include <functional>

namespace SanctuaryAppearance {
inline bool isDark()
{
    const auto palette = QApplication::palette();
    return palette.color(QPalette::Window).lightness() < palette.color(QPalette::WindowText).lightness();
}

class PaletteWatcher final : public QObject {
public:
    PaletteWatcher(QWidget* widget, std::function<void()> update) : QObject(widget), m_update(std::move(update))
    {
        m_update();
        qApp->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange)
            QMetaObject::invokeMethod(this, m_update, Qt::QueuedConnection);
        return false;
    }

private:
    std::function<void()> m_update;
};

inline QString colors(QString style)
{
    struct Color {
        const char* name;
        const char* light;
        const char* dark;
    };
    static constexpr Color swatches[] = {
        {"@surface", "#ffffff", "#1c2026"},
        {"@text", "#1f2328", "#ebeff4"},
        {"@muted", "#57606a", "#b0bccb"},
        {"@border", "#d0d7de", "#414a56"},
        {"@controlBorder", "#8c959f", "#697888"},
        {"@panel", "#f6f8fa", "#262d36"},
        {"@hover", "#eef1f4", "#333d49"},
        {"@link", "#0969da", "#91c6ff"},
        {"@selected", "#ddf4ff", "#264567"},
        {"@greenText", "#1a7f37", "#86db9f"},
        {"@greenPanel", "#dafbe1", "#1f3b2b"},
        {"@greenButton", "#1a7f37", "#1e713c"},
        {"@greenButtonHover", "#176d30", "#236f42"},
        {"@alertRow", "#fff8e5", "#342d21"},
        {"@alertPanel", "#fff1c2", "#49391f"},
        {"@alertText", "#7d4e00", "#f0ca82"},
        {"@errorPanel", "#ffebe9", "#482731"},
        {"@errorText", "#a40e26", "#ffa8ba"},
        {"@disabledPanel", "#eaeef2", "#303741"},
        {"@disabledText", "#6e7781", "#92a0ae"},
    };
    const bool dark = isDark();
    for (const auto& color : swatches)
        style.replace(QLatin1StringView(color.name) + QLatin1Char(';'), QLatin1StringView(dark ? color.dark : color.light) + QLatin1Char(';'));
    return style;
}

inline void apply(QWidget* widget, const QString& style)
{
    const auto resolved = colors(style);
    if (widget->styleSheet() != resolved)
        widget->setStyleSheet(resolved);
}
}
