# Toutes les options de Prism dans Sanctuary Launcher

Contrat associé au [cahier des charges 0.4](cahier-des-charges.md), demandé par
Koka le **8 novembre 2026**. Sanctuary Launcher conserve les fonctions de sa
base **Prism 11.1.1**. L'accueil blanc avec les étapes 1 à 5 simplifie l'arrivée
sur Sanctuary ; il ne remplace pas la gestion complète de Prism.

## Accès et navigation

Le bouton **Tous les outils / All tools** reste accessible depuis l'accueil.
Lorsque l'instance Sanctuary existe, il ouvre sa gestion complète. Sans instance,
les réglages globaux et la gestion des instances restent accessibles ; les pages
qui nécessitent des fichiers suivent leurs prérequis habituels. Un accès aux
autres instances et un retour visible à l'accueil Sanctuary sont conservés.

Les raccourcis Mods, Shaders et Réglages, lorsqu'affichés, ouvrent ces mêmes
pages. Ils ne constituent pas une liste exhaustive des possibilités et ne
doivent pas lancer un second gestionnaire moins complet.

## Pages de l'instance à conserver

| Outil | Fonctions à conserver selon les possibilités de Prism |
| --- | --- |
| Mods | Liste, ajout, activation/désactivation, mises à jour et ouverture du dossier |
| Packs de ressources | Gestion des packs, ajouts et accès aux fichiers |
| Packs de shaders | Gestion des packs et ajouts compatibles |
| Captures d'écran | Consultation, gestion locale et ouverture du dossier |
| Mondes | Gestion des sauvegardes locales et outils associés, dont datapacks lorsqu'applicables |
| Serveurs | Liste des serveurs de l'instance, ajout, modification, état et connexion |
| Versions et composants | Minecraft, chargeur et composants de l'instance, avec indication des écarts au profil officiel |
| Réglages de l'instance | Java, mémoire, paramètres de lancement et autres réglages proposés par la base |
| Console et journaux | Console de lancement, logs et autres journaux, diagnostic et export avec masquage des données sensibles |
| Autres pages natives | Gestion du modpack, notes, datapacks et pages spécialisées prévues par Prism |

Les règles de visibilité amont continuent de dépendre de la version et du type
d'instance. Conserver une page historique de textures, coremods ou NilLoader
ne signifie pas que ces contenus conviennent à la version actuelle de Sanctuary.
Les contenus client ajoutés doivent être vérifiés pour la version exacte du jeu.

## Réglages globaux et autres instances

Conserver les réglages généraux du launcher, de langue et d'apparence, Minecraft,
Java et mémoire, comptes, API, outils externes et proxy/réseau. Préserver la
création, la copie, l'import/export et les autres actions de gestion des instances
de Prism, sans transformer le launcher en une interface à trois boutons.

La candidature Sanctuary régit l'accès à son serveur. Elle ne doit pas masquer
les outils locaux, supprimer une instance personnelle ou interdire les usages
normaux des autres instances. Les licences Minecraft et les contrôles propres
à chaque serveur restent nécessaires pour jouer.

Les services externes gardent leurs propres conditions et identifiants : les
clés CurseForge et Imgur amont ne sont pas réutilisées. Conserver leurs outils
ne vaut pas configuration ou validation de ces intégrations. Le raccordement
Microsoft Sanctuary et l'accès Minecraft restent eux aussi à terminer.

## Profil Sanctuary et données personnelles

Une seule instance officielle reste suivie par Sanctuary Update, sans limiter
le nombre d'instances personnelles. Les modifications avancées des composants
requis doivent rendre visible l'écart au profil officiel ; une copie est proposée
pour les expériences incompatibles avec ce profil. Une synchronisation préserve
les ajouts et préférences personnels et vérifie les fichiers gérés.

Les mondes de Prism sont les sauvegardes locales. Ils ne donnent aucun accès
d'administration au monde du serveur Sanctuary. Les opérations sur les mondes
existants continuent de suivre les contrats de migration applicables.

## État réel et vérifications à effectuer

La lecture des sources du fork confirme la présence des pages dans
[InstancePageProvider.h](../launcher/InstancePageProvider.h), des réglages
globaux dans [Application.cpp](../launcher/Application.cpp) et de l'accès aux
outils classiques depuis le premier accueil dans
[MainWindow.cpp](../launcher/ui/MainWindow.cpp). Ces sources sont consultées
sans modification pour ce cadrage.

La nouvelle maquette et le bouton Tous les outils restent des objectifs à
intégrer. La présence des classes ne prouve pas une recette fonctionnelle
nouvelle ni la compatibilité d'un mod ou d'un service externe. LAUNCH-13 doit
vérifier les accès, la navigation, les opérations utiles des pages, le retour
à l'accueil et la conservation des données, y compris pendant l'attente
d'admission. Cette révision ne livre que de la documentation et des images.
