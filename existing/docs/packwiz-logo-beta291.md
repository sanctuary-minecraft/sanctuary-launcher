# PACK-LOGO — logo Sanctuary pendant packwiz

Branche `codex/packwiz-sanctuary-logo`, préparation locale **beta.291**, depuis
le socle beta.290. Demande du 26 octobre 2026 : afficher le bandeau Sanctuary
fourni dans la fenêtre de téléchargement et de mise à jour, avant Minecraft.

## Résultat

Le PNG `minecraft.png` fourni par le créateur est conservé octet pour octet
dans `launcher/packwiz/sanctuary.png` (1024 × 256, RGBA). Son SHA-256 est
`efee73a6c812c89b2900d41f6446c0f906ccf85136b6c226daf42870a41a5157`.
La fenêtre l'affiche sur fond noir à 640 × 160, proportions conservées,
au-dessus de la progression réelle. Ses boutons, le texte initial et le
compte à rebours suivent la langue du système : français, anglais sinon.
Les messages et dialogues du moteur amont conservent leurs traductions amont.

`scripts/packwiz_installer.py` télécharge le JAR officiel
[packwiz-installer v0.5.14](https://github.com/packwiz/packwiz-installer/releases/tag/v0.5.14),
vérifie son SHA-256, puis remplace uniquement `InstallWindow.class` par
l'adaptation Java de sa fenêtre Swing. Toutes les autres classes et notices
restent identiques : téléchargement, vérification des hashes, sélection des
mods facultatifs, erreurs et annulation continuent de dépendre de packwiz.
La licence MIT de comp500 et le source de l'adaptation sont joints au JAR.
Le logo est embarqué : aucun téléchargement d'image n'est requis pour l'afficher.

Le build utilise Java 25 ; les classes de la fenêtre ciblent Java 8 comme
l'amont. Les SHA-256 de l'amont, du logo et du résultat sont fixés dans
`launcher/packwiz/installer.json`. Une compilation différente échoue avant
d'écrire le JAR de distribution. Les dépendances et JAR générés restent dans
`build/packwiz-branding/`, ignoré par Git.

## Archive Prism et mises à jour

La commande habituelle `python3.14 scripts/pack.py prism <url-du-canal>`
embarque le bootstrap officiel vérifié et le nouvel installateur vérifié,
avec la même configuration Minecraft 26.3 / Fabric 0.19.5 et le même canal
stable. `--bootstrap-no-update` empêche le bootstrap de remplacer la fenêtre
Sanctuary par le binaire officiel. Cette option concerne **l'installateur** ;
la synchronisation des mods packwiz reste active à chaque lancement.
Le nom beta.291 du ZIP local n'avance pas ce canal : avant publication, un
lancement visant l'adresse stable récupère la version déjà publiée, beta.289
au moment de cette préparation. Les essais beta.291 utilisent un serveur HTTP
de développement isolé, jamais le monde ni le serveur personnel.

Les évolutions de l'installateur doivent donc être préparées et testées
explicitement, avec mise à jour du verrou et du vérificateur Web. Une simple
avance du canal de mods ne met pas à jour l'outil d'une instance existante.
Lors d'une future synchronisation locale demandée, sauvegarder l'instance,
copier le JAR vérifié et ajouter l'option à sa commande avant lancement en
conservant les autres réglages ; conserver cette même instance, sans réimport.
Le kit serveur conserve son installateur officiel et son fonctionnement sans
interface. Le logo n'est pas ajouté à l'index des fichiers du monde ou du jeu.

`verify_prism` contrôle désormais les deux JAR par leurs hashes et la présence
de l'option qui conserve l'habillage. Les sauvegardes et fichiers personnels
restent refusés dans le ZIP. Le MRpack n'embarque pas les commandes Prism et
ne suffit pas à installer cet habillage.

## Vérifications

- Logo fourni identique octet pour octet ; JAR résultant contrôlé par son
  SHA-256 `9e2912fdd05e5c23721acd58cd5c61bbf13061e453627e8f4df9a4c53f7aa875`.
- Les deux tests de distribution de `test_packwiz_installer.py` passent :
  toutes les entrées de l'amont sont relues, seule la classe de fenêtre change,
  et le véritable JAR vérifié est retrouvé dans l'export Prism.
- Les dix tests de `test_publish_web_pack.py` passent : outils modifiés refusés,
  option de conservation obligatoire, contrôles de publication préservés.
- Parcours Swing réel en FR et EN réussi avec le `GUIHandler` officiel :
  progression déterminée/indéterminée, options, Annuler, Continuer et compte à
  rebours. Les captures `preview-fr.png` et `preview-en.png` sont inspectées.
  Le test ne lance ni Minecraft ni un microphone.
- Deux exécutions de `check build assemblePack` avec le profil
  `whale290,recipes,progression` échouent sur le même test serveur hérité :
  `eggQuotaNaturalSpawnAndFlightUseActualServerEntities`, quota après utilisation
  de l'œuf entre Overworld et Nether (18 réussites sur 19). Le profil isolé
  `whale290` échoue également sur ce cas (6 réussites sur 7). Les sources du
  comportement et de son test ne sont pas modifiées dans ce chantier. Cet échec
  reste ouvert ; aucune validation complète des baleines n'est revendiquée.

- `./gradlew check build assemblePack -PsanctuaryFocusedTests=recipes,progression
  -PsanctuaryAtlasOnly=true` réussit en **2 min 42 s**, avec **13 tests serveur
  requis réussis**, les contrôles purs de `check`, le JAR et les profils client
  et serveur assemblés. Ce périmètre ne valide pas le test de baleines en échec.
- Comparaison des JAR beta.290/beta.291 : **2 658 classes de jeu identiques**,
  aucune classe ajoutée. Le compteur et les notices évoluent, sans changement
  du code de jeu.
- Deux lancements du véritable bootstrap et de l'installateur adapté réussis
  depuis le ZIP extrait dans `native instance with spaces/`, avec les arguments
  de sa commande Prism et un canal HTTP local beta.291. Un seul JAR Sanctuary,
  fichiers gérés identiques au second passage, installateur avec logo conservé,
  trois fichiers témoins personnels inchangés. Aucun monde ouvert. Le mode
  sans interface vérifie la synchronisation ; les tests Swing ci-dessus
  vérifient la fenêtre. Reçu `native-validation.json`.

Archive locale `build/Sanctuary-Prism-beta.291.zip` : **3 928 525 octets**,
SHA-256 `95c57f5e70627a2847d1cc2033c8d9f50f44ef4a302ca892bc727e0c37ece0a9`.
JAR Sanctuary beta.291 :
`bd130418a05f820dffd86b00595c5b7ce96293ec82867926f5493a95736aee73`.
Les interfaces Linux/Windows, le lancement Minecraft depuis Prism et une
mise à jour de l'instance personnelle ne sont pas revendiqués par ces essais
macOS. Le test de quota des baleines reste à résoudre avant une validation
complète de cette base de jeu.

Les reçus et captures restent sous `build/packwiz-branding/`.

Les mods évoqués pendant ce chantier sont seulement
[préparés en tickets pour la passe suivante](integrations-mods-next.md).
Aucune publication du canal, du site, du serveur Beta ou de l'instance
personnelle n'est demandée ou effectuée par cette préparation locale.
