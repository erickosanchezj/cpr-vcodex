#!/usr/bin/env python3
"""Check Unicode codepoint coverage in .cpfont files.

This validates the coverage that the device will see after SD-card font
conversion. It reads the v4 .cpfont header and per-style interval tables, then
checks whether requested Unicode codepoints are present in every bundled style.
"""

from __future__ import annotations

import argparse
import struct
import sys
import unicodedata
from pathlib import Path

from cpfont_version import CPFONT_VERSION

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="backslashreplace")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8", errors="backslashreplace")


MAGIC = b"CPFONT\x00\x00"
HEADER_FORMAT = "<8sHHB19s"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
STYLE_TOC_FORMAT = "<B3xIIBhhHHBBBI4x"
STYLE_TOC_SIZE = struct.calcsize(STYLE_TOC_FORMAT)
INTERVAL_FORMAT = "<III"
INTERVAL_SIZE = struct.calcsize(INTERVAL_FORMAT)

DEFAULT_CODEPOINTS = [
    0x2113,  # SCRIPT SMALL L
    0x03BB,  # GREEK SMALL LETTER LAMDA
    0x002B,  # PLUS SIGN
    0x0056,  # LATIN CAPITAL LETTER V
    0x03C0,  # GREEK SMALL LETTER PI
    0x2192,  # RIGHTWARDS ARROW
    0x221E,  # INFINITY
    0x2260,  # NOT EQUAL TO
    0x00B0,  # DEGREE SIGN
]

STYLE_NAMES = {
    0: "regular",
    1: "bold",
    2: "italic",
    3: "bold-italic",
}


def parse_codepoint(value: str) -> int:
    text = value.strip()
    if not text:
        raise argparse.ArgumentTypeError("empty codepoint")
    if text.lower().startswith("u+"):
        return int(text[2:], 16)
    if text.lower().startswith("0x"):
        return int(text, 16)
    if len(text) == 1:
        return ord(text)
    return int(text, 16)


def parse_codepoint_list(value: str) -> list[int]:
    return [parse_codepoint(part) for part in value.split(",") if part.strip()]


def cp_label(cp: int) -> str:
    char = chr(cp)
    name = unicodedata.name(char, "<unknown>")
    display = "\u25cc" + char if unicodedata.combining(char) else char
    return f"U+{cp:04X} {display} {name}"


def read_exact(file, size: int, context: str) -> bytes:
    data = file.read(size)
    if len(data) != size:
        raise ValueError(f"truncated file while reading {context}")
    return data


def read_cpfont_intervals(path: Path) -> dict[int, list[tuple[int, int]]]:
    with path.open("rb") as file:
        magic, version, _flags, style_count, _reserved = struct.unpack(
            HEADER_FORMAT, read_exact(file, HEADER_SIZE, "global header")
        )
        if magic != MAGIC:
            raise ValueError("invalid .cpfont magic")
        if version != CPFONT_VERSION:
            raise ValueError(f"unsupported .cpfont version {version}; expected {CPFONT_VERSION}")
        if style_count == 0 or style_count > 4:
            raise ValueError(f"invalid style count {style_count}")

        toc_entries = []
        for _ in range(style_count):
            toc = struct.unpack(STYLE_TOC_FORMAT, read_exact(file, STYLE_TOC_SIZE, "style TOC"))
            (
                style_id,
                interval_count,
                _glyph_count,
                _advance_y,
                _ascender,
                _descender,
                _kern_left_count,
                _kern_right_count,
                _kern_left_classes,
                _kern_right_classes,
                _ligature_count,
                data_offset,
            ) = toc
            toc_entries.append((style_id, interval_count, data_offset))

        intervals_by_style: dict[int, list[tuple[int, int]]] = {}
        for style_id, interval_count, data_offset in toc_entries:
            file.seek(data_offset)
            intervals = []
            for _ in range(interval_count):
                first, last, _offset = struct.unpack(
                    INTERVAL_FORMAT, read_exact(file, INTERVAL_SIZE, "interval")
                )
                intervals.append((first, last))
            intervals_by_style[style_id] = intervals

    return intervals_by_style


def interval_contains(intervals: list[tuple[int, int]], cp: int) -> bool:
    lo = 0
    hi = len(intervals)
    while lo < hi:
        mid = (lo + hi) // 2
        first, last = intervals[mid]
        if cp < first:
            hi = mid
        elif cp > last:
            lo = mid + 1
        else:
            return True
    return False


def iter_cpfont_files(paths: list[Path]) -> list[Path]:
    files: list[Path] = []
    for path in paths:
        if path.is_dir():
            files.extend(sorted(path.rglob("*.cpfont")))
        elif path.suffix.lower() == ".cpfont":
            files.append(path)
        else:
            print(f"Skipping non-.cpfont path: {path}", file=sys.stderr)
    return files


def main() -> int:
    parser = argparse.ArgumentParser(description="Check Unicode coverage in .cpfont files.")
    parser.add_argument("paths", nargs="+", type=Path, help=".cpfont files or folders to scan")
    parser.add_argument(
        "--codepoints",
        type=parse_codepoint_list,
        default=DEFAULT_CODEPOINTS,
        help="Comma-separated codepoints such as 0x2113,U+03BB,2192,π",
    )
    parser.add_argument(
        "--all-styles",
        action="store_true",
        help="Require every bundled style to include each codepoint, not just one style.",
    )
    args = parser.parse_args()

    files = iter_cpfont_files(args.paths)
    if not files:
        print("No .cpfont files found.", file=sys.stderr)
        return 2

    had_missing = False
    for path in files:
        try:
            intervals_by_style = read_cpfont_intervals(path)
        except (OSError, ValueError, struct.error) as exc:
            had_missing = True
            print(f"ERROR: {path}")
            print(f"  - {exc}")
            continue

        missing: list[str] = []
        for cp in args.codepoints:
            present_styles = [
                style_id
                for style_id, intervals in intervals_by_style.items()
                if interval_contains(intervals, cp)
            ]
            if args.all_styles:
                missing_styles = sorted(set(intervals_by_style) - set(present_styles))
                if missing_styles:
                    style_list = ", ".join(STYLE_NAMES.get(s, str(s)) for s in missing_styles)
                    missing.append(f"{cp_label(cp)} missing from {style_list}")
            elif not present_styles:
                missing.append(cp_label(cp))

        if missing:
            had_missing = True
            print(f"MISSING: {path}")
            for item in missing:
                print(f"  - {item}")
        else:
            style_list = ", ".join(STYLE_NAMES.get(s, str(s)) for s in sorted(intervals_by_style))
            print(f"OK: {path} ({style_list})")

    return 1 if had_missing else 0


if __name__ == "__main__":
    raise SystemExit(main())
