#!/usr/bin/env python3
"""Prepare an immutable launcher release and its stable download page files."""

import argparse
import base64
import hashlib
import json
import plistlib
import re
import shutil
import subprocess
import tempfile
import xml.etree.ElementTree as XML
import zipfile
from pathlib import Path


BASE_URL = "https://sanctuary-minecraft.net/launcher-updates/releases/"
SPARKLE = "http://www.andymatuschak.org/xml-namespaces/sparkle"
ASSETS = {
    "windows-x64": "Sanctuary-Launcher-windows-x64.exe",
    "macos-arm64": "Sanctuary-Launcher-macos-arm64.zip",
    "linux-x64": "Sanctuary-Launcher-linux-x64.AppImage",
}
XML.register_namespace("sparkle", SPARKLE)


def openssl(program, *args):
    return subprocess.check_output([program, *map(str, args)])


def encode(data):
    return base64.b64encode(data).decode("ascii")


def digest(file):
    calculator = hashlib.sha256()
    with file.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            calculator.update(block)
    return calculator.hexdigest()


def sign(program, key, file):
    raw = openssl(program, "pkeyutl", "-sign", "-rawin", "-inkey", key, "-in", file)
    if len(raw) != 64:
        raise ValueError("Expected a 64-byte Ed25519 signature")
    return encode(raw)


def appcast(version, platform, file, signature, destination):
    feed = XML.Element("rss", version="2.0")
    channel = XML.SubElement(feed, "channel")
    XML.SubElement(channel, "title").text = "Sanctuary Launcher"
    XML.SubElement(channel, "link").text = "https://sanctuary-minecraft.net/telechargements"
    XML.SubElement(channel, "description").text = "Sanctuary Launcher stable releases"
    item = XML.SubElement(channel, "item")
    XML.SubElement(item, "title").text = f"Sanctuary Launcher {version}"
    XML.SubElement(item, f"{{{SPARKLE}}}version").text = version
    XML.SubElement(item, f"{{{SPARKLE}}}shortVersionString").text = version
    attributes = {
        "url": f"{BASE_URL}{version}/{file.name}",
        "length": str(file.stat().st_size),
        "type": "application/octet-stream",
        f"{{{SPARKLE}}}edSignature": signature,
        f"{{{SPARKLE}}}version": version,
    }
    if platform == "windows-x64":
        attributes[f"{{{SPARKLE}}}os"] = "windows"
        attributes[f"{{{SPARKLE}}}installerArguments"] = "/S /Update"
    else:
        XML.SubElement(item, f"{{{SPARKLE}}}minimumSystemVersion").text = "26.0"
    XML.SubElement(item, "enclosure", attributes)
    XML.ElementTree(feed).write(destination, encoding="utf-8", xml_declaration=True)


def stage(args):
    if not re.fullmatch(r"(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)", args.version):
        raise ValueError("Expected a release version such as 0.1.0")
    key_der = openssl(args.openssl, "pkey", "-in", args.key, "-pubout", "-outform", "DER")
    if len(key_der) != 44 or key_der[:12].hex() != "302a300506032b6570032100":
        raise ValueError("Expected an Ed25519 signing key")
    public_key = encode(key_der[12:])
    with zipfile.ZipFile(args.macos) as archive:
        result = plistlib.loads(archive.read("sanctuary-launcher.app/Contents/Info.plist"))
        if result.get("CFBundleVersion") != args.version or result.get("SUPublicEDKey") != public_key:
            raise ValueError("The Mac archive version and Sparkle public key must match the release")
    with args.windows.open("rb") as executable:
        if executable.read(2) != b"MZ":
            raise ValueError("Expected a Windows installer")
    with args.linux.open("rb") as image:
        header = image.read(11)
        if header[:4] != b"\x7fELF" or header[8:11] != b"AI\x02":
            raise ValueError("Expected a type 2 Linux AppImage")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.output.exists():
        raise FileExistsError(args.output)
    with tempfile.TemporaryDirectory(dir=args.output.parent) as temp:
        root = Path(temp) / "staged"
        release = root / "launcher-releases" / args.version
        channel = root / "launcher-channels" / args.version
        release.mkdir(parents=True)
        channel.mkdir(parents=True)
        sources = release / "Sanctuary-Launcher-sources.tar.gz"
        shutil.copyfile(args.sources, sources)
        summary = {"version": args.version, "public_key": public_key, "downloads": {}}
        repository = Path(__file__).resolve().parent.parent

        inputs = {"windows-x64": args.windows, "macos-arm64": args.macos, "linux-x64": args.linux}
        for platform, source in inputs.items():
            payload = release / ASSETS[platform]
            shutil.copyfile(source, payload)
            install_zip = channel / f"Sanctuary-Launcher-{platform}.zip"
            if platform == "linux-x64":
                payload.chmod(0o755)
                manifest = {"schema": 1, "platform": platform, "version": args.version,
                            "size": payload.stat().st_size, "sha256": digest(payload)}
                message_file = Path(temp) / "manifest"
                message_file.write_text(json.dumps(manifest, separators=(",", ":")), encoding="utf-8")
                envelope = {"payload": encode(message_file.read_bytes()),
                            "signature": sign(args.openssl, args.key, message_file)}
                (channel / "linux-x64.json").write_text(json.dumps(envelope), encoding="utf-8")
            else:
                appcast(args.version, platform, payload, sign(args.openssl, args.key, payload), channel / f"{platform}.xml")

            if platform == "macos-arm64":
                shutil.copyfile(payload, install_zip)
            else:
                with zipfile.ZipFile(install_zip, "w", compression=zipfile.ZIP_DEFLATED) as archive:
                    archive.write(payload, payload.name)
                    for name in ["LICENSE", "COPYING.md"]:
                        archive.write(repository / name, name)
            summary["downloads"][platform] = {"sha256": digest(install_zip),
                                               "signature": sign(args.openssl, args.key, install_zip)}

        summary["sources"] = {"sha256": digest(sources), "signature": sign(args.openssl, args.key, sources)}
        (channel / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
        root.rename(args.output)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", required=True)
    for name in ["macos", "linux", "windows", "sources", "key", "output"]:
        parser.add_argument(f"--{name}", required=True, type=Path)
    parser.add_argument("--openssl", default="openssl")
    args = parser.parse_args()
    stage(args)


if __name__ == "__main__":
    main()
