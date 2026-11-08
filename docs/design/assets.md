# Origine des images du premier lot

Ces ressources décrivent le premier lot à paysage, conservé pour l'historique.
La [direction blanche désormais retenue](README.md) possède son
[registre de provenance](blanc-2026-11-08/provenance.json) distinct ; aucune
ressource du binaire n'est remplacée par la révision documentaire 0.3.

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

Les conteneurs `program_info/sanctuary-launcher.icns` et `.ico` embarquent
directement les mêmes octets PNG du favicon 32 × 32 de
`sanctuary-web/public/favicon.png`. Aucun redessin ou redimensionnement n'a
été appliqué. Leur résolution reste provisoire avant distribution.

Les sources et variantes graphiques héritées de Prism restent dans
l'historique et dans certains fichiers `program_info/`. Le branding des
installateurs et paquets non macOS, ainsi que l'Icon Composer de macOS 26,
doit être finalisé avant distribution. Le présent lot valide l'accueil Qt
et le bundle local construit avec l'icône ICNS Sanctuary.
