# Développer le fork Sanctuary

## Base et état du premier lot

Le premier lot est développé sur `codex/launcher-prism-foundation` depuis
Prism **11.1.1**, commit `7d2d3c1ec6fcf8255ebacd02d9f792688575dfb4`.
La fusion conserve les deux historiques Git. Le sous-module
`libraries/libnbtplusplus` reste épinglé au commit amont
`3538933614059f0f44388a2b16f3db25ce42285b`.

La cible de la révision 0.4 du [cahier des charges](cahier-des-charges.md)
est la DA blanche, le **bloc unique numéroté de 1 à 5** et le **titre de
l'étape active**, avec code SGA facultatif, installation possible pendant
l'attente et activation normale sans saisie du code. La [maquette actuelle](design/README.md) conserve
le badge historique et reprend le mot-symbole du site dans l'en-tête ; elle
reste une cible statique. Le tableau ci-dessous décrit le premier lot
existant à paysage, pas l'implémentation de cette nouvelle cible. Le raccordement
des admissions et l'activation automatique exigent encore des contrats côté
site et mod ; les critères de recette correspondants restent à exécuter.

Koka demande également de conserver **toutes les options de Prism**. Les pages
d'instance sont déjà référencées par `launcher/InstancePageProvider.h`, et les
outils classiques restent derrière l'entrée avancée du premier accueil dans
`launcher/ui/MainWindow.cpp`. Les réglages globaux sont déclarés dans
`launcher/Application.cpp`. Cette lecture du code confirme leur présence dans
la base, pas une nouvelle recette fonctionnelle. L'entrée **Tous les outils**
de la maquette blanche et ses raccourcis restent à raccorder selon le
[contrat dédié](outils-prism.md).

| Partie | État vérifiable |
| --- | --- |
| LAUNCH-01, base et identité | Source Prism importée ; binaire, App ID, icônes macOS/Windows et données propres ; compilation macOS locale |
| LAUNCH-02, accueil | Widget natif Qt FR/EN, liens, commandes Réglages/Outils avancés ; rendus contrôlés à 1280 × 800 |
| Installation | Import Prism existant avec URL officielle préremplie ; parcours réel non encore testé |
| Actualités | Instantané embarqué des publications beta.372, beta.370 et beta.369 ; liens vers le site |
| Comptes et jeu | Application Entra créée ; Client ID non raccordé au binaire, connexion et Jouer désactivés ; aucun test de connexion Minecraft |
| Distribution | Bundle de développement lié à Homebrew ; aucun paquet autonome ni déploiement |

Le profil mémorisé est désigné par `SanctuaryInstanceId`, enregistré pendant
un import demandé depuis l'accueil. Son existence ne prouve ni sa version,
ni son intégrité, ni son état de mise à jour : le texte indique seulement
« Profil Sanctuary préparé ». La prochaine étape doit contrôler le contenu
importé et verrouiller l'origine du pack avant de déclarer une installation
prête. Le dialogue Prism permet encore de modifier la sélection.

## Construire sur le poste macOS

Prévoir les outils en ligne de commande Apple, Homebrew et CMake. Le poste
de validation utilise macOS **26.5.1 arm64**, Qt **6.12.0**, Ninja **1.13.2**,
ECM **6.31.0**, cmark **0.31.2**, libarchive **3.8.11**, qrencode **4.1.1**
et tomlplusplus **3.4.0**. Les dépendances sont locales et ignorées par Git.

```sh
brew install cmake ninja extra-cmake-modules qtbase qtnetworkauth qtimageformats qtsvg cmark libarchive qrencode tomlplusplus openjdk@17
git submodule update --init --recursive
./scripts/build-sanctuary-macos.sh
```

Le script choisit explicitement **JDK 17 pour compiler les auxiliaires Java
de Prism**, dont le niveau de source est ancien. Le **jeu Sanctuary requiert
Java 25** : le JDK de construction ne remplace pas cette exigence.

Le résultat local est `build/native/sanctuary-launcher.app`. Le script cible
macOS **26.0**, car les bibliothèques Homebrew du poste ne permettent pas de
revendiquer une compatibilité avec un système plus ancien. Le bundle dépend
de ces bibliothèques ; ni copie autonome des frameworks, ni signature, ni
notarisation ne sont validées. Ne pas distribuer ce bundle comme installateur.

La recette de compilation officielle de Prism reste une référence pour les
[autres plateformes](https://prismlauncher.org/wiki/development/build-instructions/),
sans preuve de compatibilité du fork Sanctuary tant qu'elles ne sont pas testées.

## Données et services

Le dossier macOS par défaut est
`~/Library/Application Support/SanctuaryLauncher`. L'App ID est
`net.sanctuary-minecraft.launcher`. La migration automatique historique depuis
PolyMC/MultiMC est retirée : aucune importation de comptes Prism n'est prévue
par ce lot. Le launcher peut télécharger ses métadonnées et traductions
publiques depuis les services Prism existants.

| Configuration amont | Choix Sanctuary |
| --- | --- |
| Microsoft Client ID | Vide ; capacité de connexion absente |
| Clé CurseForge, Client ID Imgur | Vides |
| Updater GitHub Prism | URL vide |
| Sparkle Prism | Clé publique et flux vides |
| Flux RSS Prism | URL vide ; lecture RSS ignorée |
| Métadonnées du jeu, traductions | Services publics Prism conservés |
| Aide des dialogues avancés | Documentation Prism conservée |

Les workflows GitHub de construction et publication amont ne sont pas activés
dans le fork. Leurs définitions restent consultables dans l'historique amont.
Aucun push n'a valeur de livraison Sanctuary Update. Ce lot n'avance aucun
canal packwiz et ne modifie ni site, ni serveur, ni instance Prism personnelle.

Le français est le choix initial. L'accueil possède ses textes FR/EN propres ;
les dialogues Prism utilisent leur système de traductions existant et peuvent
rester en anglais avant le téléchargement du catalogue. La localisation du
fork complet reste à vérifier, tout comme les polices et l'échelle système.

## Recette effectuée

La compilation Release C++/Qt et les **22 suites CTest** réussissent. La suite
`SanctuaryHome` contrôle l'encombrement FR/EN à 1280 × 800, l'action Installer
sans Microsoft, le blocage de Jouer sans application Microsoft, les signaux
Réglages/Outils avancés après reconstruction et les liens d'actualités. Elle
vérifie aussi que l'accueil ne déclare pas le serveur en ligne ou le pack à jour.

```sh
SANCTUARY_CAPTURE_DIR="$PWD/build/native/captures" QT_QPA_PLATFORM=offscreen \
  ctest --test-dir build/native --output-on-failure
```

Les rendus sont produits par Qt pendant ses tests, sans capture du bureau.
Le [rendu français conservé](implementation/accueil-qt-fr.png) montre l'état
initial sans pack ni compte. L'application démarre sur le poste avec son
dossier neuf, mais la lecture automatique de sa fenêtre via l'accessibilité
macOS a expiré. Les clics réels, l'import complet, le jeu, le serveur, Vulkan,
les réglages shaders/Sound Physics et les autres plateformes restent à tester.

## Reprise

Poursuivre LAUNCH-03 sur un dossier de développement isolé : verrouiller le
pack officiel, vérifier métadonnées/empreintes, gérer interruption et reprise,
et tester deux installations/synchronisations sans duplication. LAUNCH-04
définit ensuite le flux public d'actualités dans le site. L'application Entra
est [créée et documentée](microsoft.md) ; l'accès Minecraft puis le raccordement
du Client ID restent à traiter avant les essais LAUNCH-06/07.
Les objectifs et critères complets restent dans le
[cahier des charges](cahier-des-charges.md).
