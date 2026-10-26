# PACK-03 — Serveur Sanctuary préenregistré et icône

Ticket du **26 octobre 2026**, branche `codex/default-server`, version `beta.295`.

Le premier démarrage du client ajoute **Sanctuary Beta**, adresse publique
`beta.sanctuary-minecraft.net`, dans la liste multijoueur vanilla. Le mécanisme
est livré dans le JAR : il fonctionne après import du ZIP Prism du site,
installation MRpack et mise à jour d'une instance packwiz existante.

`servers.dat` reste un fichier personnel, jamais géré par packwiz. L'ajout
conserve l'ordre, les noms, réglages et champs inconnus des autres favoris.
Un favori Sanctuary existant est conservé sans doublon, y compris avec le nom
DNS en majuscules, un point final ou les ports 25565/25567. Une entrée cachée
issue de connexion directe devient visible sans être renommée. Avant la
première écriture, une copie exacte est conservée dans
`servers.dat.before-sanctuary-beta`. Un fichier illisible reste intact.

Le marqueur local `.sanctuary/default-server-beta` indique que l'ajout initial
est terminé : le joueur peut ensuite renommer, déplacer ou supprimer le favori.
Il n'est pas recréé à chaque lancement. Aucun téléchargement ni connexion de
jeu automatique n'est ajouté.

Le logo 160 × 160 du pack est redimensionné en PNG 64 × 64, sans nouvelle
illustration. Cette même image sert d'icône initiale du favori et de
`server-icon.png` dans le profil serveur packwiz. Le serveur annonce ainsi
l'icône aux clients lors du ping de la liste multijoueur. Le profil client ne
possède jamais `servers.dat` ; le profil serveur possède explicitement cette
icône. Les mondes et réglages de jeu ne changent pas.

La livraison commune intègre le ticket terminé
[CLAIM-03](property-command-beta294.md) : seule la commande anglaise
`/sanctuary property` et ses sous-commandes sont conservées. Les deux fichiers
de production, les deux fichiers de GameTests et le contrat sont repris
exactement depuis sa préparation locale beta.294, après ses 18 tests serveur
réussis, sans modifier son checkout de travail.

## Vérifications

- `defaultServerListSmoke` : 15 contrôles, dont lecture par `ServerData`
  vanilla, icône 64 × 64, favoris existants, sauvegarde exacte, suppression,
  absence de doublon et fichier corrompu suivi d'une réparation.
- Parcours client **Vulkan FR/EN**, GUI 3 en 1280 × 800 : réussi en 38 s,
  hook réel de démarrage, liste vanilla, nom et icône, ping public 0/20.
  Deux captures inspectées ; aucun monde ouvert. Journal et captures sous
  `build/release295/` et `mods/sanctuary/build/run/clientGameTest/screenshots/`.
- `./gradlew check build assemblePack assembleServerPack` : réussi en
  **3 min 22 s**, avec les contrôles purs et **18 tests serveur ciblés**
  `property270,claims272,claims277,claims278`, `sanctuaryAtlasOnly=true`.
  Les tests vérifient à nouveau la commande unifiée sur la base commune.
  Journal : `build/release295/check-build.log`.
- Publication Gitea, canal, CT 350, téléchargement Web et Prism : à consigner
  après livraison ; cette préparation ne prouve pas leur mise à jour.
