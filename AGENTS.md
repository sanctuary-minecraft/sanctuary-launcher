# Travailler sur sanctuary-launcher


## Base de travail depuis le 8 novembre 2026

Koka a demandé de faire de la Beta 372 la base principale de toute l'organisation
GitHub, d'importer les sources et leurs historiques, puis de conserver Gitea en
archive. Cette demande remplace la restriction historique interdisant l'import
des prototypes privés. Les anciennes bases Release restent intégralement sous
le tag `archive/release-foundation-20261108` et dans leurs branches existantes.

- Lire README.md, SOURCE_IMPORT.json et le répertoire des dépôts de sanctuary-docs.
- Conserver les dépôts privés, les identifiants Sanctuary et les versions historiques.
- Ne pas appliquer les anciennes PR Release sans réévaluer leur base et leur périmètre.
- Préserver les changements existants ; travailler dans une branche `codex/<sujet>`.
- Aucun secret, compte, session, base SQL, monde, dépendance téléchargée ou JAR
  généré dans Git. Le wrapper Gradle est l'exception prévue.
- Un import GitHub ou un push n'autorise aucun déploiement, reset de monde,
  activation d'expansion, changement d'accès ou de canal public.
- Les tests graphiques Sanctuary ciblent Vulkan uniquement.
- Les sources de Henry et des adaptateurs communautaires restent exécutées par
  leurs applications d'origine ; une extraction ne crée pas un service autonome.
- Documenter les vérifications réelles et leurs limites ; ne pas annoncer une
  compilation ou un essai en jeu à partir d'un ancien reçu.
