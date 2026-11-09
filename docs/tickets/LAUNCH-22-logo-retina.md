# LAUNCH-22 — Logo net sur Retina

Branche : `codex/launcher-retina-wordmark`.

Corriger le mot-symbole PNG flou : conserver l’original 1024 × 172 et le dessiner
directement à la densité de l’écran, sans fabriquer un bitmap intermédiaire
180 px ensuite agrandi. Garder la taille logique et les alignements précédents,
dans la barre latérale et le composant d’accueil autonome.

Vérifier la construction et les rendus Qt à facteur 2, puis relancer le seul
profil de développement. Faire également le point documentaire sur les tests
Microsoft possibles avant la réponse AppID Minecraft, sans annoncer une
intégration de connexion ou un essai réel non effectué.
