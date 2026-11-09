#!/usr/bin/env bash
# Build and run only the isolated Sanctuary local-development profile.
set -euo pipefail
sanctuary_mode="${1:-run}"
case "$sanctuary_mode" in run|--verify|--logs|--debug) ;; *) echo "Usage: $0 [--verify|--logs|--debug]" >&2; exit 2 ;; esac
sanctuary_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sanctuary_profile="$sanctuary_root/build/local-launcher"
sanctuary_bundle="$sanctuary_root/build/native/sanctuary-launcher.app"

# Only terminate this binary with this exact development data directory.
python3 - "$sanctuary_profile" <<'PY'
import os, signal, subprocess, sys
profile=sys.argv[1]
for line in subprocess.check_output(['ps','-ax','-o','pid=','-o','command='],text=True).splitlines():
    fields=line.strip().split(None,1)
    if len(fields)!=2: continue
    pid,command=fields
    if '/sanctuary-launcher.app/Contents/MacOS/sanctuary-launcher' in command and ('--dir '+profile) in command:
        try: os.kill(int(pid),signal.SIGTERM)
        except ProcessLookupError: pass
PY
"$sanctuary_root/scripts/build-sanctuary-macos.sh"
python3 - "$sanctuary_profile" "$(brew --prefix openjdk@17)/bin/java" <<'PY'
from pathlib import Path
import sys
profile=Path(sys.argv[1])
profile.mkdir(parents=True,exist_ok=True)
config=profile/'sanctuary-launcher.cfg'
if not config.exists():
    config.write_text('[General]\nLanguage=fr\nApplicationTheme=system\nIconTheme=pe_colored\nSanctuaryLocalMode=true\nIgnoreJavaWizard=true\nUserAskedAboutAutomaticJavaDownload=true\nAutomaticJavaDownload=false\nAutomaticJavaSwitch=false\nJavaPath=/opt/homebrew/opt/openjdk@17/bin/java\nPastebinURL=\n'.replace('/opt/homebrew/opt/openjdk@17/bin/java',sys.argv[2]),encoding='utf-8')
# Public Entra registration, confined to this development profile.
content=config.read_text(encoding='utf-8')
lines=content.splitlines()
start=next((i+1 for i,line in enumerate(lines) if line.strip()=='[General]'),None)
if start is None:
    lines.extend(['','[General]'])
    start=len(lines)
end=next((i for i in range(start,len(lines)) if lines[i].startswith('[')),len(lines))
section=lines[start:end]
for key,value in {'MSAClientIDOverride':'8e8a2428-a727-4668-98ea-2fdaf0037943','SanctuaryMicrosoftDevTest':'true'}.items():
    section=[line for line in section if not line.startswith(key+'=')]
    section.append(key+'='+value)
config.write_text('\n'.join(lines[:start]+section+lines[end:])+'\n',encoding='utf-8')
profile.chmod(0o700)
print('Profil local isolé :',profile)
PY
if [[ "$sanctuary_mode" == --debug ]]; then
    exec lldb -- "$sanctuary_bundle/Contents/MacOS/sanctuary-launcher" --dir "$sanctuary_profile"
fi
# A synthetic scale used by offscreen tests must not magnify the real app.
env -u QT_SCALE_FACTOR /usr/bin/open -n "$sanctuary_bundle" --args --dir "$sanctuary_profile" --alive
if [[ "$sanctuary_mode" == --verify ]]; then
    python3 - "$sanctuary_profile" <<'PY'
import subprocess,sys,time
profile=sys.argv[1]
for _ in range(30):
    lines=subprocess.check_output(['ps','-ax','-o','pid=','-o','command='],text=True).splitlines()
    matches=[line.strip().split(None,1)[0] for line in lines if '/sanctuary-launcher.app/Contents/MacOS/sanctuary-launcher' in line and '--dir '+profile in line]
    if matches:
        print('Sanctuary Launcher local démarré, PID '+matches[0])
        break
    time.sleep(0.2)
else: raise SystemExit('Le processus local ne démarre pas.')
PY
elif [[ "$sanctuary_mode" == --logs ]]; then
    exec /usr/bin/log stream --info --style compact --predicate 'process == "sanctuary-launcher"'
fi
