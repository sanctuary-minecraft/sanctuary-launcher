# Jouer et connexion directe Beta — 13 novembre 2026

Ticket LAUNCH-24, branche `codex/launcher-play-server`.

## Raccordement

Jouer demande un lancement uniquement après confirmation du compte Microsoft,
de la licence et du profil, d’une session Discord et d’un dossier admis portant
le même pseudo Minecraft (comparaison insensible à la casse). Le pack officiel
et l’instance doivent être présents et vérifiés. L’attente, le retrait d’accès,
une autre identité Minecraft ou une simulation locale ne remplacent ces preuves.

MainWindow revérifie ces conditions au clic, enregistre les pages ouvertes,
contrôle les empreintes de l’installateur et des fichiers gérés, puis appelle
`Application::launch` en mode Normal avec le compte par défaut vérifié et une
cible `beta.sanctuary-minecraft.net`. Le moteur Prism garde authentification,
rafraîchissement, téléchargement du jeu, précommande packwiz, Quick Play et
résolution SRV. Aucun jeton ne passe par un script de lancement maison.

Le lancement et la partie verrouillent Jouer, la synchronisation du pack et
ses pages d’édition. Journaux du jeu, autres journaux et captures restent
accessibles dans la fenêtre Sanctuary. Les demandes de console natives pour
cette instance rejoignent le même onglet, sans fenêtre InstanceWindow séparée.

Le script Run utilise le Java 25 local pour le jeu en remplaçant seulement
l’ancienne valeur Run Java 17 ; le build des auxiliaires Prism reste en Java 17.
Les préférences de runtime personnalisées sont conservées.

## Vérifications automatiques

Construction par `./script/build_and_run.sh --verify` réussie ; **25 suites CTest**,
26,80 secondes après les derniers correctifs. Les tests couvrent le signal de lancement réel et la séparation
des simulations, le pseudo incorrect, les accès non admis, Discord déconnecté
ou en cours de vérification, l’instance absente, le pack non vérifié ou occupé,
le second lancement, la reprise après la partie et l’accès aux journaux.

## Recette réelle

Le test Microsoft du 13 novembre réussit et l’interface confirme réellement
licence et profil. Le serveur répond au ping Minecraft 26.3, zéro joueur sur
vingt places avant l’essai ; le SRV indique le port public 25567. Ces contrôles
ne constituent pas encore une arrivée de joueur.

La première ouverture recompilée attend l’autorisation macOS du dossier
Documents : la trace native se trouve dans l’accès aux métadonnées du bundle,
et le journal TCC confirme une demande d’autorisation après changement du
binaire ad hoc. Aucun réglage TCC, Trousseau ou certificat n’est modifié.
Après autorisation donnée par Koka, le launcher retrouve le pack et ses comptes.
Le compte Minecraft sélectionné ne correspond pas au pseudo du dossier : Jouer
reste désactivé, comme attendu. Koka choisit le compte du dossier existant.
Bien démarrer propose désormais **Changer de compte**, avec le même dialogue
Sanctuary par code, sans supprimer le compte déjà enregistré. L’erreur affiche
les deux pseudos pour expliquer le blocage. La nouvelle connexion confirme
le compte correspondant au dossier et sa licence/profil.

Le premier essai s’arrête avant le jeu : JavaCheck.jar manque dans le bundle.
Le script copiait l’icône mais pas les auxiliaires produits dans `build/native/jars`.
Il embarque désormais JavaCheck.jar, NewLaunch.jar et NewLaunchLegacy.jar dans
Contents/MacOS/jars et les compare aux originaux construits. Le JavaCheck ainsi
embarqué s’exécute avec Java **25.0.3 / aarch64 / Eclipse Adoptium** ; les trois
archives contiennent le point d’entrée attendu.

L’essai révèle aussi que `MinecraftInstance::createLaunchTask` remplace icon.png
par l’icône d’instance Prism, invalidant le hash du pack au lancement suivant.
Cette écriture est désormais exclue pour l’instance officielle Sanctuary.
Le fichier original embarqué est remis en place après comparaison au SHA-256
enregistré par packwiz, puis une synchronisation officielle confirme beta.373.
L’installateur réutilisait son cache et ne restaurait pas seul cette altération ;
la réparation générale de fichiers modifiés hors canal reste un point distinct.

**Recette réussie :** Jouer est actif avec le compte attendu. Le clic ouvre les
journaux dans la même fenêtre, télécharge les ressources Mojang, valide Java 25
et lance Fabric **0.19.5 / Minecraft 26.3 / Sanctuary beta.373** en ligne.
L’icône conserve son empreinte après création du processus. Le client annonce
réellement **Vulkan, MoltenVK 1.4.2**, sur Apple M1. Il rejoint directement
la Beta au port 25567, sans sélection du serveur dans le menu principal.

Le journal de la CT 350, service actif, confirme l’arrivée du compte à
**13:02:48 Europe/Paris** (12:02:48 dans le journal serveur UTC). Koka confirme
ensuite que son personnage est dans le monde. Cette confirmation visuelle est
distincte des journaux : le processus Java du jeu n’est pas exposé comme une
application capturable par l’outil natif de cette séance. Aucune capture du
jeu ni validation visuelle automatisée n’est revendiquée.

Dans le launcher, Jeu en cours et Mettre à jour sont désactivés pendant la
partie ; les journaux restent accessibles et les éditions sont suspendues.
Le client est ensuite fermé normalement à 13:04:11. L’accueil affiche de nouveau
Jouer actif, le bon profil prêt et le pack vérifié ; les outils d’édition et
Mettre à jour sont réactivés. Aucun redémarrage serveur,
changement de whitelist ou intervention sur les données du personnage n’a été
effectué pour cet essai. Les réglages complets shaders/audio, le microphone,
les autres OS et la distribution autonome restent hors de cette validation.
