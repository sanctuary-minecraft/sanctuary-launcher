# Recette du launcher local — 8 novembre 2026

Branche de travail : `codex/launcher-local-onboarding`. Base Prism 11.1.1.
Lot associé aux tickets LAUNCH-02, LAUNCH-03 et LAUNCH-13, avec lecture publique
LAUNCH-04. Il ne livre aucune authentification ou activation serveur.

## Résultat

Une fenêtre native blanche héberge Accueil, Actualités et les pages du jeu.
Accueil montre les cinq étapes jusqu'à préparation, puis les masque au profit
de Jouer, Intendance, Gazette et Tableau d'affichage. Le code SGA est facultatif.
Les réglages globaux s'ouvrent dans la même fenêtre. Les fonctions des pages
Prism restent réutilisées, avec leurs sous-dialogues spécialisés.

Les comptes, l'accès et le badge historique sont des exemples locaux séparés
des comptes Prism. Le pack est réellement importé depuis le site et vérifié.
Le bouton Jouer local affiche un résultat de parcours sans démarrer Minecraft.
Les contenus communautaires restent privés : les routes demandent la session
Discord du site, non raccordée dans le mode local.

## Chaîne Sanctuary Update vérifiée

Le launcher consomme les publications existantes ; ce travail ne déploie ni
pack, ni site, ni serveur. Le canal stable n'est pas modifié.

| Élément | Valeur observée |
| --- | --- |
| ZIP officiel | `https://sanctuary-minecraft.net/downloads/sanctuary-prism.zip` |
| SHA-256 ZIP | `745aa431bc7666e0004a61546f6ce6c717d42ba89a100133966be828912b42d1` |
| Canal stable | `https://git.botsu.net/koka/sanctuary-beta/raw/branch/packwiz/pack.toml` |
| Cible immuable | `https://sanctuary-minecraft.net/distribution/packwiz/channels/d3c2d339c2bdd4aeef7f32d54ef445c40721cbc7/pack.toml` |
| Version publiée/installée | `beta.373` |
| SHA-256 manifeste | `e3f3eaed87af312e927f265150d9ab0a3b6d478460a80a08ef9d3226e982ec36` |
| SHA-256 index attendu | `f0e6ecf894ec94fe1eaec3cc45a625ad42decc0ff5623c68e7b5412ba9b0d95a` |
| Minecraft / Fabric / Java du jeu | `26.3` / `0.19.5` / `25` |
| SHA-256 Sanctuary client | `8631eda7174f4f2ff5f2a65a9f32d64a8eb2cc6b75a515e817d4db9005991049` |

Les empreintes des deux outils packwiz sont contrôlées avant exécution :
bootstrap `a8fbb24dc604278e97f4688e82d3d91a318b98efc08d5dbfcbcbcab6443d116c`,
installateur Sanctuary `9e2912fdd05e5c23721acd58cd5c61bbf13061e453627e8f4df9a4c53f7aa875`.
Les fichiers gérés sont ensuite vérifiés par empreinte et côté client.
Aucun raw GitHub privé, secret ou jeton de joueur n'est nécessaire.

## Essais réellement effectués

- Construction Release C++/Qt sur macOS 26.5.1 arm64, Qt 6.12.0 ; **24 suites CTest réussies**.
- Import depuis le bouton Installer de la fenêtre native, sans assistant Prism
  séparé, dans `build/local-launcher/instances/Sanctuary Beta`.
- Synchronisation packwiz réelle avec Java 17, mode client sans interface :
  onze JAR client dont Sanctuary beta.373 ; BlueMap serveur non installé.
- Deuxième passage par le bouton Mettre à jour : toujours une seule instance,
  mêmes fichiers de test dans config/captures/saves, mod facultatif EntityCulling
  toujours désactivé après synchronisation. Les sentinelles ont ensuite été
  retirées et ce mod de test réactivé. Aucun monde réel n'a été ouvert ou modifié.
- Navigation native dans les pages mods, ressources, shaders et réglages
  globaux : contenu hébergé dans la fenêtre Sanctuary, sans `InstanceWindow`.
- Parcours local Discord/Microsoft/parrainage : les étapes disparaissent une
  fois l'accès simulé accordé et le pack vérifié ; les trois rubriques apparaissent.
  Le bouton Jouer confirme le parcours local sans lancer le jeu.
- Les trois flux privés répondent par une demande de session Discord. Aucune
  donnée communautaire n'est inventée ou rendue publique.
- Actualités : beta.373 et les publications précédentes sont lues réellement
  sur `/actualites`, dans la même fenêtre, sans page de connexion embarquée.
- Icône utilisateur et ICNS/ICO contrôlés ; la police SGA est bien chargée par Qt.

Les tests contrôlent aussi les états attente/refus/révocation, le blocage de
Jouer sans pack réel vérifié, l'absence de prérequis SGA, la restauration des
états locaux, les formulaires invalides et les chemins/empreintes incorrects.

## Rendus des widgets

Ces images sont produites par Qt pendant les tests, à 1280 × 800 ; elles ne
sont pas des captures d'un jeu ou d'une session authentifiée, ni la capture
complète du conteneur latéral natif.

[Début](accueil-blanc-start-qt-fr.png) ·
[Attente](accueil-blanc-pending-qt-fr.png) ·
[Prêt local](accueil-blanc-ready-qt-fr.png)

Après la dernière reconstruction, les 24 suites passent et le processus
avec le dossier de développement démarre. Le rattachement automatique de
l'accessibilité macOS a ensuite expiré ; les observations natives ci-dessus
ont été effectuées avant cette dernière relance. Les ajustements finaux de
navigation, d'enregistrement des pages visitées et de séparation date/version
sont couverts par les tests, sans nouvelle capture de bureau finale.

## Limites ouvertes

Microsoft/Mojang, Discord et le dossier d'admission ne sont pas raccordés.
Le code SGA affiché est fictif. Le mod conserve son parcours manuel actuel :
l'activation automatique demandée n'est pas livrée par cette interface.
L'authentification réelle est nécessaire pour les contenus communautaires.
Le lecteur d'actualités dépend du HTML du site ; contrat structuré et cache
persistant restent à préparer.

Ni lancement du jeu, ni serveur/personnage, ni shader, ni Sound Physics,
ni Vulkan n'ont été testés dans ce lot. L'absence de lancement n'est pas
un mode de jeu hors licence. Les opérations internes de toutes les pages
Prism, l'intégration de la console du futur lancement, la conservation après
panne brutale et la réparation de chaque erreur réseau restent à éprouver.
Le contrôle de conformité doit encore couvrir les composants modifiés.

Le bundle local dépend de Homebrew et n'est pas signé/notarisé. Aucune
validation Windows/Linux/macOS antérieur, aucun binaire distribué, aucune
modification de Prism personnel et aucun déploiement Sanctuary Update.
