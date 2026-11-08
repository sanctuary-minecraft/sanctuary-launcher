# Accueil minimal et Bien démarrer — 8 novembre 2026

Ticket LAUNCH-17, branche `codex/launcher-minimal-posts`.
Le launcher natif devient la référence ; aucune nouvelle maquette.

## Résultat

Navigation : **Bien démarrer**, **Accueil**, **Actualités**, puis les outils
Prism. Bien démarrer conserve les cinq étapes après la connexion Discord,
dans la même fenêtre. Le bouton Microsoft reste visible et désactivé avec
son attente Microsoft/Mojang expliquée. Les deux pages partagent une session
Discord et une installation officielle ; aucun compte ou accès simulé n’est
utilisé pour l’accueil réel.

L’accueil retire les titres de rubrique, cadres et barres d’actions des trois
flux. Intendance reste en haut, Gazette à gauche et Tableau à droite.
Chaque résumé montre le visage du skin Minecraft de l’auteur et son titre.
Le clic déplie le texte complet et la photo de Gazette, une publication ouverte
par flux, avec défilement interne. Le footer et Jouer restent fixes. Un lien
discret ouvre la discussion sur le site. La mise à jour périodique conserve
l’article ouvert si son identifiant existe encore ; les textes sont simples
et les titres échappés, sans HTML de publication exécuté.

Les visages 32 × 32 viennent du même fournisseur public que le site, sans
Authorization ni cookie Sanctuary. Cache mémoire borné, initiale de repli.
Les photos passent par l’API Sanctuary authentifiée et sont chargées seulement
à l’ouverture. JPEG limité à 49 152 octets et 960 × 540 avant décodage, aucune
redirection suivie. Une déconnexion efface les contenus et invalide les
réponses anciennes. Aucun contenu communautaire privé n’est ajouté à Git.

## Vérifications

- Construction native C++20/Qt6 sur macOS Apple Silicon et lancement du profil
  isolé `build/local-launcher` avec le script Run canonique.
- 25 suites CTest passent. Couverture ciblée : tutoriel conservé après Discord,
  Microsoft/Jouer désactivés, premier onglet Bien démarrer, footer visible aux
  trois tailles 1000 × 680, 1120 × 760 et 1280 × 800, lecture dépliable,
  conservation à l’actualisation, accordéon et défilement d’un texte long,
  titre échappé, corps texte simple et requête photo vers le seul site prévu.
- Les visages de deux auteurs sont décodés depuis un fournisseur réseau simulé,
  rendus en 32 × 32, demandés sans Authorization ni cookie, puis réutilisés en
  mémoire après modification du titre. Le fournisseur public réel répond aussi
  avec un PNG 64 × 64 au contrôle du 8 novembre.
- Rendus réels des widgets Qt dans `build/native/captures-minimal/`, données de
  test identifiées comme telles. Le tutoriel reste vérifié en FR/EN. Les rendus
  de test antérieurs sans réseau montrent volontairement des initiales.
- Vérification native visuelle : Bien démarrer précède Accueil et Actualités,
  navigation dans la même fenêtre et footer toujours visible. Le profil local
  affichait une session Discord déconnectée à ce contrôle : aucun affichage
  de vraie photo ni de visage privé n’est revendiqué sur ce lancement.

## Site et limites

Complément Web `09299e8`, branche `codex/launcher-inline-posts` : indicateur
`has_photo` et endpoint de lecture JPEG privé. 32 tests PHP / 259 assertions
passent avec accès communauté, révocation et exclusion des publications
cachées, hors serveur, non-articles, réponses ou sans photo. Pint passe.
Deux fichiers livrés sur CT 320 avec cache de routes ; 359 autres sources
suivies identiques et 162 changements préexistants préservés. Sauvegarde privée :
`/root/sanctuary-beta-maintenance/launcher-inline-photos-20261108T210210Z`.
Contrôle public : photo anonyme 401, actualités 200.

Le raccordement Discord et les vraies publications ont été confirmés par Koka
avant ce lot. Les nouveaux visages et photos privés restent à confirmer avec
sa session reconnectée. Microsoft, Minecraft, le personnage et la connexion
au serveur restent en attente. Aucun monde, serveur de jeu, pack, canal ou
installation Prism personnelle modifié.

## Ajustement LAUNCH-18 — date et heure, Intendance sans visage

Branche `codex/launcher-home-clock`. L’accueil quotidien affiche désormais
la date longue et l’heure locales en titre (FR/EN, format 24 heures à la
minute). L’affichage se rafraîchit automatiquement et à l’ouverture de l’onglet ;
Bien démarrer conserve son titre d’étape.

L’Intendance ne crée ni visage, ni initiale, ni espace réservé à un avatar et
n’appelle plus le fournisseur de skins. Son contenu déplié est aligné à gauche.
Les visages de Gazette et du Tableau sont conservés.

Vérification : script Run `--verify`, 25 suites CTest réussies, profil isolé
relancé. Tests adaptés pour le header visible, l’absence de visage/requête
Intendance et le rendu d’un visage dans le Tableau. Rendu ciblé Qt contrôlé
aux trois tailles prévues, capture à 1120 × 760 dans
`build/native/captures-clock/minimal-home-fr.png` : titre date/heure, trois flux
et footer visible. Ces publications sont des données de test ; aucune nouvelle
lecture de contenu privé, modification Web, de pack ou essai Minecraft.

## Ajustement LAUNCH-19 — titre Accueil et cadres gris clair

Branche `codex/launcher-home-panels`. Le titre Accueil (Home) reprend les
23 px et la graisse 600 du titre des autres pages natives. La date et l’heure
passent en sous-titre discret, avec leur actualisation automatique.

Les trois flux retrouvent un cadre fin #d0d7de, des coins de 4 px et le fond
gris très léger #f6f8fa des tableaux natifs. Les widgets internes restent
transparents sur ce fond. La présentation Sanctuary reste à l’intérieur :
titres dépliables, texte et photos, visages dans Gazette et Tableau seulement,
sans titre de rubrique ou nouvelle barre d’actions.

Script Run `--verify` réussi : 25 suites CTest passent et profil local isolé
relancé. Test existant adapté pour Accueil/date en deux lignes. Rendu Qt
contrôlé aux trois tailles prévues ; capture 1120 × 760 dans
`build/native/captures-panels/minimal-home-fr.png` : titre, date, cadres,
positions des flux et footer visible. Publications de test, pas de nouvelle
validation de photos privées ni d’essai Minecraft. Aucun changement Web,
pack, monde, serveur ou instance Prism personnelle.
