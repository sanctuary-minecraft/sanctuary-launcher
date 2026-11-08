#!/usr/bin/env python3
"""Build the branded packwiz window over an immutable upstream installer.

Only InstallWindow.class is replaced. Downloads, hashes, optional mods, error
handling and cancellation remain those of the official v0.5.14 installer.
Generated JARs and the downloaded dependency stay in ignored build/.
"""
import hashlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import zipfile
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'launcher/packwiz'
OUTPUT = ROOT / 'build/packwiz-branding/packwiz-installer-sanctuary.jar'
WINDOW = 'link/infra/packwiz/installer/ui/gui/InstallWindow.class'
LOGO = 'sanctuary/packwiz/logo.png'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def require(condition, message):
    if not condition:
        raise SystemExit(message)


def build():
    lock = json.loads((SOURCE / 'installer.json').read_text())
    logo = (SOURCE / 'sanctuary.png').read_bytes()
    require(digest(logo) == lock['logo_sha256'], 'Packwiz logo hash mismatch')
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    upstream = OUTPUT.parent / 'packwiz-installer-upstream.jar'
    if not upstream.is_file():
        with urlopen(lock['upstream_url'], timeout=45) as response:
            data = response.read()
        require(digest(data) == lock['upstream_sha256'], 'Upstream packwiz hash mismatch')
        upstream.write_bytes(data)
    require(digest(upstream.read_bytes()) == lock['upstream_sha256'],
            'Cached upstream packwiz hash mismatch')
    classes = OUTPUT.parent / 'classes'
    if classes.exists():
        shutil.rmtree(classes)
    classes.mkdir()
    subprocess.run(['javac', '--release', '8', '-Xlint:-options', '-encoding', 'UTF-8', '-g:none',
                    '-classpath', str(upstream), '-d', str(classes),
                    str(SOURCE / 'InstallWindow.java')], check=True)
    with zipfile.ZipFile(upstream) as archive:
        entries = {name: archive.read(name) for name in archive.namelist()
                   if not name.endswith('/')}
    require(WINDOW in entries, 'Upstream InstallWindow moved; review the adapter')
    entries[WINDOW] = (classes / WINDOW).read_bytes()
    entries[LOGO] = logo
    for name in ('InstallWindow.java', 'LICENSE'):
        entries['META-INF/sanctuary-packwiz/' + name] = (SOURCE / name).read_bytes()
    output = io.BytesIO()
    with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
        # The bootstrap uses JarInputStream; keep the manifest first.
        names = ['META-INF/MANIFEST.MF'] + sorted(set(entries) - {'META-INF/MANIFEST.MF'})
        for name in names:
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            archive.writestr(info, entries[name])
    data = output.getvalue()
    require(digest(data) == lock['installer_sha256'],
            'Branded installer differs from lock: ' + digest(data))
    OUTPUT.write_bytes(data)
    print('Sanctuary packwiz installer verified: ' + str(OUTPUT))
    return data


if __name__ == '__main__':
    build()
