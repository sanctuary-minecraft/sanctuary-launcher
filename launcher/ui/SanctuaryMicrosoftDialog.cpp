// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryMicrosoftDialog.h"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

SanctuaryMicrosoftDialog::Result SanctuaryMicrosoftDialog::run(QWidget* parent, bool french)
{
    const auto text = [french](const char* fr, const char* en) { return QString::fromUtf8(french ? fr : en); };
    Result result;
    QDialog dialog(parent);
    dialog.setObjectName("sanctuaryMicrosoftLogin");
    dialog.setWindowTitle(text("Sanctuary · Connexion Microsoft", "Sanctuary · Microsoft sign-in"));
    dialog.setMinimumWidth(460);
    auto* layout = new QVBoxLayout(&dialog);
    auto label = [&dialog,layout](const QString& name,const QString& value) {
        auto* widget = new QLabel(value,&dialog);
        widget->setObjectName(name);
        widget->setTextFormat(Qt::PlainText);
        widget->setWordWrap(true);
        layout->addWidget(widget);
        return widget;
    };
    label("microsoftIntro",text("Microsoft → Xbox → Minecraft. Le lancement exige une licence et un profil vérifiés.",
                              "Microsoft → Xbox → Minecraft. Game launch requires verified ownership and a profile."));
    auto* checkpoint = label("microsoftCheckpoint",text("Connexion Microsoft à confirmer.", "Microsoft sign-in not confirmed yet."));
    auto* status = label("microsoftStatus",text("Préparation…", "Preparing…"));
    auto* code = label("microsoftCode",{});
    code->setMinimumHeight(64);
    code->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    code->setStyleSheet("font-size:26px; font-weight:600; padding:12px 0;");
    auto* open = new QPushButton(text("Ouvrir Microsoft", "Open Microsoft"),&dialog);
    open->setObjectName("openMicrosoft");
    open->setEnabled(false);
    layout->addWidget(open);
    label("microsoftOtherAccount",text("Autre compte : ouvrez microsoft.com/link dans une fenêtre privée et utilisez ce code.",
                                      "Different account: open microsoft.com/link in a private window and use this code."));
    auto* copy = new QPushButton(text("Copier le code", "Copy code"),&dialog);
    copy->setEnabled(false);
    layout->addWidget(copy);
    QObject::connect(open,&QPushButton::clicked,open,[open] {
        const auto url = open->property("verificationUrl").toUrl();
        if (!url.isEmpty()) QDesktopServices::openUrl(url);
    });
    QObject::connect(copy,&QPushButton::clicked,copy,[code] { QApplication::clipboard()->setText(code->text()); });
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel,&dialog);
    buttons->button(QDialogButtonBox::Cancel)->setText(text("Annuler", "Cancel"));
    layout->addWidget(buttons);
    auto account = MinecraftAccount::createBlankMSA();
    auto flow = account->login(true);
    const auto updateCheckpoint = [account,checkpoint,text] {
        if (!account->accountData()->msaToken.token.isEmpty())
            checkpoint->setText(text("Connexion Microsoft validée ; vérification Xbox/Minecraft en cours.",
                                     "Microsoft sign-in confirmed; checking Xbox/Minecraft."));
    };
    QObject::connect(flow.get(),&Task::status,&dialog,[status,updateCheckpoint](const QString& message) {
        status->setText(message);
        updateCheckpoint();
    });
    QObject::connect(flow.get(),&AuthFlow::authorizeWithBrowserWithExtra,&dialog,
                     [code,status,open,copy,text,&dialog](const QString& uri,const QString& value,int) {
        const QUrl url(uri);
        if (url.scheme() != "https" || (url.host() != "microsoft.com" && url.host() != "www.microsoft.com")
            || (url.path() != "/link" && url.path() != "/devicelogin") || !url.userInfo().isEmpty() || (url.port(-1) != -1 && url.port() != 443)) {
            status->setText(text("Adresse de connexion Microsoft inattendue. Annulez et réessayez.",
                                 "Unexpected Microsoft sign-in address. Cancel and retry."));
            return;
        }
        const bool renewed = !code->text().isEmpty() && code->text() != value;
        code->setText(value);
        status->setText(renewed ? text("Code renouvelé : saisissez le nouveau code affiché ici dans Microsoft.", "Code renewed: enter the new code shown here on Microsoft.")
            : text("Ouvrez Microsoft, saisissez ce code et connectez-vous avec le compte qui possède Minecraft Java.",
                   "Open Microsoft, enter this code and sign in with the account that owns Minecraft Java."));
        open->setProperty("verificationUrl",url);
        open->setEnabled(true);
        copy->setEnabled(true);
        dialog.adjustSize();
    });
    QObject::connect(flow.get(),&Task::failed,&dialog,[&result,account,status,checkpoint,open,copy,code,buttons,text](const QString& reason) {
        const bool microsoft = !account->accountData()->msaToken.token.isEmpty();
        checkpoint->setText(microsoft ? text("Connexion Microsoft validée ; accès Minecraft non confirmé.",
                                             "Microsoft sign-in confirmed; Minecraft access not confirmed.")
                                     : text("Connexion Microsoft non confirmée.", "Microsoft sign-in not confirmed."));
        result.message = checkpoint->text() + "\n" + reason;
        status->setText(reason);
        open->setEnabled(false);
        copy->setEnabled(false);
        code->clear();
        buttons->button(QDialogButtonBox::Cancel)->setText(text("Fermer", "Close"));
    });
    QObject::connect(flow.get(),&Task::succeeded,&dialog,[&result,account,&dialog,text] {
        if (account->ownsMinecraft() && account->hasProfile() && account->accountState() == AccountState::Online) {
            result.account = account;
            result.message = text("Connexion Microsoft, licence et profil Minecraft vérifiés.",
                                  "Microsoft sign-in, Minecraft ownership and profile verified.");
        } else result.message = text("Microsoft connecté ; licence ou profil Minecraft non confirmé.",
                                     "Microsoft connected; Minecraft ownership or profile not confirmed.");
        dialog.accept();
    });
    QObject::connect(flow.get(),&Task::aborted,&dialog,&QDialog::reject);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    QTimer::singleShot(0,flow.get(),&Task::start);
    dialog.exec();
    if (flow->isRunning()) {
        flow->abort();
        result.message = text("Connexion Microsoft annulée.", "Microsoft sign-in cancelled.");
    }
    if (result.message.isEmpty()) result.message = text("Connexion Microsoft annulée ou code expiré.", "Microsoft sign-in cancelled or code expired.");
    return result;
}
