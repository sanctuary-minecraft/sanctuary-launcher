# Distribuer Sanctuary Launcher

Le téléchargement sur [sanctuary-minecraft.net](https://sanctuary-minecraft.net/telechargements)
est le point d'installation initial. Les exports sont préparés dans des dossiers
de build ignorés. Chaque version distribuée doit avoir son archive de sources
correspondantes, avec les sous-modules épinglés et les fichiers de build.

## Distribution GitHub

Le [dépôt public du launcher](https://github.com/sanctuary-minecraft/sanctuary-launcher)
contient les sources de la distribution et les builds GitHub Actions.
Pour publier une version GitHub, mettre à jour `Launcher_RELEASE_VERSION`
dans `CMakeLists.txt`, puis pousser un nouveau tag `vMAJEUR.MINEUR.CORRECTIF`
sur le commit à livrer, par exemple `v0.1.7` pour la version `0.1.7`.

Le workflow **Build Sanctuary Launcher** vérifie que le tag correspond à la
version du code. Il compile et teste Windows x64, macOS 26 sur Apple Silicon
et Linux x64. Après réussite des trois builds, il prépare la release en
brouillon, ajoute les trois ZIP, l'archive source avec les sous-modules et
`SHA256SUMS`, puis publie la release. Une relance peut compléter un brouillon
avec les mêmes artefacts. Tout fichier déjà présent avec une autre empreinte
fait échouer la publication. Les lancements manuels par plateforme produisent
des artefacts de compilation dans Actions.

La release historique `v0.1.6` contient les fichiers signés distribués sur le
site. Pour chaque nouvelle version, la mise à jour automatique des installations
passe par la procédure de signature du canal HTTPS ci-dessous. La clé privée
Ed25519 reste sur le poste de publication.

## Construire les exports

Configurer CMake avec `Launcher_RELEASE_VERSION`, avec la clé publique
`Launcher_SANCTUARY_UPDATE_KEY` et avec
`Launcher_MSA_CLIENT_ID=8e8a2428-a727-4668-98ea-2fdaf0037943` pour utiliser
l'application publique Sanctuary. Un profil vierge affiche
**Connecter Microsoft**. Le flux par code passe ensuite par Xbox, la licence et
le profil Minecraft Java. L'application Entra est un client public, sans secret
à intégrer. Les builds locaux conservent leur parcours de développement.

| Cible | Préparation | ZIP reconnu par le site |
| --- | --- | --- |
| Windows x64 | Qt 6, JDK 17, MSYS2 CLANG64, WinSparkle 0.9.4 et NSIS. Renseigner `Launcher_WINSPARKLE_DIR`, exécuter `cmake --install` et passer le dossier installé à `program_info/sanctuary_install.nsi` avec les paramètres `VERSION`, `PAYLOAD` et `OUTPUT`. | `Sanctuary-Launcher-windows-x64.zip` |
| macOS 26 et Apple Silicon | CMake, Qt 6, JDK 17. `scripts/export-sanctuary-macos.sh <build> <exports>` archive le bundle avec Qt et ses trois JAR, applique une signature ad hoc et vérifie le bundle. | `Sanctuary-Launcher-macos-arm64.zip` |
| Linux x64 | CMake, Qt 6, JDK 17, OpenSSL 3, Qt6Keychain et libsecret. `scripts/export-sanctuary-linux.sh <build> <outils> <exports>` utilise linuxdeploy, son module Qt et appimagetool pour créer un AppImage. | `Sanctuary-Launcher-linux-x64.zip` |

Tester Microsoft avec un compte Minecraft Java depuis un profil vierge. Vérifier
la prise en compte d'un changement de compte et l'annulation sans compte ajouté.
Tester le Packwiz du site, les auxiliaires Java et l'ouverture du ZIP sur chaque
plateforme. Le serveur utilise Java 25 pour le jeu. Vérifier la restauration de
la session Sanctuary dans chaque coffre avec `SANCTUARY_TEST_SESSION_STORE=1`
sur un profil de recette isolé. Linux nécessite un coffre de session disponible.

## Canal stable du launcher

Les trois builds vérifient les mises à jour avec la clé publique Ed25519 de
Sanctuary embarquée. Le site héberge les flux à `/launcher-updates/stable/`
et les fichiers immuables à `/launcher-updates/releases/<version>/`. Les
réglages et instances du joueur sont dans le dossier de données utilisateur.

- Sparkle télécharge les mises à jour macOS en arrière-plan et vérifie la
  signature de l'archive avant extraction. Le bundle possède une signature
  ad hoc. La première ouverture peut demander une autorisation Gatekeeper.
  [Publication Sparkle](https://sparkle-project.org/documentation/publishing/)
- WinSparkle vérifie automatiquement les mises à jour et propose l'installation
  dans le launcher. Il vérifie la signature Sanctuary avant d'exécuter
  l'installateur NSIS, qui place chaque version dans un dossier propre. Windows
  peut afficher un avertissement SmartScreen lors du premier lancement.
  [WinSparkle](https://winsparkle.org/)
- L'AppImage Linux consulte un manifeste signé et vérifie la taille et le
  SHA-256 du nouvel AppImage avant de le remplacer à la fermeture. Le
  téléchargement se fait en arrière-plan. L'AppImage doit être exécutable
  et placé dans un dossier où le joueur peut écrire.

## Signer une version

Le poste de publication conserve la clé privée du canal stable. Seule sa
clé publique entre dans les builds, sur le serveur web et dans les sources.
Sur le poste de signature, préparer les fichiers avec OpenSSL 3.

```sh
python3 scripts/stage-launcher-release.py --version "${release_version}" \
  --macos "${launcher_macos_zip}" --linux "${launcher_linux_appimage}" \
  --windows "${launcher_windows_installer}" --sources "${launcher_sources_tar}" \
  --key "${launcher_signing_key}" --openssl "${openssl3}" --output "${release_stage}"
```

Le dossier `launcher-releases/<version>` contient les trois fichiers
de mise à jour et les sources. Le dossier `launcher-channels/<version>`
contient les ZIP d'installation, les manifestes et leurs empreintes.
Téléverser les deux dossiers et vérifier les sommes des fichiers sur le
serveur avant de basculer le lien `launcher` par renommage atomique.
Publier chaque correctif sous un numéro supérieur.

La recette du 26 novembre a confirmé le parcours Microsoft, la licence et
le profil Minecraft Java depuis un profil vierge sur macOS Apple Silicon.

## Publication 0.1.0 du 26 novembre 2026

- Windows 10 x64, macOS 26 Apple Silicon et Debian 13 x64 ont chacun passé
  les 25 suites CTest. Les trois exports démarrent avec un profil neuf.
- Sparkle a installé le bundle 0.1.0 depuis le flux HTTPS public sur une
  copie de recette en 0.0.9. Le bundle obtenu passe `codesign --verify --deep --strict`.
- L'AppImage 0.0.9 a vérifié le manifeste et remplacé son fichier à la
  fermeture. Le SHA-256 correspond à l'AppImage 0.1.0 publiée et le profil de
  test est conservé au redémarrage.
- Windows a démarré depuis l'installateur NSIS avec WinSparkle et les trois
  auxiliaires Java. WinSparkle accepte la signature de l'installateur et
  rejette une signature altérée. Un pilote de recette WinSparkle 0.0.9 a
  installé la 0.1.0 depuis le flux HTTPS public. Le raccourci pointe vers la
  0.1.0 après installation, avec l'ancien dossier et le profil conservés.
- Les flux publics correspondent aux fichiers préparés. Les trois ZIP
  téléchargés depuis la page publique correspondent aux SHA-256 annoncés.

## Publication 0.1.1 du 26 novembre 2026

Le launcher impose la palette claire et des fonds blancs sur Windows,
macOS et Linux. Les réglages d'apparence conservent les choix d'icônes.
Un profil avec `ApplicationTheme=dark` démarre avec la palette claire.

Les 25 suites CTest passent sur chaque plateforme. Le démarrage des trois
exports est vérifié avec un ancien réglage sombre. Sous Debian, les pixels
mesurés dans la fenêtre ouverte confirment le fond blanc dans la bordure,
la barre latérale et la zone centrale. Sous Windows, les contrôles
d'affichage utilisent les tests Qt hors écran et le journal du vrai launcher.

Les trois ZIP publics correspondent aux SHA-256 publiés. Sparkle et
l'AppImage ont installé la 0.1.1 à partir de la 0.1.0 via le canal HTTPS.
L'AppImage redémarre en clair avec le même profil. WinSparkle détecte
la mise à jour depuis la 0.1.0 et accepte la signature de l'installateur 0.1.1.

## Version 0.1.2

Windows chiffre la session Sanctuary avec DPAPI dans le profil du launcher.
macOS conserve son entrée dans le Trousseau. Linux utilise le coffre du
bureau lorsqu'il est actif. Le bouton de déconnexion révoque la session et
supprime la copie locale.

WinSparkle vérifie aussi au démarrage pendant l'intervalle entre deux
vérifications périodiques. Les réglages du launcher proposent un bouton de
vérification manuelle. Le raccourci Windows ouvre un exécutable fixe qui
lit la version active dans `current.ini`. NSIS remplace ce fichier de
manière atomique après avoir écrit la version. Un raccourci existant
directement épinglé sur `versions\0.1.0` peut être remplacé par celui du
menu Démarrer.

Sur le poste Windows de Koka, l'installateur signé a installé la 0.1.2.
Le raccourci Démarrer et l'épingle ont été contrôlés. Une recette WinSparkle
a enregistré une nouvelle vérification après relance dans l'intervalle
quotidien. Les tests Windows et macOS ont restauré un jeton de recette puis
vérifié son retrait après déconnexion.

La 0.1.2 est publiée le 26 novembre 2026. Les 25 suites passent sur les
trois plateformes. Debian a restauré puis effacé un jeton de recette dans
un coffre GNOME isolé. Les trois ZIP téléchargés correspondent aux SHA-256
publiés. macOS et Linux ont installé la version signée depuis la 0.1.1.

## Version 0.1.5

Sparkle vérifie le canal en arrière-plan dès l’ouverture sur macOS lorsque
les vérifications automatiques sont actives. L’intervalle par défaut passe
à une heure pendant que l’application reste ouverte. Sparkle installe les
versions vérifiées lors de la fermeture.

Les 25 suites CTest passent sur chaque plateforme. Le Mac de Koka a téléchargé
la 0.1.5 depuis le flux public et l’a installée via Sparkle. La fenêtre montre
la 0.1.5, le bundle installé passe la vérification de signature et l’heure
de vérification a avancé dès la relance.

## Version 0.1.7

Sous Windows, la carte et le wiki redimensionnent la fenêtre enfant WebView2
avec leur panneau Qt. La 0.1.6 la laissait à 0 × 0 dans le wiki. Sur le PC
Windows de Koka, la fenêtre enfant corrigée mesure 860 × 652 à l'ouverture
et 1 030 × 730 après agrandissement, comme son panneau. Les 25 tests CTest
Windows passent.

## Version 0.1.9

Jouer devient la page d’ouverture. Le bouton de connexion de la barre
latérale ouvre l’étape suivante et explique la procédure en infobulle.
Les pages Sanctuary et les outils du jeu ont chacun leur groupe.
Jouer reste grisé et le lancement attend la fin des étapes obligatoires.

Le build de développement a démarré sur macOS avec un profil vierge.
Les 25 tests CTest passent, dont le parcours de connexion en français
et en anglais avec des états de recette et le changement d’onglet au clavier.

## Version 0.2.0

Le bandeau des habitants est commun aux pages du launcher. Il consulte le
point d’accès authentifié `/api/launcher/v1/residents` toutes les 15 secondes.
Le site associe la whitelist du serveur à sa liste de connexions Minecraft.
Chaque visage est affiché en couleur en ligne et en niveaux de gris hors ligne.
Une réponse indisponible est signalée dans le bandeau.

Le test Qt vérifie les deux rendus et la suppression des visages à la
déconnexion du compte. Le jeton de session est réservé aux requêtes
Sanctuary. Les requêtes de visages restent publiques.

## Version 0.2.1

Le logo Sanctuary et le bandeau Habitants partagent la même ligne au-dessus
des pages et des réglages. Le test Qt vérifie l’alignement du logo, du libellé
et du premier visage.

## Version 0.2.2

La barre latérale regroupe Jouer, Actualités, Carte et Wiki. Le lien Jouer
utilise un style neutre et garde son état grisé pendant la configuration.
Le test Qt vérifie l’ordre des liens et le rendu du lien Jouer.

## Version 0.2.5

Le fond de Jouer dans la barre latérale parcourt les couleurs de
l’arc-en-ciel au survol, une fois la configuration terminée.
Des icônes monochromes accompagnent les entrées de la barre latérale.
Le test Qt vérifie le rendu des icônes, le défilement et le retour au fond
neutre.

Le bundle macOS désigne explicitement le fichier ICNS Sanctuary. L’export
compare ce fichier à la source officielle et vérifie l’absence du catalogue
d’icônes hérité de Prism.
