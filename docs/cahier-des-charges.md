# Cahier des charges de Sanctuary Launcher

Version documentaire 0.2, actualisée le **8 novembre 2026**. Sanctuary Launcher
doit installer le pack Sanctuary, présenter ses actualités et rejoindre son
serveur depuis une application dédiée fondée sur Prism Launcher.

La [maquette d'accueil](design/README.md) est validée par Koka. Le développement
est désormais demandé. Un premier lot natif fondé sur Prism compile localement
avec un accueil Qt FR/EN ; son [état vérifié](developpement.md) distingue ce qui
fonctionne des objectifs restants. Ce cahier décrit la cible complète, sans
valider une connexion Minecraft ni une distribution pour les joueurs.

## 1. Objectif et périmètre

Le joueur télécharge Sanctuary Launcher, choisit son dossier, obtient Java et
le pack compatibles, se connecte avec son compte Microsoft puis clique sur
**Jouer** pour rejoindre Sanctuary. Aux lancements suivants, il retrouve la
même installation, ses préférences et une mise à jour contrôlée du pack.

Le parcours principal ne demande pas de connaître Prism, Java, Fabric ou
packwiz. Les outils avancés utiles de Prism restent accessibles dans les
réglages et le diagnostic. La V1 ne crée pas de nouveau backend, de second
compte Sanctuary ou de service Discord autonome.

| Choix retenu | Conséquence |
| --- | --- |
| Fork de Prism avec identité Sanctuary | Conserver les mécanismes de lancement utiles de Prism et cibler les adaptations |
| Accueil conforme à la maquette validée | Actualités et bouton Jouer sont les priorités de l'écran |
| GitHub comme référence | Sources dans l'organisation `sanctuary-minecraft`, Gitea archivé |
| Sanctuary Update à la demande | Un push seul ne déclenche pas de déploiement |
| Shaders et Sound Physics activés initialement | Initialiser le profil dédié une fois, puis respecter les choix du joueur |
| Microsoft en attente | Le cadrage et les écrans peuvent avancer ; l'authentification réelle reste à raccorder |

## 2. Existant et base Prism

Le pack actuel **beta.372** utilise Minecraft **26.3**, Fabric **0.19.5** et
Java **25**. Ces valeurs seront lues dans les métadonnées de publication et
ne doivent pas devenir des constantes dispersées dans le launcher.

| Élément | État au 8 novembre 2026 |
| --- | --- |
| Pack et instance Prism du site | Installation et mises à jour packwiz existantes |
| Bootstrap Sanctuary | Sources, logo et empreintes conservés dans `existing/` |
| Favori du serveur | Ajout initial géré par le mod ; ce n'est pas une connexion directe |
| Sound Physics Remastered | Option client sélectionnée par défaut ; les métadonnées actuelles décrivent son moteur comme désactivé par défaut |
| Shaders Sanctuary | Présents dans le mod ; le profil de première installation reste à adapter |
| Actualités FR/EN | Existantes dans le site ; aucun flux dédié au launcher exposé |
| Fork Sanctuary | Base Prism et accueil Qt compilés localement ; recette complète à poursuivre |
| Application Microsoft Sanctuary | Créée le 8 novembre, flux clients publics activés ; accès Minecraft et intégration au fork en attente |
| Mise à jour de l'application launcher | Chaîne à préparer, distincte de celle du pack |

La base retenue est **Prism 11.1.1**, dernière release stable vérifiée sur
GitHub le 8 novembre, commit `7d2d3c1ec6fcf8255ebacd02d9f792688575dfb4`.
Son [CMake](https://github.com/PrismLauncher/PrismLauncher/blob/11.1.1/CMakeLists.txt)
utilise C++20 et Qt 6. Son import et une compilation macOS locale sont réalisés ;
la compatibilité avec le pack exact reste à éprouver par installation et jeu.

`sanctuary-launcher` est la destination du projet. L'import conserve l'historique
Prism et celui de ce dépôt, ses documents, `existing/` et les archives Release.
Le tag `upstream/prism/11.1.1` fixe la base et un remote `upstream` suit Prism ;
les mises à niveau amont seront revues et testées avant intégration. Aucune
réécriture Electron ou Tauri n'est prévue.

La [politique des forks Prism](https://github.com/PrismLauncher/PrismLauncher#forkingredistributingcustom-builds-policy)
prévoit une identité distincte et le remplacement ou la désactivation des
identifiants d'API amont. Le [code est sous GPL-3.0-only](https://github.com/PrismLauncher/PrismLauncher/blob/11.1.1/LICENSE) :
la distribution des binaires devra prévoir les sources correspondantes pour
leurs destinataires selon la licence. Les dépôts restent privés à ce stade ;
les modalités de fourniture des sources seront définies avant distribution,
sans changer ici leur visibilité. Conserver les attributions et licences.

## 3. Accueil et parcours Jouer

**UI-01.** Reprendre le ciel Minecraft, les panneaux sombres transparents, les
traits fins, le texte clair, le mot-symbole et les boutons gris de la maquette.
Le paysage reste décoratif et ne gêne ni la lecture ni les commandes.

**UI-02.** Afficher le compte, la version réellement installée du pack et son
état de mise à jour. Le statut serveur porte une date d'observation ; une
information absente ou périmée donne un état inconnu. Les mentions de version,
de disponibilité et de compte de la maquette ne sont jamais des textes fixes.

**UI-03.** Présenter trois actualités avec version, date, titre, résumé et lien
de lecture. Wiki, Communauté et Site Sanctuary ouvrent le navigateur. La V1
n'embarque pas l'ensemble du site dans une nouvelle interface.

**UI-04.** Garder l'essentiel lisible à 1280 × 800 et à la taille de référence,
y compris avec l'échelle système et les textes anglais. Prévoir navigation
clavier, focus visible et commandes compréhensibles sans dépendre d'une image.

| État | Comportement principal attendu |
| --- | --- |
| Aucune installation | Installer propose un dossier et prépare Java, Fabric et le pack |
| Téléchargement ou mise à jour | Progression, interruption propre et reprise ; lancement indisponible avant vérification des fichiers requis |
| Pack prêt, compte non connecté | Jouer ouvre Microsoft puis reprend le parcours de lancement |
| Compte connecté et pack prêt | Vérification de session et lancement vers le serveur Sanctuary |
| Jeu en cours | État En cours de jeu ; pas de second lancement sur le même dossier |
| Échec | Cause utile, Réessayer et accès au diagnostic, sans boucle de relancement |

## 4. Installation et protection des données

**INST-01.** Réutiliser Prism pour Java, les composants du jeu et le lancement,
et packwiz pour le pack. Une installation vide doit aboutir sans import manuel
d'un ZIP dans Prism. Vérifier espace, droits, plateforme, versions et empreintes.
Les chemins avec espaces ou caractères accentués doivent fonctionner.

**INST-02.** Reprendre ou retenter les téléchargements interrompus. Un fichier
manquant ou incorrect ne permet pas d'annoncer « Pack à jour ». Un verrou
empêche lancement, réparation et mise à jour concurrents du même dossier.

**INST-03.** Gérer une seule installation Sanctuary par profil du launcher.
Préserver sauvegardes, captures, favoris et réglages personnels. Réparer
uniquement les fichiers gérés identifiés comme incorrects, sans suppression
globale du dossier et sans nouveau profil à chaque mise à jour.

**INST-04.** L'installation Prism existante reste intacte par défaut. Sa
reprise éventuelle demande un choix explicite et une sauvegarde ; elle ne
copie pas les jetons de compte et n'autorise pas deux launchers écrivant
simultanément sur les mêmes fichiers. La méthode de reprise reste à valider.

Les parcours actuels sont documentés dans
[beta-download.md](../existing/docs/beta-download.md) et
[default-server-beta295.md](../existing/docs/default-server-beta295.md).

## 5. Microsoft et connexion au serveur

**AUTH-01.** Utiliser une application **Sanctuary Launcher** et son propre
Client ID autorisé pour Minecraft. L'[enregistrement Entra](https://learn.microsoft.com/en-us/entra/identity-platform/quickstart-register-app)
est réalisé : voir les [identifiants et paramètres enregistrés](microsoft.md).
La [demande AppID Minecraft](https://aka.ms/mce-reviewappid) est envoyée le
8 novembre 2026 ; sa réception est confirmée et la décision reste en attente.
Le compte d'administration est rattaché à son nouvel annuaire Azure ;
la création d'un compte dédié n'est pas un préalable imposé.

**AUTH-02.** Reprendre le flux adapté de la base Prism retenue. La saisie des
identifiants se fait chez Microsoft, jamais sur le site Sanctuary. Aucun
secret client n'est embarqué dans l'application de bureau. Les jetons restent
sur l'appareil et sont exclus des requêtes au site, des journaux et diagnostics.
Le stockage hérité de Prism devra être examiné avant distribution ; une
protection OS des secrets est à résoudre pour les plateformes retenues, sans
prétendre qu'elle est déjà fournie par ce cadrage.

**AUTH-03.** Prévoir des messages FR/EN utiles pour connexion annulée, session
expirée, licence absente, profil Xbox incomplet et refus des services. Renouveler
une session selon le mécanisme Microsoft ; redemander la connexion lorsqu'elle
ne peut pas être rétablie. La validation de propriété du jeu reste obligatoire.

**JOIN-01.** Après authentification et contrôle du pack, rejoindre l'adresse
Sanctuary sans étape volontaire dans le menu principal. Prism
[prévoit une cible serveur au lancement](https://prismlauncher.org/wiki/getting-started/command-line-interface/) ;
son comportement réel avec Minecraft 26.3 doit être validé. L'adresse et le
port proviennent d'une configuration vérifiée, pas de constantes dispersées.
La cible actuelle documentée est `beta.sanctuary-minecraft.net` ; sa résolution
DNS/SRV et le port effectivement publié feront partie de la recette.

**JOIN-02.** Le compte Microsoft, le compte Discord du site et l'admission au
serveur restent distincts. Une authentification réussie ne crée pas de droit
d'accès Sanctuary. Respecter le refus du serveur et orienter vers le parcours
communautaire existant ; ne pas consommer un vrai code d'admission pour un test.

**JOIN-03.** Une indisponibilité ou un refus donne une cause et une possibilité
de reprise, sans reconnexion en boucle. Les états simulés des futures maquettes
ne valident pas une authentification réelle ou un essai multijoueur.

## 6. Réglages initiaux

**CFG-01.** Pour une nouvelle installation compatible, activer les shaders
Sanctuary et le moteur Sound Physics avec les dépendances exactes du pack.
La présence du JAR Sound Physics ne vaut pas activation ; le comportement de
première installation de beta.372 doit donc être adapté et vérifié.

**CFG-02.** Initialiser les seules valeurs nécessaires une fois, avec marqueur
de version et sauvegarde des fichiers existants touchés. Conserver ensuite
les choix du joueur, y compris la désactivation d'un effet. Ne pas imposer un
`options.txt`, `servers.dat` ou fichier personnel à chaque mise à jour packwiz.

**CFG-03.** Exposer mémoire, qualité graphique, shaders et son avec des
libellés compréhensibles. Mesurer les recommandations sur les machines cibles,
sans annoncer un réglage universel optimal. Indiquer un redémarrage nécessaire
et éviter les écritures concurrentes aux réglages d'un jeu actif.

**CFG-04.** Les validations graphiques du jeu ciblent **Vulkan uniquement**.
Expliquer une incompatibilité matérielle ; aucune prise en charge OpenGL du
jeu ne découle des anciens essais ou des dépendances Qt du launcher. La
conformité visuelle de l'application reste un contrôle distinct.

## 7. Actualités et données publiques

**WEB-01.** Lire un petit flux public FR/EN dérivé des actualités déjà publiées
par Sanctuary Web. Éviter une seconde rédaction des notes ou une lecture de
données privées GitHub. Les liens renvoient aux pages HTML existantes.

Le contrat proposé est une route versionnée de type `/launcher/v1/home.json`.
**Cette route n'existe pas encore** ; son nom et ses champs seront validés
dans le ticket du site avant le raccordement.

| Donnée proposée | Règle |
| --- | --- |
| Version du format et date | Détecter un format incompatible et une publication périmée |
| Langue | FR/EN avec repli explicite si une traduction manque |
| Dernier pack publié | Version et plateforme réellement activées, pas une préparation |
| Actualités | Identifiant stable, version, date, titre, résumé et URL de lecture |
| Liens publics | Site, wiki, communauté et distribution HTTPS vérifiés |
| Statut serveur éventuel | État et date ; ne pas confondre disponibilité du site et du jeu |

**WEB-02.** Le flux ne contient ni comptes, ni secrets, ni commandes. Rendre
les textes sans exécution de script ou HTML arbitraire ; n'ouvrir que les
destinations HTTPS attendues. Garder le dernier flux valide avec sa date.
Une panne du site affiche le cache ou son indisponibilité sans bloquer
inutilement un pack prêt et une session valide. Les erreurs nécessaires au
téléchargement ou au jeu gardent leur propre traitement.

## 8. Versions et Sanctuary Update

**UPD-01.** Distinguer version de l'application et version du pack. Une beta
du mod n'exige pas une nouvelle application si son contrat reste compatible.
Une mise à jour du launcher ne change pas à elle seule le monde ou le pack.

**UPD-02.** Les sources/releases sont sur GitHub ; les fichiers joueurs passent
par le relais HTTPS public. Préserver les URL packwiz déjà utilisées par Prism.
Aucune connexion Gitea ni aucun jeton GitHub privé n'est requis pour jouer.

**UPD-03.** À la demande de Koka, Sanctuary Update suit le
[runbook actif](https://github.com/sanctuary-minecraft/sanctuary/blob/main/docs/github-updates.md) :
artefacts vérifiés, canal, relais, serveur, téléchargement, actualités FR/EN,
wiki et même installation locale, selon le périmètre demandé. Conserver les
données personnelles ; ne pas déclarer une livraison complète après un échec.

**UPD-04.** La livraison du binaire du launcher aura une étape dédiée. Le
publieur actuel du pack attend exactement quatre artefacts et ne distribue
pas cette application. Prévoir paquets par plateforme, empreintes, origine
vérifiable, signature OS et sources correspondantes. Garder le dernier
exécutable fonctionnel jusqu'à vérification du nouveau, hors jeu actif.
La méthode de mise à jour reste à choisir avant distribution.

## 9. Langues et plateformes

Tous les nouveaux parcours, libellés et erreurs sont en FR/EN. Prévoir
notamment Jouer/Play, Installer/Install, Mettre à jour/Update, Réessayer/Retry,
Se connecter/Sign in, Non connecté/Not signed in et un état serveur inconnu.
Le choix de langue ne change pas les données de jeu. Les diagnostics sont
consultables et exportables après masquage des données de compte et jetons ;
aucun envoi automatique de journaux ou ajout de télémétrie n'est prévu ici.

Proposition de première validation : **Windows x64 et macOS Apple Silicon**,
correspondant aux usages connus. Linux, macOS Intel, Windows ARM et les OS
minimaux restent à décider. Une compilation Prism ne prouve pas la compatibilité
du pack Sanctuary sur une plateforme.

## 10. Critères de recette

| Vérification | Résultat exigé |
| --- | --- |
| Accueil FR/EN et échelle système | Hiérarchie de la maquette, textes lisibles et commandes accessibles |
| Installation vide et chemin accentué | Java, composants et pack prêts sans import manuel dans Prism |
| Téléchargement interrompu ou altéré | Reprise ou erreur claire ; aucun état à jour avec des fichiers incorrects |
| Deux synchronisations | Un seul JAR Sanctuary attendu et aucun changement au second passage |
| Premier démarrage graphique | Shaders et Sound Physics réellement actifs sur Vulkan après contrôle de compatibilité |
| Réglages personnels puis mise à jour | Désactivation volontaire, sauvegardes, captures et favoris conservés par empreintes |
| Jeu actif | Pas de réparation, mise à jour ou deuxième jeu concurrent sur le même dossier |
| Site absent ou flux malformé | Repli explicite ; fonctions encore disponibles utilisables |
| Microsoft annulé, expiré ou refusé | État exact, action de reprise utile et aucun jeton dans les diagnostics |
| Compte et admission valides | Arrivée directe sur Sanctuary sans étape volontaire dans le menu principal |
| Serveur indisponible ou admission refusée | Message utile et reprise sans boucle ni contournement |
| Distribution du launcher | Binaire identifié, origine vérifiée, sources correspondantes accessibles et mise à jour récupérable |

Les premiers essais utilisent des dossiers isolés. La reprise d'une instance
personnelle, les essais réels Microsoft et une livraison en production auront
leur demande correspondante. Ne pas ouvrir un monde personnel pour vérifier
une copie de fichiers ou une maquette.

## 11. Tickets futurs et points ouverts

Chaque ticket aura un résultat vérifiable et une branche `codex/<sujet>`.
Ce découpage définit les résultats visés ; les tickets ne sont pas encore des
issues GitHub. Le premier lot couvre la base et une partie de l'accueil.

| Ticket prévu | Résultat |
| --- | --- |
| LAUNCH-01 Base Prism | Confirmer le tag, conserver les historiques, appliquer l'identité Sanctuary et inventorier les API amont |
| LAUNCH-02 Accueil | Reproduire la maquette en Qt avec états simulés, FR/EN et navigation |
| LAUNCH-03 Installation | Installer/réparer depuis le relais dans un dossier isolé, avec reprise et protection des données |
| LAUNCH-04 Actualités | Définir puis livrer le flux public dans `sanctuary-web`, son cache et l'affichage launcher |
| LAUNCH-05 Réglages | Initialiser shaders/Sound Physics une fois ; prouver activation Vulkan et conservation des préférences |
| LAUNCH-06 Microsoft | Créer/configurer l'application, obtenir l'accès Minecraft et tester les vrais parcours de compte |
| LAUNCH-07 Rejoindre | Après installation et Microsoft, valider connexion directe et erreurs serveur/admission |
| LAUNCH-08 Distribution | Préparer paquets, signatures, sources et étape launcher de Sanctuary Update ; recette avant publication |

LAUNCH-02 à LAUNCH-05 peuvent avancer sans l'accès Microsoft. Les vrais essais
de compte et de multijoueur de LAUNCH-06/07 dépendent de cet accès et de comptes
autorisés. Le développement est engagé à la demande de Koka ; l'import manuel
prérempli du premier lot ne valide pas encore LAUNCH-03. Les actualités restent
embarquées tant que LAUNCH-04 n'a pas livré son contrat et son flux.

Avant les étapes concernées, valider les plateformes de distribution,
mesurer le profil graphique/mémoire, choisir la reprise d'une instance Prism,
valider le contrat du flux public et résoudre stockage protégé des jetons,
signature, mise à jour de l'application et fourniture des sources du fork.

## 12. Références

- [Maquette approuvée](design/README.md).
- [Release Prism 11.1.1](https://github.com/PrismLauncher/PrismLauncher/releases/tag/11.1.1),
  [commit amont](https://github.com/PrismLauncher/PrismLauncher/commit/7d2d3c1ec6fcf8255ebacd02d9f792688575dfb4),
  [politique des forks](https://github.com/PrismLauncher/PrismLauncher#forkingredistributingcustom-builds-policy)
  et [licence GPL](https://github.com/PrismLauncher/PrismLauncher/blob/11.1.1/LICENSE).
- [Enregistrement Entra](https://learn.microsoft.com/en-us/entra/identity-platform/quickstart-register-app)
  et [demande AppID Minecraft](https://aka.ms/mce-reviewappid).
- [Pack distribué](https://github.com/sanctuary-minecraft/sanctuary-pack),
  [actualités du site](https://sanctuary-minecraft.net/actualites) et
  [routes API existantes](https://github.com/sanctuary-minecraft/sanctuary-web/blob/main/routes/api.php).
- [Distribution et Sanctuary Update](https://github.com/sanctuary-minecraft/sanctuary/blob/main/docs/github-updates.md).
