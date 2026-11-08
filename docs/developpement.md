# Développer le fork Sanctuary

## Base et périmètre local

Base Prism **11.1.1**, commit `7d2d3c1ec6fcf8255ebacd02d9f792688575dfb4`.
Le sous-module `libraries/libnbtplusplus` reste épinglé à
`3538933614059f0f44388a2b16f3db25ce42285b`.
Le lot `codex/launcher-local-onboarding` porte le
[cahier des charges 0.9](cahier-des-charges.md) : fenêtre unique, Accueil et
Actualités dans la navigation, cinq étapes avant préparation puis rubriques
communautaires. La [recette](implementation/launcher-local-20261108.md)
conserve la recette initiale. Le [raccordement Discord](discord-connexion.md)
utilise désormais le vrai compte et le dossier du site ; Microsoft reste différé.

## Construire et lancer sur macOS

Dépendances du poste : macOS **26.5.1 arm64**, Qt **6.12.0**, Ninja **1.13.2**,
ECM **6.31.0**, cmark **0.31.2**, libarchive **3.8.11**, qrencode **4.1.1**,
tomlplusplus **3.4.0**, CMake et outils Apple.

```sh
brew install cmake ninja extra-cmake-modules qtbase qtnetworkauth qtimageformats qtsvg cmark libarchive qrencode tomlplusplus openjdk@17
git submodule update --init --recursive
./script/build_and_run.sh --verify
```

Le script construit le bundle, exécute CTest et ouvre le profil isolé
`build/local-launcher`. Il conserve ses données aux relances et initialise
`SanctuaryLocalMode=true` uniquement lors de sa création. Il arrête uniquement
le processus de ce profil avant reconstruction : terminer les téléchargements
et enregistrer les éditions avant Run. Les modes `--logs` et `--debug`
ouvrent respectivement les journaux système et LLDB. L'action Run de
`.codex/environments/environment.toml` pointe vers ce script.

Pour construire sans relancer : `./scripts/build-sanctuary-macos.sh`.
Les auxiliaires Java de Prism sont compilés avec **JDK 17**. Ce JDK exécute
également le bootstrap packwiz dans le profil de test ; **le jeu requiert
Java 25**. Aucun démarrage du jeu n'est effectué par cette recette.

Le bundle `build/native/sanctuary-launcher.app` cible macOS **26.0** et dépend
des bibliothèques Homebrew. Il n'est ni autonome, ni signé, ni notarisé.
Les autres plateformes restent à compiler et à tester.

## Structure de l'interface

- `SanctuaryWorkspace` héberge les `BasePage` natives dans un `QStackedWidget`,
  avec Bien démarrer/Accueil/Actualités, une colonne logo/navigation à gauche et la colonne de contenu alignée
  sur le logo en haut. Aucun nom de pack n'est accolé au PNG. Bien démarrer,
  Accueil et Actualités précèdent les pages du jeu.
- `SanctuaryHome` utilise un header d'étape, un body flexible et un footer
  fixe avec Jouer en bas à droite. Aucun `QScrollArea` ne contient l'accueil.
  Deux vues partagent les mêmes contrôleurs : Dashboard affiche Accueil,
  la date/heure en sous-titre et les trois flux encadrés sur fond #f6f8fa ;
  Setup conserve les cinq étapes après Discord. Adaptive reste
  le composant de référence pour les tests historiques. Le footer reste fixe ;
  les publications ouvertes peuvent défiler dans les flux. Le code SGA reste facultatif.
- `SanctuaryFeed` présente les publications de Gazette et du Tableau avec leur
  auteur et un visage 32 × 32 chargé depuis le même service public que le site,
  sans Authorization. L’Intendance reste sans visage ni requête de skin.
  Les visages PNG/WebP sont bornés et conservés seulement en mémoire. Les titres
  déplient une publication par flux ; les photos JPEG privées de la Gazette sont
  chargées à la demande via SanctuarySession, avec limites 49 152 octets et
  960 × 540 avant décodage. Les textes restent simples, sans HTML exécuté.
- `SanctuarySession` gère l'association dans le navigateur, la session de
  lecture et sa conservation dans le Trousseau macOS. Aucun secret Discord
  n'est dans le client. Le cache local de simulation ne vaut jamais admission.
- `SanctuaryFeed` lit les actualités publiques et les flux privés authentifiés
  via l'API launcher du site. Une déconnexion efface les aperçus et invalide
  les réponses anciennes ; une réponse 401 demande une reconnexion.
- `SanctuaryPack` importe le ZIP officiel sans assistant séparé, résout le canal
  stable vers son manifeste immuable, contrôle les outils et les fichiers gérés,
  puis conserve le même `SanctuaryInstanceId`.
- `MainWindow` assemble les pages de `InstancePageProvider` et les réglages
  globaux dans la fenêtre Sanctuary. La liste générique multi-instance est masquée.

Une page invalide empêche de quitter son formulaire. Les réglages des pages ouvertes sont
appliqués lors du changement de page et via Enregistrer ; les pages jamais
visitées ne réécrivent pas les préférences avec leurs valeurs de repli. Les outils de
fichiers du jeu sont désactivés pendant la synchronisation du pack.
Les sous-dialogues natifs des outils restent possibles.

## Données et services

Le dossier par défaut reste `~/Library/Application Support/SanctuaryLauncher` ;
App ID : `net.sanctuary-minecraft.launcher`. Run utilise exclusivement le dossier
ignoré `build/local-launcher`. Aucun compte, jeton ou profil personnel Prism
n'est importé. `SanctuaryLocalState` est un état de démonstration séparé ;
`SanctuaryPackVersion` et `SanctuaryPackHash` décrivent le pack vérifié.
Les simulations ne permettent aucun lancement de Minecraft ou accès serveur.
Le parcours runtime utilise Discord réel, même quand SanctuaryLocalMode est
activé pour le développement. Le Client ID Microsoft reste absent ; les
publications sont accessibles après confirmation du dossier Sanctuary, sans
prétendre que le compte Minecraft est déjà vérifié.

Le Client ID Microsoft du binaire reste vide jusqu'au raccordement prévu.
Les clés CurseForge/Imgur et les mises à jour automatiques du binaire Prism
restent désactivées. Les métadonnées du jeu, traductions et aides publiques
Prism restent utilisées. Les textes propres à Sanctuary sont FR/EN ; les
pages héritées peuvent rester en anglais sans catalogue téléchargé.

## Vérifications

```sh
SANCTUARY_CAPTURE_DIR="$PWD/build/native/captures-local" QT_QPA_PLATFORM=offscreen \
  ctest --test-dir build/native --output-on-failure
```

Les **25 suites** passent. Les tests propres à Sanctuary contrôlent les
états du parcours, la séparation simulation/pack réel, le code facultatif,
la navigation dans une seule fenêtre, les formulaires invalides, le lecteur
d'actualités et les empreintes du pack. Les PNG produits sont des rendus de
widgets Qt, distincts des observations de la fenêtre native.

La [recette de l'accueil fixe](implementation/accueil-fixe-20261108.md)
contrôle les dix scénarios, deux langues et trois tailles, y compris le code
SGA ouvert : footer visible, boutons accessibles, aucun défilement de page.

La recette native du lot précédent comprend l'import réel du site et deux passages packwiz
sur la même instance, avec conservation de sentinelles locales et d'un mod
facultatif désactivé. Elle ne valide ni Minecraft, ni personnage, ni serveur,
ni Vulkan/shaders/Sound Physics. Les essais graphiques futurs ciblent Vulkan.
Aucun site, serveur, canal packwiz ni Prism personnel n'est modifié par ce lot.
