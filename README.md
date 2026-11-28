# Sanctuary Launcher

Fork natif de **Prism Launcher 11.1.1**, avec une fenêtre Sanctuary blanche,
les outils du jeu intégrés et une seule installation officielle Sanctuary Beta.
La version **0.1.8** est distribuée pour Windows x64, macOS Apple Silicon et Linux x64.

[Téléchargements Sanctuary](https://sanctuary-minecraft.net/telechargements) · [Sources publiques et releases GitHub](https://github.com/sanctuary-minecraft/sanctuary-launcher)

## Interface actuelle

**Jouer** ouvre le tableau de bord dès le démarrage. La date et
l’heure locales apparaissent juste en dessous et s’actualisent automatiquement.
Le tableau de bord partage les bords de contenu des pages natives.
Les trois flux sont encadrés sur un fond gris très léger, identique au gris
des tableaux natifs. L’Intendance reste en haut sur toute la largeur, la Gazette
à gauche et le Tableau à droite en dessous. Chaque cadre affiche son titre :
Intendance, Gazette ou Tableau d’affichage. Le titre reste visible au-dessus
du contenu dépliable, sans barre d’actions supplémentaire.
Les publications de Gazette et du Tableau montrent le visage du skin Minecraft
de leur auteur près du titre ; l’Intendance reste sans visage.
Un clic déplie le texte complet et les photos de la Gazette, comme sur le site.
Chaque flux peut défiler dans le body ; Jouer reste fixé en bas à droite.

Le bouton de connexion dans la barre latérale indique l’étape suivante.
Il ouvre Discord, la connexion Microsoft, la vérification de l’accès Sanctuary
ou l’installation du pack. Son infobulle décrit la procédure. Il est rouge
au départ, jaune pendant la configuration et vert une fois les étapes
obligatoires terminées. **Connecté** donne accès aux actions de compte et au
dossier Sanctuary, qui contient aussi le code de personnage facultatif.

La barre latérale espace **Jouer**, le groupe **Actualités**, **Carte** et
**Wiki**, puis les outils de l’instance. Les pages Sanctuary prennent un
accent bleu au survol. Les outils sont neutres au repos et prennent un
accent violet.

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

**La connexion Microsoft est testable en développement** depuis la barre latérale.
Le parcours par code réutilise le moteur Prism : Microsoft, Xbox, services
Minecraft, licence et profil. Le dialogue indique si Microsoft réussit alors
que la vérification Minecraft échoue. Un compte est ajouté uniquement si la
licence et le profil sont confirmés. Le **13 novembre**, une nouvelle recette
réelle confirme Microsoft, la licence et le profil Minecraft : le 403 du
9 novembre ne se reproduit plus. Aucune réponse formelle AppID n’a été consultée.
Le test ne change pas le Trousseau Discord ni l’admission Sanctuary. Voir
[le contrat et la recette Microsoft](docs/microsoft.md). Si le navigateur
reprend le mauvais compte, ouvrir microsoft.com/link dans une fenêtre privée
permet d’utiliser le compte qui possède le jeu sans déconnecter Azure.
**Jouer** utilise maintenant le lancement en ligne Prism et rejoint directement
`beta.sanctuary-minecraft.net`, avec résolution SRV native. Le compte Minecraft
doit correspondre au pseudo du dossier Discord admis. Le pack doit être vérifié,
et aucune synchronisation ni partie ne doit être en cours. Les scénarios de
démonstration ne donnent pas accès au jeu réel. La console reste dans l’onglet
Journaux du jeu ; les éditions du pack et les mises à jour sont suspendues
pendant le lancement et la partie. Voir [la recette de lancement](docs/implementation/play-server-20261113.md).

La session du launcher est limitée à la lecture et expire après sept jours.
Le jeton Sanctuary est protégé par Windows (DPAPI), le Trousseau macOS ou
le coffre de la session Linux s'il est disponible. Chaque profil dispose
de sa propre session. La déconnexion efface la copie locale, révoque la
session et vide les contenus affichés.

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

Voir le [cahier des charges 0.11](docs/cahier-des-charges.md), le
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
Windows, Linux et les réglages graphiques/audio complets restent à valider.
Le **13 novembre**, lancement en ligne, arrivée directe sur la Beta et présence
du personnage sont confirmés par les journaux client/serveur et par Koka.
Le client utilise réellement Vulkan / MoltenVK. La [recette](docs/implementation/play-server-20261113.md)
distingue ces contrôles d’une validation graphique ou audio complète.

Un push ne publie aucun binaire et ne déclenche pas Sanctuary Update.

## Origine et distribution

Les historiques du dépôt et de Prism restent conservés, avec les licences
[GPL-3.0-only](LICENSE), [COPYING](COPYING.md) et le
[README amont](docs/upstream/Prism-README.md). Base : `upstream/prism/11.1.1`,
commit `7d2d3c1ec6fcf8255ebacd02d9f792688575dfb4`.
Sanctuary Launcher est un fork indépendant. Les identifiants Microsoft,
CurseForge et Imgur amont et les mises à jour du binaire Prism sont désactivés.
Les métadonnées Minecraft et traductions publiques restent utilisées.

L'[application Microsoft Sanctuary](docs/microsoft.md) accède aux services
Minecraft dans la recette du 13 novembre ; aucune réponse formelle à la demande
AppID n’a été consultée. La distribution autonome sera préparée avant livraison aux joueurs.
Les dépôts restent privés. Les anciennes fondations Release restent sous
`archive/release-foundation-20261108`.
