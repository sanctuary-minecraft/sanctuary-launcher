# Sanctuary Launcher

Préparation du launcher dédié à Sanctuary, fondé sur un futur fork de
[Prism Launcher](https://github.com/PrismLauncher/PrismLauncher).
**Aucun fork Prism ni nouvelle authentification Microsoft n'est implémenté.**

Le parcours demandé est : télécharger le launcher, se connecter à Microsoft,
installer le pack, lire les actualités du site et rejoindre directement le
serveur. Le profil initial souhaité active les shaders Sanctuary et Sound
Physics, puis respecte les préférences du joueur. Ces activations ne sont pas
réalisées par cet import.

## Ce qui existe déjà

`existing/launcher/packwiz/` contient l'installateur packwiz habillé Sanctuary,
ses sources Java, sa licence, son logo et ses empreintes. `existing/scripts/`
conserve les outils de construction d'origine ; ils utilisent l'arborescence
complète de [sanctuary](https://github.com/sanctuary-minecraft/sanctuary). Les JAR générés ne sont pas versionnés.

Le [parcours Prism actuel](existing/docs/beta-download.md) télécharge et met
déjà le pack à jour. [sanctuary-pack](https://github.com/sanctuary-minecraft/sanctuary-pack) conserve ses manifests ; les
actualités FR/EN proviennent de [sanctuary-web](https://github.com/sanctuary-minecraft/sanctuary-web).

## Connexion Microsoft à préparer

Créer une application **Sanctuary Launcher** dans Entra, puis demander l'accès
Minecraft avec son propre Client ID. La création du compte dédié a rencontré
une vérification humaine en boucle ; aucune application ni demande d'accès
n'a été créée ou envoyée dans cette séance.

- [Inscription Entra](https://learn.microsoft.com/en-us/entra/identity-platform/quickstart-register-app)
- [Procédure Mojang](https://help.minecraft.net/hc/en-us/articles/16254801392141)
- [Formulaire AppID Review](https://aka.ms/mce-reviewappid)
- [Politique des forks Prism](https://github.com/PrismLauncher/PrismLauncher#forkingredistributingcustom-builds-policy)

Le compte administrateur dédié doit rester distinct des comptes de jeu des
joueurs. Aucun secret client, token ou compte de jeu n'appartient au dépôt.
Le cadrage antérieur est conservé sous [`archive/release-foundation-20261108`](https://github.com/sanctuary-minecraft/sanctuary-launcher/tree/archive/release-foundation-20261108).
