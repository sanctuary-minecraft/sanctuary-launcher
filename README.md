# Sanctuary Launcher

Fork natif de **Prism Launcher 11.1.1**, avec une fenêtre Sanctuary blanche,
les outils du jeu intégrés et une seule installation officielle Sanctuary Beta.
La version **0.1.0 de développement** fonctionne localement sur macOS Apple Silicon.

## Interface actuelle

**Accueil** reprend le titre et la typographie des autres pages. La date et
l’heure locales apparaissent juste en dessous et s’actualisent automatiquement.
Accueil et Bien démarrer utilisent les mêmes bords de contenu que les pages
natives, sans marge latérale ou largeur maximale supplémentaire dans la fenêtre.
Les trois flux sont encadrés sur un fond gris très léger, identique au gris
des tableaux natifs. L’Intendance reste en haut sur toute la largeur, la Gazette
à gauche et le Tableau à droite en dessous. Chaque cadre affiche son titre :
Intendance, Gazette ou Tableau d’affichage. Le titre reste visible au-dessus
du contenu dépliable, sans barre d’actions supplémentaire.
Les publications de Gazette et du Tableau montrent le visage du skin Minecraft
de leur auteur près du titre ; l’Intendance reste sans visage.
Un clic déplie le texte complet et les photos de la Gazette, comme sur le site.
Chaque flux peut défiler dans le body ; Jouer reste fixé en bas à droite.

**Bien démarrer** est le premier onglet, permanent dans la même fenêtre. Il conserve
les cinq étapes : Discord, Microsoft, accès Sanctuary, code SGA facultatif et
installation. Il reste accessible après la connexion Discord, avec l'étape
Microsoft visible et explicitement en attente d'autorisation. Une première
arrivée sans session Discord ou sans pack vérifié ouvre cette page. Un joueur
ayant une session et le pack retrouve l'accueil, avec un lien vers le tutoriel.
Les deux pages partagent la même session et la même installation.

**Actualités** consulte l'historique des publications du site dans un onglet
séparé. Le logo PNG original occupe seul le haut de la barre latérale ; il est
dessiné depuis sa résolution complète pour les écrans Retina, sans réduction
intermédiaire en basse résolution.

**Discord est raccordé au site Sanctuary** : Connecter Discord ouvre le
navigateur, puis une confirmation lie ce launcher au compte déjà présent.
Le dossier et l'état de whitelist sont relus sur le site, sans candidature
ou admission créée automatiquement. Les trois panneaux utilisent les vraies
publications privées, après récupération d'un accès confirmé. Koka confirme
la connexion réelle et leur affichage le 8 novembre 2026.

Microsoft/Mojang reste à raccorder : **Jouer reste désactivé**, même si Discord
retrouve un accès accordé. Les comptes Microsoft simulés des tests ne valent
pas une connexion réelle. Les contenus communautaires peuvent être lus sans
attendre Microsoft ; les étapes et leur état restent dans Bien démarrer.

La session du launcher est limitée à la lecture et expire après sept jours.
Sur macOS, le jeton est conservé dans le Trousseau, séparément pour chaque
profil du launcher ; aucun jeton Discord ou mot de passe n'est embarqué.
La déconnexion révoque la session et vide les contenus affichés. Les autres
plateformes gardent la session en mémoire pour ce lot.

Le code SGA reste facultatif et masqué. Le badge des joueurs historiques de
la bêta est prévu ; son exemple local ne constitue pas une admission réelle.
Les vingt places concernent les connexions simultanées, sans plafond de membres.

Voir le [contrat Discord et la recette](docs/discord-connexion.md).

Voir aussi la [recette de l’accueil minimal et du tutoriel](docs/implementation/accueil-minimal-20261108.md).

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

Voir le [cahier des charges 0.9](docs/cahier-des-charges.md), le
[contrat des outils](docs/outils-prism.md) et la
[recette du lot local](docs/implementation/launcher-local-20261108.md), complétée
par les recettes de l'[accueil fixe](docs/implementation/accueil-fixe-20261108.md)
et du [header intégré](docs/implementation/accueil-header-20261108.md).
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

Les **25 suites CTest** passent. La navigation et l'installation ont également
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
