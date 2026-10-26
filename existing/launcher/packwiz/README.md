# Fenêtre packwiz Sanctuary

`sanctuary.png` est le bandeau fourni par le créateur, conservé sans modification.
`InstallWindow.java` adapte la seule fenêtre de progression de packwiz-installer
v0.5.14, sous MIT. Le moteur officiel et ses autres interfaces restent intacts.
Le contrat de distribution est dans [le ticket beta.291](../../docs/packwiz-logo-beta291.md).

Construire avec le JDK 25 utilisé par le dépôt :

```sh
python3.14 scripts/packwiz_installer.py
python3.14 scripts/test_packwiz_installer.py
```

Le premier appel télécharge l'amont vérifié dans `build/packwiz-branding/`.
Le JAR Sanctuary généré doit correspondre à `installer.json` ; ce verrou et
`INSTALLER_SHA256` de `scripts/publish_web_pack.py` évoluent ensemble après revue
et vérification de la nouvelle fenêtre. Ne pas remplacer le moteur ni désactiver
le contrôle de hash pour faire passer une compilation différente.

Le parcours Swing réel FR/EN se vérifie sur un poste graphique, sans ouvrir
Minecraft :

```sh
javac -encoding UTF-8 -classpath build/packwiz-branding/packwiz-installer-sanctuary.jar \
  -d build/packwiz-branding/check launcher/packwiz/InstallWindowCheck.java
java -Duser.language=fr -classpath build/packwiz-branding/check:build/packwiz-branding/packwiz-installer-sanctuary.jar \
  InstallWindowCheck build/packwiz-branding/preview-fr.png
java -Duser.language=en -classpath build/packwiz-branding/check:build/packwiz-branding/packwiz-installer-sanctuary.jar \
  InstallWindowCheck build/packwiz-branding/preview-en.png
```

Le séparateur de classpath est `;` sous Windows. Le parcours contrôle les
callbacks du vrai `GUIHandler`, la progression et le compte à rebours ; il
capture la fenêtre et la ferme. Les captures ne font pas partie du JAR livré.
