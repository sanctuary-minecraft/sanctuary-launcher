# Connexion Microsoft en développement — 9 novembre 2026

Ticket LAUNCH-23, branche `codex/launcher-microsoft-dev`.

## Résultat disponible

Bien démarrer propose **Tester Microsoft · dev** dans le profil local Run.
Le script configure l’AppID public Sanctuary dans le seul dossier
`build/local-launcher`. La valeur compilée par défaut reste vide et le drapeau
de test vaut false dans les autres profils. Aucun secret client ni changement
Entra n’est nécessaire pour ce parcours par code.

Le dialogue réutilise `MinecraftAccount::login(true)` et la chaîne DeviceCode
Prism : Microsoft, Xbox, services Minecraft, licence, profil et skin. Un seul
parcours est lancé ; le flux navigateur nécessitant une URI de redirection
n’est pas démarré en parallèle. Les textes sont affichés comme texte simple.
Le lien ouvert sur clic est limité aux adresses Microsoft de vérification
attendues. Le code de connexion reste temporaire et n’est pas documenté.

Le statut distingue Microsoft validé d’un accès Minecraft non confirmé. Un
compte n’est ajouté à la liste native Prism que si la chaîne réussit avec une
licence et un profil confirmés. Le stockage natif des comptes reste privé au
propriétaire et le dossier de développement est limité à celui-ci. Annuler
interrompt la tâche ; un échec reste affiché et ne coche pas l’étape Microsoft.
Jouer reste désactivé dans ce lot, même en cas de succès complet.

Aucun changement du Trousseau Discord, de la signature ad hoc, de l’admission,
du pack, du serveur ou du monde. Les nouvelles identités de signature évoquées
précédemment ne sont pas créées, conformément au choix de Koka.

## Vérifications

25 suites CTest réussissent après le premier raccordement, dont le nouveau
contrôle de l’UI : le bouton demande une authentification réelle, ne modifie
pas les scénarios locaux et ne débloque pas Jouer. Le passage Microsoft non
confirmé / compte vérifié / test désactivé respecte les états attendus.

Recette native : le service Microsoft accepte la demande de code avec
l’AppID Sanctuary. Code reçu, ouverture du dialogue et annulation vérifiés ;
aucun compte ajouté. Un défaut de hauteur du libellé du code a été repéré et
corrigé avant la recette utilisateur.

Un essai réel confirme ensuite Microsoft, l’authentification Xbox utilisateur,
puis un refus XSTS `2148916233` : le compte utilisé n’a pas de profil Xbox Live.
Koka confirme que ce compte ne possède pas Minecraft. Le parcours n’a donc
pas atteint la demande de session Minecraft, la licence ou le profil ; aucun
compte n’a été ajouté. Ce résultat ne permet pas de conclure à une approbation
ou à un refus AppID par Mojang.

Safari reprenait automatiquement le compte déjà connecté. Une fenêtre privée
Microsoft est ouverte pour essayer le compte qui possède déjà Minecraft, sans
déconnecter le compte d’administration Azure. La validation avec ce compte
reste à constater. Aucun identifiant personnel, code, jeton ou capture de
connexion n’est versionné.

Le dialogue explique désormais cette possibilité de choisir un autre compte.
Si le moteur Prism renouvelle un code, l’indication est explicite et les boutons
utilisent uniquement le code et le lien courants, sans ajouter de connexions
supplémentaires. L’UI suit le compte par défaut vérifié, lié au Client ID du
profil de test. Nouvelle construction et 25 suites réussies après ces ajustements,
sans relancer le processus pendant l’authentification interactive en cours.

Référence protocole : [Microsoft — flux de code de périphérique](https://learn.microsoft.com/en-us/entra/identity-platform/v2-oauth2-device-code).
