# Les outils Prism dans Sanctuary Launcher

Contrat du [cahier des charges 0.6](cahier-des-charges.md), actualisé le
8 novembre 2026. Le choix d'une **seule instance officielle** et d'une
**fenêtre principale unique** remplace la navigation multi-instance exposée
par les premières propositions. Les données préexistantes ne sont pas supprimées.

## Navigation

La barre latérale présente Accueil, Actualités, puis les pages natives de
l'instance Sanctuary Beta. Les réglages globaux s'affichent dans la même
fenêtre et proposent un retour à Accueil. Ouvrir une page ne bascule plus
vers une fenêtre générique `InstanceWindow` de Prism.

Les pages sont celles de Prism 11.1.1, hébergées par `SanctuaryWorkspace` :
leurs fonctions et leurs critères de visibilité restent conservés.
Les sélecteurs de fichiers, téléchargements de contenu, sous-dialogues
spécialisés et ouvertures de dossiers restent des actions natives explicites.

| Outil | Fonctions conservées dans les pages natives |
| --- | --- |
| Mods | Liste, ajout, activation/désactivation, mises à jour et dossier |
| Packs de ressources | Gestion des packs et fichiers |
| Packs de shaders | Gestion des packs et ajouts compatibles |
| Captures d'écran | Consultation, gestion locale et dossier |
| Mondes | Sauvegardes locales et outils associés, dont datapacks applicables |
| Serveurs | Favoris et gestion native ; connexion réelle non testée dans ce lot |
| Versions | Minecraft, chargeur et composants |
| Réglages du jeu | Java, mémoire et paramètres de lancement |
| Journaux | Console et autres logs de l'instance |
| Notes et pages spécialisées | Affichées selon le type d'instance et les règles amont |

Sans pack importé, Accueil, Actualités et réglages globaux restent disponibles.
L'attente d'admission ne verrouille pas les outils locaux d'un pack installé.
La synchronisation les suspend pour éviter les écritures concurrentes.
Une version de contenu n'est pas déclarée compatible à partir de son seul nom.

## Installation officielle et personnalisation

L'import du site mémorise un identifiant unique. Les mises à jour Sanctuary
Update réutilisent cette instance, préservent les fichiers hors gestion et
contrôlent les empreintes des fichiers du pack. Les choix de désactivation
des mods facultatifs sont conservés. Un fichier requis modifié ou absent
fait échouer la vérification ; la page propose alors Vérifier / réparer.
Les composants peuvent être inspectés avec les outils Prism ; la validation
des écarts de versions et des ajouts personnels en jeu reste à compléter.

Les contrôles génériques de création, copie, suppression et renommage
d'instances sont masqués dans l'interface Sanctuary. Le modèle amont reste
présent dans les sources ; aucune migration automatique ou suppression des
instances d'un utilisateur n'est réalisée. Prism personnel reste distinct.

## Limites actuelles

Les pages et leur navigation ont été exercées dans la fenêtre native, et les
tests couvrent leur conteneur commun. Cela ne valide pas chaque opération
interne de Prism ni tous les services tiers. Les clés CurseForge/Imgur amont
restent absentes. La connexion Microsoft Sanctuary, Discord, le lancement
réel et l'accès serveur restent à raccorder/tester. La console du futur
lancement doit également rester intégrée au parcours de la fenêtre unique.
Les mondes locaux ne donnent aucun accès d'administration au monde serveur.
