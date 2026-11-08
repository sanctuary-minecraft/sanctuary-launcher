# LAUNCH-16 — Connexion Discord, accès et contenus privés

Branche : `codex/launcher-discord-session`, dans sanctuary-launcher et sanctuary-web.

Connecter le compte Discord existant depuis le navigateur officiel, retrouver
le dossier Sanctuary et lire les publications Intendance/Gazette/Tableau avec
une session limitée à la lecture. Préserver les admissions, mondes et comptes.

Microsoft et Jouer restent en attente. Aucun état de démonstration ne doit
remplacer un état authentifié, et aucun code fictif n'est présenté comme réel.
La déconnexion révoque la session et efface les aperçus privés.

Recette : [contrat et vérifications](../discord-connexion.md). Les tests PHP
couvrent consentement, expiration/rejeu, révocation et limites des droits ; les
tests Qt couvrent le navigateur de confiance, les requêtes authentifiées et
la séparation entre données réelles et simulation.

Validation utilisateur le **8 novembre 2026** : Koka confirme la connexion
avec son vrai compte Discord et l'apparition des publications dans le launcher.
Le résultat demandé est validé ; Microsoft et le jeu restent hors de cette recette.
