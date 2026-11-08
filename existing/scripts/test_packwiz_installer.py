"""Verify that branding changes only the upstream window and survives Prism export."""
import configparser
import io
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest.mock import patch

import pack
import packwiz_installer as installer
import publish_web_pack as web


class InstallerDistributionTests(unittest.TestCase):
    def test_upstream_engine_and_notices_are_preserved_byte_for_byte(self):
        with zipfile.ZipFile(installer.OUTPUT.parent / 'packwiz-installer-upstream.jar') as upstream, \
                zipfile.ZipFile(installer.OUTPUT) as branded:
            self.assertIsNone(branded.testzip())
            self.assertEqual(branded.namelist()[0], 'META-INF/MANIFEST.MF')
            changed = {name for name in upstream.namelist() if not name.endswith('/')
                       and upstream.read(name) != branded.read(name)}
            self.assertEqual(changed, {installer.WINDOW})
            self.assertEqual(set(branded.namelist()) - set(upstream.namelist()), {
                installer.LOGO, 'META-INF/sanctuary-packwiz/LICENSE',
                'META-INF/sanctuary-packwiz/InstallWindow.java'})
            self.assertEqual(branded.read(installer.LOGO), (installer.SOURCE / 'sanctuary.png').read_bytes())

    def test_export_embeds_verified_installer_and_preserves_stable_channel(self):
        channel = 'https://git.botsu.net/koka/sanctuary-beta/raw/branch/packwiz/pack.toml'
        # Use a fixture for the network fetch; the installer itself is the real built JAR.
        fake_bootstrap = b'verified-bootstrap-fixture'
        source = pack.ROOT
        values, _ = pack.check()
        with tempfile.TemporaryDirectory() as directory, \
                patch.object(pack, 'ROOT', Path(directory)), \
                patch.object(pack, 'check', return_value=(values, [])), \
                patch.object(pack, 'urlopen', return_value=io.BytesIO(fake_bootstrap)), \
                patch.object(pack, 'BOOTSTRAP_SHA256', installer.digest(fake_bootstrap)), \
                patch.object(web, 'BOOTSTRAP_SHA256', installer.digest(fake_bootstrap)), \
                patch.object(installer, 'build', return_value=installer.OUTPUT.read_bytes()):
            pack.prism(channel)
            exported = Path(directory) / 'build' / ('Sanctuary-Prism-' + values['pack_version'] + '.zip')
            data = exported.read_bytes()
            client = {'icon_sha256': pack.sha256(source / 'packwiz/icon.png'),
                      'manifest': pack.read_toml(source / 'packwiz/pack.toml')}
            web.verify_prism(data, client, channel)
            with zipfile.ZipFile(io.BytesIO(data)) as archive:
                config = configparser.ConfigParser(interpolation=None)
                config.read_string(archive.read('instance.cfg').decode())
                self.assertIn('--bootstrap-no-update', config['General']['PreLaunchCommand'])
                self.assertEqual(archive.read('minecraft/packwiz-installer.jar'), installer.OUTPUT.read_bytes())


if __name__ == '__main__':
    unittest.main()
