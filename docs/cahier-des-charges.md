# Cahier des charges de Sanctuary Launcher

Version documentaire **0.5**, actualisée le **8 novembre 2026**. Sanctuary
Launcher est un fork natif de Prism avec une fenêtre principale blanche,
une seule instance officielle Sanctuary Beta et ses outils intégrés.

La nouvelle décision de Koka conserve les cinq étapes dans **Accueil**,
premier onglet avant les journaux, et ajoute **Actualités** séparément.
Avant préparation, Accueil présente le bloc des étapes avec le titre actif.
Quand comptes, admission et pack sont prêts, les étapes sont masquées et
l'accueil affiche Jouer, Intendance, Gazette et Tableau d'affichage.
Le code SGA reste facultatif. Cette organisation remplace la page
d'onboarding dédiée et la navigation multi-instance exposée précédemment.

Le lot natif local porte cette interface et l'installation réelle du pack.
Les comptes/admissions sont simulés pour les essais sans Microsoft ; les
liaisons authentifiées, le jeu et l'activation automatique restent à livrer.
Voir la [recette réelle](implementation/launcher-local-20261108.md).
Les [maquettes](design/README.md) restent des références historiques.

## 1. Objectif et périmètre

Le joueur relie ses comptes Discord et Microsoft, retrouve ou dépose sa
candidature, puis prépare son jeu. Il peut installer le pack pendant l'attente.
Quand son admission et son installation sont prêtes, **Jouer** rejoint Sanctuary.
L'activation du personnage ne demande pas de recopier un code dans le parcours
normal. Aux lancements suivants, il retrouve son dossier, son installation et
ses préférences ; seules les actions nécessaires lui sont redemandées.

Le parcours principal ne demande pas de connaître Prism, Java, Fabric ou
packwiz. **L'ensemble des outils de Prism reste accessible**, avec les pages
de gestion du jeu et les réglages globaux dans la même fenêtre. L'accueil Sanctuary
ne réduit pas ces possibilités. Le site conserve le dossier Sanctuary associé à
Discord ; aucun nouveau compte à mot de passe ni service Discord autonome
n'est créé. Le nouveau parcours exige des adaptations du site et du mod,
pas un second backend parallèle.

La communauté peut compter **plus de vingt membres**. Les vingt places
désignent la capacité simultanée de l'installation de jeu actuelle, lue dans
sa configuration, et non un quota d'admissions. Les joueurs historiques de
la bêta doivent être identifiés comme tels. Un futur serveur de jeu dédié
accueillant potentiellement des centaines de joueurs est une perspective
d'évolution, **hors périmètre de la V1 et sans déploiement planifié à ce stade**.

| Choix retenu | Conséquence |
| --- | --- |
| Fork de Prism avec identité Sanctuary | Conserver les mécanismes de lancement utiles de Prism et cibler les adaptations |
| DA blanche, sobre et centrée, inspirée de GitHub | Fond blanc sans décor, typographie lisible, traits gris fins et couleurs d'état discrètes |
| Un bloc, cinq étapes numérotées | Discord, Microsoft, accès, code facultatif et installation ; titre lié à l'étape active |
| Outils Prism intégrés | Pages natives de l'instance et réglages globaux dans la fenêtre Sanctuary ; une seule installation officielle |
| Accueil et Actualités | Étapes avant préparation, puis Jouer et communauté ; publications du site dans Actualités |
| Code SGA facultatif | Affichage/Copier à la demande et parcours manuel de secours ; aucun blocage normal sur la copie du code |
| Installation possible pendant l'attente | Préparer le jeu ne vaut pas admission au serveur |
| GitHub comme référence | Sources dans l'organisation `sanctuary-minecraft`, Gitea archivé |
| Sanctuary Update à la demande | Un push seul ne déclenche pas de déploiement |
| Shaders et Sound Physics activés initialement | Initialiser le profil dédié une fois, puis respecter les choix du joueur |
| Microsoft en attente | Le cadrage et les écrans peuvent avancer ; l'authentification réelle reste à raccorder |

## 2. Existant et base Prism

Le pack de départ du cadrage, **beta.372**, utilise Minecraft **26.3**,
Fabric **0.19.5** et Java **25**. Ces valeurs seront lues dans les métadonnées de publication et
ne doivent pas devenir des constantes dispersées dans le launcher.

| Élément | État au 8 novembre 2026 |
| --- | --- |
| Pack et instance Prism du site | Installation et mises à jour packwiz existantes |
| Bootstrap Sanctuary | Sources, logo et empreintes conservés dans `existing/` |
| Favori du serveur | Ajout initial géré par le mod ; ce n'est pas une connexion directe |
| Sound Physics Remastered | Option client sélectionnée par défaut ; les métadonnées actuelles décrivent son moteur comme désactivé par défaut |
| Shaders Sanctuary | Présents dans le mod ; le profil de première installation reste à adapter |
| Actualités FR/EN | Lecture native de la page publique `/actualites` ; contrat JSON/cache durable encore à préparer |
| Fork Sanctuary | Fenêtre blanche, navigation native intégrée et pack beta.373 installés/testés localement ; jeu non testé |
| Application Microsoft Sanctuary | Créée le 8 novembre, flux clients publics activés ; accès Minecraft et intégration au fork en attente |
| Admission actuelle du site et du mod | Candidature acceptée/parrainée, synchronisation whitelist, puis code obligatoire à la première création du personnage |
| Parcours en un bloc de cinq étapes, dont le code facultatif | Cible retenue ; liaisons authentifiées et activation sans saisie restent à développer |
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

## 3. Parcours en cinq étapes et DA blanche

### 3.1 Bloc unique et titre de l'étape en cours

**UI-01.** Présenter une interface blanche, sobre et centrée, inspirée de la
lisibilité de GitHub : colonne de contenu bornée, texte sombre sans empattement,
espaces réguliers, séparateurs gris fins et commandes compactes. Employer le
vert pour les étapes validées, l'ambre pour l'attente et des liens identifiables.
En haut à gauche, utiliser le mot-symbole Sanctuary original du site,
`sanctuary-web/public/images/minecraft.png`, déjà conservé sous
`launcher/resources/sanctuary/wordmark.png`, accompagné du texte discret
**Launcher**. Conserver sa transparence et ses proportions, sans le retaper,
le redessiner ni extraire sa reproduction depuis la maquette générée.
Le ciel, le paysage, les textures pierre/terre et les boutons Minecraft en
relief restent des références historiques. Le SGA est réservé au code ; les
consignes et commandes restent lisibles en français ou en anglais.

**FLOW-03.** Garder les cinq lignes dans un même bloc, numérotées de 1 à 5 :
**Discord**, **Microsoft**, **Accès Sanctuary**, **Code de personnage** et
**Installation**. La ligne active est mise en évidence et le titre principal
nomme son action ou son état, plutôt que d'afficher « Bienvenue sur Sanctuary ».
Les étapes déjà accomplies restent visibles et mémorisées tant que le bloc est nécessaire. Dès que les prérequis obligatoires sont réunis, masquer le bloc complet. La ligne 4 porte
**Facultatif**, sans case ou validation requise pour débloquer l'installation
ou Jouer. Ne pas présenter « 5/5 validées » comme condition de lancement.

| Étape ou état actif | Titre principal indicatif |
| --- | --- |
| Discord manquant | Connecter Discord |
| Microsoft manquant ou expiré | Connecter Microsoft |
| Demande non déposée | Demander à rejoindre Sanctuary |
| Candidature en attente | Accès Sanctuary — En attente |
| Code ouvert volontairement | Mon code de personnage — Facultatif |
| Jeu à préparer | Installer Sanctuary |
| Téléchargement réel | Installation de Sanctuary |
| Comptes valides, admission confirmée et jeu prêt | Jouer à Sanctuary |

La prochaine action nécessaire guide le focus ; une session expirée rouvre
seulement sa connexion. L'attente d'admission laisse proposer l'installation
à l'étape 5. Ne pas forcer l'ouverture de la ligne 4 entre accès et installation.
Les cinq numéros décrivent le parcours, pas cinq obligations de même nature.

### 3.2 Étapes 1 et 2 — Discord et Microsoft

**FLOW-01.** Présenter les deux connexions sur les lignes 1 et 2 du bloc :
Discord identifie le dossier Sanctuary et Microsoft vérifie le profil et la possession de
Minecraft Java. Les identifiants sont saisis dans le navigateur chez le
fournisseur concerné. Afficher le compte effectivement lié pour éviter une
association involontaire, sans demander au joueur de retaper son pseudo Minecraft.

**FLOW-02.** Reprendre les connexions et le dossier existants, sans nouvelle
candidature ni nouveau personnage. Une session expirée rouvre seulement la
connexion concernée ; elle ne réinitialise ni l'admission, ni le jeu installé,
ni les préférences. Un changement de compte demande une association explicite
et ne transfère pas les droits d'un autre joueur.

### 3.3 Étape 3 — Accès Sanctuary

**ACCESS-01.** Retrouver la candidature du même dossier sur le site et dans le
launcher. Si elle n'existe pas, proposer **Demander à rejoindre**, avec le profil
Minecraft vérifié prérempli et une soumission explicite. L'authentification
seule ne dépose pas de candidature et ne crée pas d'admission.

**ACCESS-02.** Montrer un statut exact et la prochaine action utile : candidature
à envoyer, en attente, acceptée, refusée ou accès retiré. Le parrainage reste
une voie facultative d'acceptation du même dossier, selon les règles et quotas
du site. **J'ai une invitation** ou **Utiliser un code de parrainage** ne crée
pas une seconde procédure d'inscription et ne contourne pas un refus serveur.
Une acceptation dont la synchronisation serveur n'est pas terminée affiche
**Accès accordé — préparation en cours**, sans annoncer que le jeu est accessible.

**ACCESS-03.** Le site décide de l'admission ; le serveur confirme les droits
de la session et autorise la création du personnage à la première arrivée.
La cible est une liaison automatique des comptes vérifiés, sans copie d'un
code dans le parcours normal. La création du personnage en jeu reste distincte
de l'admission. Aucune activation ni consommation n'a lieu à la simple connexion
au launcher, à l'affichage du code, au téléchargement ou à son annulation.

**ACCESS-06.** Les **20 places** désignent la capacité simultanée du serveur,
distincte du nombre de membres et de la liste des candidatures. La communauté
et les admissions peuvent dépasser vingt joueurs. Lire la capacité publiée
du serveur plutôt que figer 20 dans le launcher. **Serveur complet — 20/20**
ne remet pas un joueur admis en liste d'attente. Une réponse périmée ou absente
reste inconnue ; aucune file automatique ou réservation de place n'est promise ici.

**ACCESS-07.** Identifier les **joueurs historiques de la bêta** par un marqueur
durable visible à la ligne Accès Sanctuary et dans le dossier du site. Sa
provenance vient des historiques de référence confirmés ou d'une validation de l'équipe, sans
l'attribuer à tout nouvel utilisateur du launcher. Conserver ce marqueur lors
des reconnexions, réinstallations et évolutions futures. Il est informatif ;
les droits d'accès continuent de suivre l'admission et sa révocation éventuelle.
Le périmètre historique et la source de rattachement seront documentés avant
un import, sans modifier les personnages ou mondes pour fabriquer ce statut.

### 3.4 Étape 4 — Code de personnage facultatif

**ACCESS-04.** Le code de personnage en SGA est **facultatif pour l'utilisateur**.
Le placer sur la ligne 4 sous **Afficher mon code SGA**, avec **Copier**, après
acceptation du dossier. Il ne figure pas comme étape obligatoire à cocher et
ne conditionne pas Jouer lorsque l'admission automatique est confirmée.
Afficher le code existant, sans réémission à chaque ouverture. Le code de
parrainage destiné à inviter quelqu'un reste distinct et en lettres ordinaires.

**ACCESS-05.** Conserver le parcours manuel à code pour l'assistance et les
clients existants tant que la transition n'est pas validée. Le code ne permet
pas d'usurper un compte, de réutiliser une admission d'un autre monde ou de
passer outre une révocation. Une erreur de reprise conserve les données et
offre une nouvelle tentative ou une aide, sans recréer un personnage existant.

### 3.5 Étape 5 — Installation et accès au jeu

**GAME-01.** Préparer Java, les composants compatibles, le pack et les réglages
initiaux depuis une seule commande **Installer Sanctuary**. Cette préparation
est possible à la demande pendant l'attente d'admission ; elle ne vaut pas
autorisation de rejoindre. Conserver le résultat pour éviter un nouveau
téléchargement après acceptation. Respecter les contraintes d'authentification
des téléchargements et la vérification de propriété avant tout jeu.

**GAME-02.** Activer **Jouer** lorsque le compte Minecraft est valide, l'admission
serveur est confirmée et les fichiers requis sont vérifiés, sans opération
concurrente sur cette installation. Le code SGA n'est pas une validation manuelle
supplémentaire. Les contrôles serveur restent obligatoires à l'arrivée, même
si le launcher affichait précédemment un état prêt.

**GAME-03.** Garder les raccourcis **Mods**, **Shaders** et **Réglages** utiles
au jeu installé via la barre latérale des pages natives de Prism.
Cette navigation reste disponible dans la même fenêtre. Une installation
préparée peut être configurée pendant l'attente ;
son droit d'entrée au serveur demeure distinct. Conserver les préférences aux
lancements et mises à jour suivants, sans réexécuter l'assistant complet.

### 3.6 Accueil quotidien et états

**UI-02.** Afficher les comptes, la version réellement installée du pack et son
état de mise à jour. Le statut serveur porte une date d'observation ; une
information absente ou périmée donne un état inconnu. Les mentions de version,
de disponibilité et de compte de la maquette ne sont jamais des textes fixes.

**UI-03.** **Accueil** est le premier onglet, au-dessus des journaux du jeu.
Un onglet **Actualités** distinct présente les dernières versions, dates,
titres et résumés publiés sur le site. Ne pas recopier une rédaction statique.

Avant préparation, Accueil affiche les cinq lignes et masque Intendance,
Gazette et Tableau d'affichage. Quand les comptes sont valides, l'admission
est confirmée et le pack vérifié, masquer les étapes et montrer **Jouer**,
**Intendance**, **Gazette** et **Tableau d'affichage**. Le code SGA n'est pas
une condition de cette transition ; il reste consultable à la demande.
Une session à renouveler ou un accès retiré remet les étapes nécessaires
sans effacer les préférences. Les rubriques communautaires gardent les
permissions du site et ne deviennent pas publiques par leur intégration.

**UI-04.** Garder l'essentiel lisible à 1280 × 800 et à la taille de référence,
y compris avec l'échelle système et les textes anglais. Prévoir navigation
clavier, focus visible et commandes compréhensibles sans dépendre d'une image.

| État | Comportement principal attendu |
| --- | --- |
| Compte manquant ou session expirée | La ligne Discord ou Microsoft demande seulement la connexion nécessaire |
| Pas de candidature | Accès Sanctuary propose une demande explicite préremplie ou une invitation |
| Candidature en attente, jeu absent | Attente expliquée ; Installer Sanctuary disponible à la demande ; Jouer indisponible |
| Candidature en attente, jeu installé | Installation conservée, réglages accessibles, statut consultable ; Jouer indisponible |
| Candidature refusée ou accès retiré | Motif approprié et démarche utile ; pas de nouvelle soumission automatique |
| Accès accepté, synchronisation serveur inachevée | Accès accordé — préparation en cours ; aucune déclaration prématurée de disponibilité |
| Accès accepté, installation absente | Installation devient l'étape active, avec Installer Sanctuary |
| Téléchargement ou mise à jour | Progression, interruption propre et reprise ; lancement indisponible avant vérification des fichiers requis |
| Comptes valides, admission confirmée et pack prêt | Jouer rejoint Sanctuary sans demander la saisie du code SGA |
| Joueur admis, serveur complet | Afficher Serveur complet et une reprise possible ; conserver l'admission |
| Jeu en cours | État En cours de jeu ; pas de second lancement sur le même dossier |
| Échec | Cause utile, Réessayer et accès au diagnostic, sans boucle de relancement |

### 3.7 Outils natifs dans la fenêtre Sanctuary

**PRISM-01.** La navigation persistante ouvre les pages complètes de
l'instance officielle : mods, ressources, shaders, captures, mondes, serveurs,
versions, notes, réglages, console et autres journaux. Conserver les pages
spécialisées lorsqu'applicables ; voir le [contrat Prism](outils-prism.md).

**PRISM-02.** Réutiliser les pages natives dans un conteneur Sanctuary commun,
sans ouvrir un second éditeur générique Prism. Leurs sous-dialogues utiles,
sélecteurs de fichiers et ouvertures explicites de dossiers restent permis.
Les règles de visibilité et de compatibilité amont continuent de s'appliquer.

**PRISM-03.** Présenter les réglages globaux — Java, mémoire, Minecraft,
comptes, apparence, langue, réseau et outils externes — dans cette même fenêtre,
avec retour visible à Accueil et conservation des modifications valides.

**PRISM-04.** La candidature régit l'accès au serveur, sans supprimer les
fichiers ou verrouiller les outils locaux du pack installé. Suspendre les
écritures de ces outils pendant la synchronisation. Les mondes locaux
restent distincts du monde du serveur.

**PRISM-05.** Exposer une seule instance officielle Sanctuary Beta, mémorisée
par son identifiant et mise à jour en place. Masquer les commandes génériques
de création/copie/suppression d'instances dans le parcours principal. Conserver
les données préexistantes sans import automatique de Prism personnel.
Les ajouts et préférences personnels doivent survivre aux mises à jour ;
un fichier requis modifié doit être signalé ou réparé avant de déclarer
le pack conforme. Le contrôle des versions/composants modifiés reste à terminer.

## 4. Installation et protection des données

**INST-01.** Réutiliser Prism pour Java, les composants du jeu et le lancement,
et packwiz pour le pack. Une installation vide doit aboutir sans import manuel
d'un ZIP dans Prism. Vérifier espace, droits, plateforme, versions et empreintes.
Les chemins avec espaces ou caractères accentués doivent fonctionner.

**INST-02.** Reprendre ou retenter les téléchargements interrompus. Un fichier
manquant ou incorrect ne permet pas d'annoncer « Pack à jour ». Un verrou
empêche lancement, réparation et mise à jour concurrents du même dossier.

**INST-03.** Gérer une seule installation Sanctuary par profil du launcher.
Cette règle concerne l'installation officielle suivie par Sanctuary Update ;
elle ne migre ni ne modifie les installations d'autres launchers.
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

**AUTH-04.** Raccorder la connexion Discord au dossier du site et associer le
profil Minecraft réellement vérifié, avec son identifiant stable, au même
joueur. Un pseudo ou UUID simplement déclaré par le client ne suffit pas à
prouver cette liaison. Ne pas copier de jeton de bot Discord, de secret client
ou de jeton d'administration du serveur dans le launcher.

**AUTH-05.** La récupération du dossier, de l'admission et du code utilise un
contrat authentifié propre au joueur, à définir côté site. Les routes actuelles
`/api/v1/access-codes/verify` et `/redeem` utilisent un jeton réservé au serveur
et ne constituent pas une API à embarquer telle quelle dans l'application de
bureau. Le flux public d'actualités ne transporte aucune de ces données privées.

**JOIN-01.** Après authentification, confirmation de l'admission et contrôle du
pack, rejoindre l'adresse Sanctuary sans étape volontaire dans le menu principal. Prism
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

**JOIN-04.** L'activation sans saisie demande un nouveau contrat entre site,
launcher et mod, conservant la décision et la confirmation côté serveur.
Le [contrat actuel de première arrivée](https://github.com/sanctuary-minecraft/sanctuary/blob/main/docs/inscription-web-beta166.md)
exige encore un code ; il ne devient pas facultatif par une simple modification
de l'interface. Prévoir la coexistence avec les clients existants et les reçus
de reprise, liés au joueur et au monde par le serveur. Aucun nouveau monde,
effacement d'admission ou changement de sauvegarde n'est autorisé par ce cadrage ;
toute migration nécessaire devra avoir son propre contrat et sa sauvegarde.

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

**CFG-05.** Permettre l'ajout de mods client facultatifs et la sélection de
shaders compatibles avec la version exacte du jeu et le moteur graphique retenu.
Distinguer ces ajouts des fichiers gérés indispensables au pack et au serveur,
conserver les choix personnels pendant une synchronisation et expliquer une
incompatibilité avant de déclarer l'installation prête. La gestion locale des
mods et shaders ne modifie ni l'admission ni la configuration du serveur.

## 7. Actualités et données publiques

**WEB-01.** Lire les actualités publiques FR/EN déjà publiées
par Sanctuary Web. Éviter une seconde rédaction des notes ou une lecture de
données privées GitHub. Les liens renvoient aux pages HTML existantes.

Le lot local lit actuellement le tableau des versions de `/actualites` en
lecture seule, sans scripts. Un contrat structuré et un cache durable restent
à préparer. La route versionnée proposée est `/launcher/v1/home.json`.
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

**WEB-04.** Réutiliser les rubriques du site pour l'accueil quotidien :
Intendance correspond au flux `bulletin`, Gazette à `article`, Tableau
à `notice`. Les routes `/communaute/flux?kind=…` demandent une session Discord.
Le lot local affiche cet état sans publier les contenus privés et sans
considérer un compte simulé comme authentifié. Le pont de session reste à livrer.

**WEB-03.** Le contrat privé à livrer pour le launcher couvre le dossier du
joueur, la candidature et le parrainage, la confirmation d'accès serveur,
le marqueur historique bêta et la consultation facultative du code existant.
Reprendre un dossier déjà présent ; éviter les candidatures et admissions
dupliquées. Afficher l'état inconnu en cas d'échec de lecture, sans transformer
une erreur réseau en refus ni un cache ancien en preuve d'admission. Le site
et le serveur restent les autorités de leurs décisions respectives.

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
Les lignes du parcours sont Discord/Discord, Microsoft/Microsoft, Accès Sanctuary/Sanctuary access,
Code de personnage/Character code et Installation/Installation. Traduire aussi les titres
dynamiques de l'étape active, sans modifier sa numérotation.
Prévoir En attente/Pending, Accès accordé/Access granted, Serveur complet/Server full,
Afficher mon code/Show my code et Joueur historique de la bêta/Historical Beta player.
Prévoir Accueil/Home, Actualités/News, Packs de ressources/Resource packs,
Packs de shaders/Shader packs, Captures d'écran/Screenshots, Mondes/Worlds,
Serveurs/Servers, Versions/Versions et Journaux/Logs.

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
| DA blanche, bloc unique, cinq étapes et FR/EN | Cinq lignes numérotées lisibles à 1280 × 800 ; titre correspondant à l'étape active ; ligne 4 facultative sans prérequis de copie du code |
| Identité dans l'en-tête | Mot-symbole original du site en haut à gauche, proportions et transparence conservées, texte Launcher discret à côté |
| Comptes et dossier déjà liés | Reprise de la candidature et des personnages ; pas de doublon ni d'assistant complet rejoué |
| Un seul compte expiré ou compte changé | Reconnexion ciblée ou association explicite ; installation et droits de l'autre compte non transférés |
| Candidature en attente | Statut cohérent avec le site ; installation possible à la demande, Jouer indisponible |
| Candidature acceptée, synchronisation serveur inachevée | Préparation en cours explicite, sans déclarer l'accès jouable trop tôt |
| Refus ou accès retiré | Action utile et refus serveur respecté, même si le launcher affichait un ancien état prêt |
| Code SGA jamais consulté ou copié | Un joueur autorisé peut rejoindre et créer son personnage par le parcours automatique |
| Consultation facultative du code indisponible | Erreur ciblée dans Afficher mon code ; une admission automatique confirmée reste utilisable |
| Code affiché, copie, installation annulée | Aucune réémission ou consommation prématurée ; code inchangé et masqué avant admission |
| Première arrivée, coupure, client manuel existant | Activation automatique ou secours à code selon le contrat ; reprise sans second personnage ni admission d'un autre monde |
| Communauté dépassant vingt membres, serveur plein | Admissions non plafonnées à vingt ; capacité simultanée lue du serveur, admission conservée en cas de saturation |
| Joueur historique de la bêta | Marqueur sourcé conservé après réinstallation ; aucun nouveau joueur marqué historique par simple connexion |
| Installation vide et chemin accentué | Java, composants et pack prêts sans import manuel dans Prism |
| Téléchargement interrompu ou altéré | Reprise ou erreur claire ; aucun état à jour avec des fichiers incorrects |
| Deux synchronisations | Un seul JAR Sanctuary attendu et aucun changement au second passage |
| Premier démarrage graphique | Shaders et Sound Physics réellement actifs sur Vulkan après contrôle de compatibilité |
| Réglages personnels puis mise à jour | Désactivation volontaire, sauvegardes, captures et favoris conservés par empreintes |
| Mods facultatifs et shaders personnalisés | Compatibilité contrôlée, ajouts et préférences conservés, fichiers serveur requis toujours vérifiés |
| Outils Prism | Pages complètes de l'instance dans la fenêtre principale, navigation persistante et retour Accueil |
| Réglages globaux | Java, mémoire, comptes, réseau, apparence et outils externes dans la même fenêtre ; retour Accueil |
| Admission en attente et outils locaux | Gestion des fichiers de l'instance accessible selon ses prérequis ; aucun droit d'accès Sanctuary accordé par ces outils |
| Profil officiel et personnalisation avancée | Version/composants modifiés signalés, copies personnelles distinguées du profil synchronisé, sauvegardes et données locales conservées |
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
issues GitHub. Le lot local couvre la fenêtre et une partie des parcours ;
les liaisons authentifiées et les essais du jeu restent distincts.

| Ticket prévu | Résultat |
| --- | --- |
| LAUNCH-01 Base Prism | Confirmer le tag, conserver les historiques, appliquer l'identité Sanctuary et inventorier les API amont |
| LAUNCH-02 Accueil | Fenêtre blanche, Accueil avant les journaux, étapes puis communauté selon préparation ; livré localement avec états simulés |
| LAUNCH-03 Installation | Installer/réparer depuis le relais dans un dossier isolé, avec reprise et protection des données |
| LAUNCH-04 Actualités | Lecture native du site livrée ; contrat structuré et cache durable à compléter |
| LAUNCH-05 Réglages | Initialiser shaders/Sound Physics une fois ; prouver activation Vulkan et conservation des préférences |
| LAUNCH-06 Comptes | Raccorder Microsoft et Discord au même dossier, obtenir l'accès Minecraft et tester les vrais parcours de connexion/reprise |
| LAUNCH-07 Rejoindre | Après installation et Microsoft, valider connexion directe et erreurs serveur/admission |
| LAUNCH-08 Distribution | Préparer paquets, signatures, sources et étape launcher de Sanctuary Update ; recette avant publication |
| LAUNCH-09 Accès Sanctuary | Raccorder candidature/parrainage et statuts serveur au dossier unique, avec installation possible pendant l'attente |
| LAUNCH-10 Activation | Livrer le contrat site/launcher/mod sans saisie normale du code, avec secours manuel et reprise des admissions existantes |
| LAUNCH-11 Personnalisation | Gérer les mods client facultatifs et shaders compatibles sans écraser le pack requis ni les préférences |
| LAUNCH-12 Historiques bêta | Définir la provenance et afficher le marqueur des anciens joueurs, sans modifier leurs personnages |
| LAUNCH-13 Outils Prism | Conteneur unique livré localement ; poursuivre la recette des opérations et du lancement réel |

LAUNCH-02 à LAUNCH-05 peuvent avancer sans l'accès Microsoft. Les vrais essais
de compte et multijoueur attendent cet accès. Le lot local importe et synchronise
réellement beta.373 depuis le site ; il ne prouve pas une session Minecraft,
une admission ou une activation. Les actualités sont lues sur le site.
Les rubriques privées, les admissions et le badge historique exigent encore
leurs liaisons authentifiées et sources de vérité. Le mod conserve actuellement
son parcours manuel à code ; l'option SGA du launcher ne le supprime pas.

Avant les étapes concernées, valider les plateformes de distribution,
mesurer le profil graphique/mémoire, choisir la reprise d'une instance Prism,
valider le contrat du flux public et résoudre stockage protégé des jetons,
signature, mise à jour de l'application et fourniture des sources du fork.
Le serveur dédié pour des centaines de joueurs reste une perspective ; ce
découpage ne programme ni sa construction, ni une expansion du monde, ni une
migration de l'infrastructure actuelle.

## 12. Références

- [Maquette en cinq étapes, titre actif, identité et limites](design/README.md),
  [première direction conservée en archive](design/archive-accueil-2026-11-08.md).
- [Conservation de tous les outils Prism](outils-prism.md).
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
