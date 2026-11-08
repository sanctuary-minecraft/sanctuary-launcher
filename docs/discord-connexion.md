# Connexion Discord de Sanctuary Launcher

Lot du 8 novembre 2026, ticket LAUNCH-16, branches `codex/launcher-discord-session`
dans le launcher et le site. Le site existant reste le serveur d'identité et
de contenu ; aucun service Discord parallèle ni nouveau compte à mot de passe.

## Parcours

Connecter Discord demande une association temporaire au site, puis ouvre le
navigateur sur `/launcher/connect`. Le joueur se connecte avec le Discord
habituel, vérifie le code d'association affiché des deux côtés et confirme
**Connecter mon launcher**. Le launcher attend la décision et récupère sa
session automatiquement, sans copier de cookie ou de jeton du navigateur.
Ce code d'association n'est pas le code SGA de personnage.

Le transport suit le principe du [Device Authorization Grant, RFC 8628](https://www.rfc-editor.org/rfc/rfc8628) : secret de requête aléatoire gardé
par le client, code de vérification à durée courte, accord dans le navigateur,
polling borné. L'autorisation expire après dix minutes, le polling est limité
à une requête toutes les cinq secondes et une association n'est consommée
qu'une fois sous verrou. La connexion Discord existante utilise toujours
[Laravel Socialite](https://laravel.com/docs/13.x/socialite), sans nouveau Client ID
Discord ni secret dans le binaire.

## Session et données

La session du launcher dure sept jours et autorise uniquement la lecture du
profil, des flux communautaires et de leurs photos privées. Les secrets sont aléatoires (256 bits) ;
le site ne conserve que leurs empreintes comme clés du cache existant, avec
expiration. Aucune migration SQL ni modification de la whitelist n'est requise.
Le profil est relu au démarrage, sur demande et chaque minute. Un échec de
vérification masque les rubriques ; une session refusée demande une reconnexion.

Le jeton est envoyé uniquement dans Authorization vers les endpoints HTTPS
fixes du site. Les redirections authentifiées ne sont pas suivies. Sur macOS,
le Trousseau conserve le jeton sous un identifiant propre au dossier de données
du launcher ; jamais dans sanctuary-launcher.cfg. Ailleurs la session reste
en mémoire dans ce lot. Déconnecter révoque la session côté site, retire le
jeton local et efface immédiatement les aperçus, même si une ancienne réponse
arrive ensuite. Une connexion hors réseau ne vaut pas révocation confirmée
côté serveur ; la suppression locale et l'expiration bornent sa durée.

| Endpoint | Contrat |
| --- | --- |
| POST `/api/launcher/v1/device` | Client fixe sanctuary-launcher ; association dix minutes |
| GET/POST `/launcher/connect` | Login Discord existant, accord/refus explicite avec CSRF |
| POST `/api/launcher/v1/token` | Secret device en corps JSON ; pending, slow_down, refus, expiration ou session |
| GET `/api/launcher/v1/profile` | Nom d'affichage, pseudo Minecraft du dossier et état d'accès ; aucun e-mail ou code secret |
| GET `/api/launcher/v1/feed?kind=…` | article, bulletin ou notice ; trois entrées récentes, une pour Intendance ; serveur configuré et publications visibles uniquement |
| GET `/api/launcher/v1/posts/{id}/photo` | Photo JPEG d’un article visible du serveur configuré, session requise, aucun cache disque |
| POST `/api/launcher/v1/revoke` | Révocation de cette session |

Les réponses privées portent `Cache-Control: private, no-store`. Les flux
réutilisent CommunityService et ses règles de visibilité. Une session launcher
n'authentifie pas les routes Web d'écriture ni l'administration. Les aperçus
Qt traitent les textes comme du texte simple, sans exécuter de HTML ou script.

Le statut d'accès provient du dossier et de la whitelist du site : absence,
attente, acceptation en synchronisation, accès confirmé, refus, retrait ou
condition Discord manquante. La communauté est affichée pour un accès confirmé,
avec **Microsoft en attente** dans Bien démarrer et Jouer désactivé. Aucune licence
Minecraft, création de personnage ou liaison Microsoft n'est déduite de Discord.
Le marqueur historique réel reste à rattacher à une source confirmée ; le badge
et le code SGA de démonstration ne sont pas utilisés pour ce parcours.

## Vérifications et livraison

- **22 tests PHP / 199 assertions** passent : consentement, refus, expiration,
  rejet d'un ancien code, révocation, identité déliée, accès existant inchangé,
  absence de droits Web d'écriture, publications cachées/autre serveur exclues,
  et reprise du retour Discord si le cookie de session initial a été perdu.
- **25 suites CTest** passent, dont le parcours réseau natif simulé : ouverture
  du seul site attendu, passage au jeton, lecture authentifiée et déconnexion.
  Les tests UI prouvent qu'un scénario local ne donne pas de droits réels.
- Site CT 320 : sept fichiers livrés par empreintes et cache de routes régénéré.
  **357 autres fichiers suivis identiques**, 158 changements préexistants préservés.
  Sauvegarde privée : `/root/sanctuary-beta-maintenance/launcher-discord-20261108T195233Z`.
- Contrôles publics : création d'association, pending/slow_down, refus anonyme
  sur profil/flux, retour vers Discord ; téléchargement et actualités beta.373
  conservés. Le launcher local est reconstruit et relancé.

Le **8 novembre 2026**, Koka confirme : « Connexion validée, les publications
apparaissent ». Ce retour valide le parcours avec son vrai compte Discord et
l'affichage des vrais contenus dans le launcher. Il s'agit d'une validation
utilisateur, distincte des tests automatiques ci-dessus ; aucun identifiant,
jeton ou contenu privé n'est conservé dans cette recette.

La reprise après redémarrage et l'expiration réelle de sept jours ne sont pas
validées par ce seul retour. Microsoft, le lancement Minecraft et le personnage
restent en attente. Aucun jeu, monde, serveur Minecraft ou canal packwiz modifié.


## Lecture dépliable — complément LAUNCH-17

Les résumés montrent le titre et le visage du skin de chaque auteur. Le clic
ouvre le contenu dans le flux, avec défilement et footer fixe. `has_photo`
annonce une photo de Gazette ; le client construit le chemin fixe de l’API
et la demande au dépliage. Aucun URL d’image fourni par un post n’est suivi.
Les photos restent privées, bornées, en mémoire ; les visages publics utilisent
le même fournisseur que le site, sans jeton Sanctuary. Bien démarrer reste
le premier onglet et conserve les cinq étapes après Discord.
Voir la [recette complémentaire](implementation/accueil-minimal-20261108.md).
