#!/usr/bin/env python3
"""Compress web assets from www_work and move .gz files into data/www."""

from __future__ import annotations

import gzip
import shutil
from pathlib import Path


SKIP_EXTENSIONS = {
    ".gz",
    ".png",
    ".jpg",
    ".jpeg",
    ".gif",
    ".webp",
    ".bmp",
    ".ico",
    ".woff",
    ".woff2",
    ".ttf",
    ".eot",
}

COMPRESSED_ONLY_FILENAMES = {
    "bt.html",
    "player.html",
}


def should_compress(path: Path, script_name: str) -> bool:
    if not path.is_file():
        return False
    if path.name == script_name:
        return False
    if path.name.startswith("."):
        return False
    if path.suffix.lower() in SKIP_EXTENSIONS:
        return False
    return True


def sync_plain_and_compressed(src_file: Path, dst_dir: Path) -> tuple[Path, Path]:
    dst_plain = dst_dir / src_file.name
    if src_file.name in COMPRESSED_ONLY_FILENAMES:
        if dst_plain.exists():
            dst_plain.unlink()
    else:
        shutil.copy2(src_file, dst_plain)

    gz_local = src_file.with_suffix(src_file.suffix + ".gz")
    dst_file = dst_dir / gz_local.name

    with src_file.open("rb") as f_in, gzip.open(gz_local, "wb", compresslevel=9) as f_out:
        shutil.copyfileobj(f_in, f_out)

    if dst_file.exists():
        dst_file.unlink()

    shutil.move(str(gz_local), str(dst_file))
    return dst_plain, dst_file


def main() -> int:
    src_dir = Path(__file__).resolve().parent
    dst_dir = src_dir.parent / "data" / "www"
    dst_dir.mkdir(parents=True, exist_ok=True)

    script_name = Path(__file__).name
    candidates = sorted(p for p in src_dir.iterdir() if should_compress(p, script_name))

    if not candidates:
        print("No files to compress in www_work.")
        return 0

    print(f"Source: {src_dir}")
    print(f"Target: {dst_dir}\n")

    moved = 0
    for src_file in candidates:
        dst_plain, dst_file = sync_plain_and_compressed(src_file, dst_dir)
        if src_file.name in COMPRESSED_ONLY_FILENAMES:
            print(f"OK  {src_file.name} -> {dst_file.name} (gz only)")
        else:
            print(f"OK  {src_file.name} -> {dst_plain.name}, {dst_file.name}")
        moved += 1

    print(f"\nDone. Synced {moved} files to {dst_dir} (plain + gzip)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
