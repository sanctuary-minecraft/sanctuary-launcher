# Origine des ressources Sanctuary

Le binaire blanc actuel utilise le mot-symbole original du site,
`launcher/resources/sanctuary/wordmark.png`, sans retaper le logo.
SHA-256 : `c173f73517f872fd0696de04674d0a125ff0ddf6b13ddfce160e27051a06a120`.

L'icône fournie par Koka, `sanctuary-icon2.png` (320 × 320), correspond
exactement au PNG déjà conservé sous `launcher/resources/sanctuary/icon.png`.
SHA-256 : `fbde6c7e30637f7cd6ad520f5a01f64b29ccc2a9a0fdeb766ee64de8591089d8`.
Les conteneurs ICNS/ICO ont été reconstruits à partir de ce PNG, par mise à
l'échelle au plus proche voisin. ICNS : 16 à 1024 px ; ICO : 16, 32, 48, 64,
128 et 256 px. `iconutil` relit le fichier macOS ; le bundle local contient
exactement l'ICNS source, SHA-256
`4e2a7eafb72f3935976f5f4ed5990fef71248cea88cc008ba14dfbb24ab8f010`.
Le logo n'a pas été redessiné.

La police SGA du site (`public/fonts/sga/Sga-Regular.woff2`, SHA-256
`249c1ec7fc876084fb0a70959c1bc597e31595ca1d81a5089f613182eef9dccd`)
est conservée en TTF dans les ressources Qt : le format WOFF2 n'était pas
chargé par `QFontDatabase` sur le poste. Conversion fontTools 4.60.2 avec
Brotli 1.2.1, famille `Sga`, sans nouvel alphabet ni dessin de glyphes.
TTF SHA-256 : `f9b069509d8a4876a6142229736f058d76024a6d7223806a5c28f106b3799efd`.
Le texte d'exemple local est fictif et ne vaut aucun accès serveur.

## Ressources du premier lot à paysage

Ces fichiers restent conservés pour l'historique et ne définissent plus
l'interface courante. Les maquettes blanches possèdent leurs registres
de provenance dans le [répertoire de design](README.md).

La maquette validée `accueil-sanctuary-reference-2026-11-08.png` reste
inchangée, avec l'empreinte conservée dans le
[registre historique](archive-accueil-2026-11-08.md).
Les images consommées par l'application sont versionnées dans
`launcher/resources/sanctuary/`, et non dans un cache personnel.

| Fichier | Origine et utilisation |
| --- | --- |
| `wordmark.png` | Copie sans transformation de `sanctuary-web/public/images/minecraft.png` ; mot-symbole existant |
| `sky.png` | Copie de `sanctuary-web/public/images/skybox/sky-0.png` ; fond du site |
| `button.png` | Copie de `sanctuary-web/public/images/ui/button.png` ; relief du bouton existant |
| `icon.png` | Copie de `sanctuary/packwiz/icon.png`, 320 × 320 ; icône de fenêtre |
| `island.png` | Imagegen intégré, 8 novembre 2026, à partir de la maquette approuvée ; paysage seul, 2172 × 724 |

La consigne de génération du paysage demandait de reprendre uniquement la
scène de la maquette : îles Minecraft flottantes, falaises, cascade, arbres,
maisons et poulet, avec un ciel bleu et des nuages cubiques ; aucun texte,
bouton, logo, panneau ou élément d'interface. Le format visé était un bandeau
3:1 de 2400 × 800 ; le PNG retenu mesure réellement 2172 × 724. Il est conservé
sans retouche de pixels. Qt effectue seulement son adaptation d'affichage.
Il s'agit d'une illustration, pas d'une capture du monde Sanctuary.

Les anciens conteneurs d'icône à favicon 32 × 32 du premier lot sont
remplacés par les tailles ci-dessus, issues de l'icône fournie par Koka.

Les sources et variantes graphiques héritées de Prism restent dans
l'historique et dans certains fichiers `program_info/`. Le branding des
installateurs et paquets non macOS, ainsi que l'Icon Composer de macOS 26,
doit être finalisé avant distribution. Le présent lot valide l'accueil Qt
et le bundle local construit avec l'icône ICNS Sanctuary.
