# Sanctuary Launcher

Préparation du launcher dédié à Sanctuary, fondé sur un futur fork de
[Prism Launcher](https://github.com/PrismLauncher/PrismLauncher).
**Aucun fork Prism ni nouvelle authentification Microsoft n'est implémenté.**

## Maquette et cahier des charges

La **[maquette d'accueil validée le 8 novembre 2026](docs/design/README.md)**
est conservée dans ce dépôt, avec son fichier original et son empreinte.
Elle reprend le style du site, les actualités et le bouton Jouer.

Le **[cahier des charges du fork](docs/cahier-des-charges.md)** décrit les
parcours, le lien avec Sanctuary Update, les critères de recette et le
découpage en tickets. Prism **11.1.1** y est une base proposée à confirmer
avant développement. L'accueil est validé ; les fonctions du futur fork
restent à implémenter. Aucun code, exécutable ou déploiement n'est ajouté
par ce cadrage.

Le parcours demandé est : télécharger le launcher, installer le pack, lire
les actualités du site, se connecter à Microsoft et rejoindre directement le
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
Minecraft avec son propre Client ID. Après le blocage de la création d'un
compte dédié, Koka a choisi de reprendre avec son compte Microsoft existant.
La connexion est momentanément en attente ; aucune application ni demande
d'accès n'a été créée ou envoyée. Le cadrage peut avancer pendant cette attente.

- [Inscription Entra](https://learn.microsoft.com/en-us/entra/identity-platform/quickstart-register-app)
- [Procédure Mojang](https://help.minecraft.net/hc/en-us/articles/16254801392141)
- [Formulaire AppID Review](https://aka.ms/mce-reviewappid)
- [Politique des forks Prism](https://github.com/PrismLauncher/PrismLauncher#forkingredistributingcustom-builds-policy)

Le compte d'administration de l'application, l'authentification des joueurs
et le compte Discord du site ont des rôles distincts. Aucun secret client,
token ou compte de jeu n'appartient au dépôt.
Le cadrage antérieur est conservé sous [`archive/release-foundation-20261108`](https://github.com/sanctuary-minecraft/sanctuary-launcher/tree/archive/release-foundation-20261108).
