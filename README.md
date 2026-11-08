# Sanctuary Launcher

Fork natif de **Prism Launcher 11.1.1**, avec une fenêtre Sanctuary blanche,
les outils du jeu intégrés et une seule installation officielle Sanctuary Beta.
La version **0.1.0 de développement** fonctionne localement sur macOS Apple Silicon.

## Interface actuelle

**Accueil** est le premier onglet, avant les journaux du jeu. **Actualités**
consulte les dernières publications FR/EN du site dans un onglet séparé.
Dans Accueil, le bloc des cinq étapes apparaît tant que le parcours n'est pas
prêt : Discord, Microsoft, accès Sanctuary, code de personnage facultatif,
installation. L'accueil tient sans défilement dans la fenêtre : le header
nomme l'étape active, le body contient les étapes et le footer garde Jouer
en bas à droite, même pendant l'attente d'admission.
Lorsque les prérequis sont réunis, ce bloc disparaît au profit de **Jouer**,
**Intendance**, **Gazette** et **Tableau d'affichage**. Ces rubriques occupent
des panneaux fixes avec un aperçu et un lien de lecture complète ; le footer
reste à sa place.

Les comptes et l'accès sont **simulés dans le profil local de développement**.
Le pack téléchargé est réel. Le bouton Jouer local teste le parcours sans
lancer Minecraft ni rejoindre un serveur. Le raccordement Microsoft/Mojang,
Discord et des admissions reste à réaliser. Les publications communautaires
privées demandent une session Discord du site ; aucune fausse publication
n'est présentée pour la remplacer.

Le code SGA reste facultatif et masqué. Le badge des joueurs historiques de
la bêta est prévu ; son exemple local ne constitue pas une admission réelle.
Les vingt places concernent les connexions simultanées, sans plafond de membres.

## Pack et outils natifs

Installer télécharge l'archive officielle du site, puis vérifie le pack avec
le canal stable existant de **Sanctuary Update**. Les mises à jour réutilisent
la même instance. Le contrôle local du 8 novembre 2026 a installé **beta.373**,
avec onze JAR client, puis effectué un second passage en conservant fichiers
personnels de test et désactivation d'un mod facultatif.

Mods, ressources, shaders, captures, mondes, serveurs, versions, notes,
réglages et journaux réutilisent les pages natives Prism dans la fenêtre
Sanctuary. Les réglages globaux s'ouvrent également dans cette fenêtre.
Les sous-dialogues utiles, sélecteurs de fichiers et ouvertures explicites
du navigateur ou du gestionnaire de fichiers restent natifs.

Voir le [cahier des charges 0.6](docs/cahier-des-charges.md), le
[contrat des outils](docs/outils-prism.md) et la
[recette du lot local](docs/implementation/launcher-local-20261108.md), complétée
par la [recette de l'accueil fixe](docs/implementation/accueil-fixe-20261108.md).
Les [maquettes conservées](docs/design/README.md) documentent les recherches
précédentes ; l'application native devient la référence de travail.

## Construire et lancer

```sh
git submodule update --init --recursive
./script/build_and_run.sh --verify
```

Le [guide de développement](docs/developpement.md) décrit les dépendances.
Le bouton Run de Codex utilise ce script. Les données de test restent dans
`build/local-launcher`, distinctes de Prism personnel. Fermer l'application
avant de reconstruire lorsqu'une mise à jour ou une édition est en cours.
Le script relance uniquement ce profil de développement.

Les **24 suites CTest** passent. La navigation et l'installation ont également
été exercées dans la fenêtre native. Le bundle `build/native/sanctuary-launcher.app`
dépend de Homebrew, cible macOS 26 et n'est ni signé ni notarisé.
Windows, Linux, le lancement réel du jeu, la connexion au serveur, les
personnages et les réglages graphiques/audio restent à valider.

Un push ne publie aucun binaire et ne déclenche pas Sanctuary Update.

## Origine et distribution

Les historiques du dépôt et de Prism restent conservés, avec les licences
[GPL-3.0-only](LICENSE), [COPYING](COPYING.md) et le
[README amont](docs/upstream/Prism-README.md). Base : `upstream/prism/11.1.1`,
commit `7d2d3c1ec6fcf8255ebacd02d9f792688575dfb4`.
Sanctuary Launcher est un fork indépendant. Les identifiants Microsoft,
CurseForge et Imgur amont et les mises à jour du binaire Prism sont désactivés.
Les métadonnées Minecraft et traductions publiques restent utilisées.

L'[application Microsoft Sanctuary](docs/microsoft.md) est créée ; sa demande
d'accès Minecraft attend une décision. Les sources correspondantes et la
distribution autonome seront préparées avant livraison aux joueurs.
Les dépôts restent privés. Les anciennes fondations Release restent sous
`archive/release-foundation-20261108`.
