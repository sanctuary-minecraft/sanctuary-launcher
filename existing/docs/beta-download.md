# DIST-WEB — téléchargement direct d'une instance Prism prête

Branche `codex/beta-download`, serveur dédié CT 350 sur `gpu`. Cette livraison
conserve les binaires immuables beta.280 et ne modifie ni monde ni format de
sauvegarde. L'Alpha reste hors cible.

## Résultat pour le joueur

Le bouton **Télécharger** de [Sanctuary Web](https://sanctuary-minecraft.net)
donne directement le fichier **Sanctuary-Beta.zip** :

```text
https://sanctuary-minecraft.net/downloads/sanctuary-prism.zip
```

Le joueur importe ce ZIP dans Prism (**Ajouter une instance → Importer depuis
un ZIP**), puis lance **Sanctuary Beta**. Packwiz télécharge le pack complet au
premier lancement et suit le même canal aux lancements suivants. La commande
avant lancement et le bootstrap sont déjà inclus ; aucune configuration manuelle
ou téléchargement séparé d'outil n'est demandé.

Le format ZIP d'instance Prism conserve ces réglages, contrairement à un MRpack
standard. Le MRpack beta.280 reste accessible par son ancienne adresse pour
compatibilité des liens, sans devenir un deuxième bouton du site. Le parcours
manuel de configuration et son lien dans le pied de page sont retirés.

L'archive légère contient uniquement `instance.cfg`, `mmc-pack.json`, l'icône et
`minecraft/packwiz-installer-bootstrap.jar`. Elle démarre avec zéro mod local.
L'instance est nommée **Sanctuary Beta**, avec `ManagedPack=false`,
`OverrideCommands=true`, la sélection automatique de Java et le canal stable :

```text
https://git.botsu.net/koka/sanctuary-beta/raw/branch/packwiz/pack.toml
```

Le nom du bouton et le téléchargement direct sont conservés. Le pied de page et
le survol indiquent la version réellement publiée. Les réponses de téléchargement
utilisent `Cache-Control: no-store` pour garder la même adresse à chaque livraison.

## Publication demandée via Codex

Aucune tâche périodique ne décide de publier une version. À la demande du créateur :

1. Vérifier et publier les nouveaux binaires immuables, puis le canal packwiz
   selon [le protocole de distribution](packwiz.md).
2. Générer l'instance prête avec `python3 scripts/pack.py prism <url-du-canal>`.
   Joindre `build/Sanctuary-Prism-beta.xxx.zip` à la release, avec le MRpack et
   le kit serveur. L'export Prism est reproductible et conserve ses commandes.
3. Mettre à jour la CT dédiée selon [le contrat serveur](server-pack.md), en
   conservant sauvegardes, réglages et secrets, puis vérifier son redémarrage.
4. Publier le téléchargement Web après contrôle de cette CT et des deux archives :

```sh
python3 scripts/publish_web_pack.py \
  --web-ct 320 --server-ct 350 --version beta.280 \
  --mrpack-url https://git.botsu.net/koka/sanctuary-beta/releases/download/beta.280/Sanctuary-beta.280.mrpack \
  --prism-url https://git.botsu.net/koka/sanctuary-beta/releases/download/beta.280/Sanctuary-Prism-beta.280.zip \
  --pack-url https://git.botsu.net/koka/sanctuary-beta/raw/branch/packwiz/pack.toml
```

Cette dernière commande s'exécute sur `gpu`, où `pct` est disponible. `--dry-run`
vérifie sans publier. Le script contrôle les index, les versions, les hashes des
dépendances et le JAR Sanctuary de la CT, du canal et du MRpack. Il refuse une
archive Prism sans commande active, visant un autre canal ou une autre plateforme,
avec un bootstrap non vérifié ou des fichiers étrangers.

Les fichiers Web restent dans
`/var/www/sanctuary/storage/app/distribution/releases/beta.xxx/prism/`.
Chaque publication est immuable. Sous verrou, `current` bascule atomiquement après
écriture complète. Les anciennes versions restent disponibles pour une reprise.
Le script ne redémarre aucun serveur et ne change pas le canal lui-même.

## Vérifications

Huit tests Python couvrent l'alignement CT/canal/MRpack, le refus d'un serveur
arrêté ou divergent, le contrat d'import Prism prêt, les fichiers interdits et la
publication atomique avec conservation de la précédente en cas de conflit.
Ils font partie de `./gradlew check`.

Les quatre tests de téléchargement et les trois tests des métriques Laravel
passent, ainsi que Pint : le bouton sert un ZIP anonyme directement, le nom est
`Sanctuary-Beta.zip`, les publications incomplètes restent indisponibles, et le
changement de version ne demande pas de reconstruire le cache de configuration.
Le MRpack reste téléchargeable par son ancien lien.

Les tests natifs utilisent une extraction de l'archive prête dans un chemin
contenant des espaces, avec les variables de sa propre commande avant lancement.
Ils exécutent packwiz sans interface ; aucun Minecraft ni monde personnel n'est
ouvert. Les fichiers témoins de réglages, favoris et sauvegardes sont comparés
avant et après synchronisation. Les journaux sont conservés sous
`build/prism-ready-validation*/`, hors de Git.

`./gradlew check build assemblePack -PsanctuaryFocusedTests=recipes,progression
-PsanctuaryAtlasOnly=true` réussit. L'archive prête mesure **92 417 octets**, SHA-256
`30dd37f3a6e1d79f8b443f6312918258276c85d5b6a5e7fbdc9c3fea7562dbf2`.
Une première tentative native reçoit des HTTP 502 de Gitea ; une nouvelle
extraction entièrement vide réussit ensuite ses deux lancements de synchronisation.
Le reçu est `build/prism-ready-validation-retry/validation.json`.

L'archive est ajoutée à la release beta.280 sans changer son tag ni ses artefacts
existants. La CT Web publie ensuite le ZIP et le MRpack vérifiés dans la même
bascule atomique. Le code Web est livré au commit
`46d784ecbb8ba3d0f20199b85bcf486831ba3750` ; les changements préexistants sont
conservés et les empreintes des comptes et permissions restent identiques.
Sauvegarde privée :
`/root/sanctuary-beta-maintenance/web-prism-ready-20261025-1792943502/`.

Le parcours navigateur **accueil → Télécharger → Sanctuary-Beta.zip reçu** est
vérifié à 1280 × 900 et 390 × 844 : HTTP 200 direct, pas de changement de page,
nom et SHA-256 identiques à l'archive testée, aucune erreur d'application ni
débordement horizontal. Le lien de guide manuel est absent. Le MRpack historique
répond toujours HTTP 200. Le navigateur utilisé est Playwright, le plugin
Browser étant absent ; reçu et captures sous `/tmp/sanctuary-prism-ready-*`.
Reçus opérationnels ignorés : `build/prism-ready-publication.json`,
`build/prism-ready-web-publication.json`, `build/prism-ready-web-deployment.log`
et `build/prism-ready-build.log`.
