// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "minecraft/auth/MinecraftAccount.h"

class QWidget;

/** Development sign-in through Prism's device-code authentication chain. */
class SanctuaryMicrosoftDialog {
public:
    struct Result {
        MinecraftAccountPtr account;
        QString message;
    };
    static Result run(QWidget* parent, bool french);
};
