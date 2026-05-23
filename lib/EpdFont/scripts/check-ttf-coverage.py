#!/usr/bin/env python3
"""Inspect Unicode coverage in a TTF/OTF before generating .cpfont files.

This is a host-side helper for deciding which fontconvert_sdcard.py intervals
are worth enabling for a source font. It reads the font's Unicode cmap and
reports coverage for symbol-heavy blocks used by technical and fiction EPUBs.
"""

from __future__ import annotations

import argparse
import sys
import unicodedata
from pathlib import Path

from fontTools.ttLib import TTFont


SYMBOL_BLOCKS = {
    "Combining Diacritics": (0x0300, 0x036F),
    "Greek": (0x0370, 0x03FF),
    "Currency Symbols": (0x20A0, 0x20CF),
    "Combining Marks for Symbols": (0x20D0, 0x20FF),
    "Letterlike Symbols": (0x2100, 0x214F),
    "Number Forms": (0x2150, 0x218F),
    "Arrows": (0x2190, 0x21FF),
    "Math Operators": (0x2200, 0x22FF),
    "Misc Technical": (0x2300, 0x23FF),
    "Control Pictures": (0x2400, 0x243F),
    "OCR": (0x2440, 0x245F),
    "Enclosed Alphanumerics": (0x2460, 0x24FF),
    "Box Drawing": (0x2500, 0x257F),
    "Block Elements": (0x2580, 0x259F),
    "Geometric Shapes": (0x25A0, 0x25FF),
    "Misc Symbols": (0x2600, 0x26FF),
    "Dingbats": (0x2700, 0x27BF),
    "Misc Math Symbols-A": (0x27C0, 0x27EF),
    "Supplemental Arrows-A": (0x27F0, 0x27FF),
    "Braille Patterns": (0x2800, 0x28FF),
    "Supplemental Arrows-B": (0x2900, 0x297F),
    "Misc Math Symbols-B": (0x2980, 0x29FF),
    "Supplemental Math Operators": (0x2A00, 0x2AFF),
    "Misc Symbols and Arrows": (0x2B00, 0x2BFF),
    "Supplemental Punctuation": (0x2E00, 0x2E7F),
    "Yijing Hexagram Symbols": (0x4DC0, 0x4DFF),
    "Alphabetic Presentation Forms": (0xFB00, 0xFB4F),
    "Halfwidth and Fullwidth Forms": (0xFF00, 0xFFEF),
    "Mathematical Alphanumeric Symbols": (0x1D400, 0x1D7FF),
    "Mahjong Tiles": (0x1F000, 0x1F02F),
    "Domino Tiles": (0x1F030, 0x1F09F),
    "Playing Cards": (0x1F0A0, 0x1F0FF),
    "Enclosed Alphanumeric Supplement": (0x1F100, 0x1F1FF),
    "Misc Symbols and Pictographs": (0x1F300, 0x1F5FF),
    "Emoticons": (0x1F600, 0x1F64F),
    "Transport and Map Symbols": (0x1F680, 0x1F6FF),
    "Alchemical Symbols": (0x1F700, 0x1F77F),
    "Geometric Shapes Extended": (0x1F780, 0x1F7FF),
    "Supplemental Arrows-C": (0x1F800, 0x1F8FF),
    "Supplemental Symbols and Pictographs": (0x1F900, 0x1F9FF),
    "Chess Symbols": (0x1FA00, 0x1FA6F),
    "Symbols and Pictographs Extended-A": (0x1FA70, 0x1FAFF),
}


DEFAULT_CODEPOINTS = {
    0x2113: "SCRIPT SMALL L",
    0x03BB: "GREEK SMALL LETTER LAMDA",
    0x0336: "COMBINING LONG STROKE OVERLAY",
    0x0056: "LATIN CAPITAL LETTER V",
    0x002B: "PLUS SIGN",
    0x2192: "RIGHTWARDS ARROW",
    0x2260: "NOT EQUAL TO",
    0x221E: "INFINITY",
    0x00B0: "DEGREE SIGN",
    0x03C0: "GREEK SMALL LETTER PI",
}


TECHNICAL_READING_BLOCKS = [
    "Combining Marks for Symbols",
    "Letterlike Symbols",
    "Misc Technical",
    "Misc Math Symbols-A",
    "Supplemental Arrows-A",
    "Supplemental Arrows-B",
    "Misc Math Symbols-B",
    "Supplemental Math Operators",
    "Misc Symbols and Arrows",
    "Mathematical Alphanumeric Symbols",
]


def parse_codepoint(value: str) -> int:
    text = value.strip()
    if text.lower().startswith("u+"):
        return int(text[2:], 16)
    if text.lower().startswith("0x"):
        return int(text[2:], 16)
    if len(text) == 1:
        return ord(text)
    return int(text, 16)


def load_cmap(path: Path) -> dict[int, str]:
    font = TTFont(path)
    return font.getBestCmap() or {}


def coverage_for_block(cmap: dict[int, str], start: int, end: int) -> tuple[int, int, float]:
    total = end - start + 1
    covered = sum(1 for cp in range(start, end + 1) if cp in cmap)
    percent = (covered * 100.0 / total) if total else 0.0
    return covered, total, percent


def print_codepoint_checks(cmap: dict[int, str], codepoints: list[int]) -> None:
    print("\nCodepoint checks:")
    for cp in codepoints:
        char = chr(cp)
        display = f"\u25CC{char}" if unicodedata.combining(char) else char
        label = DEFAULT_CODEPOINTS.get(cp, unicodedata.name(char, "UNKNOWN"))
        glyph = cmap.get(cp)
        status = "YES" if glyph else "NO "
        suffix = f" -> {glyph}" if glyph else ""
        print(f"  U+{cp:04X} {display} {label}: {status}{suffix}")


def print_block_summary(cmap: dict[int, str], threshold: float) -> list[tuple[str, int, int]]:
    print("\nSymbol block coverage:")
    suggested: list[tuple[str, int, int]] = []
    for name, (start, end) in SYMBOL_BLOCKS.items():
        covered, total, percent = coverage_for_block(cmap, start, end)
        if covered and percent >= threshold:
            suggested.append((name, start, end))
        print(f"  {name:40s} U+{start:04X}-U+{end:04X}: {covered:4d}/{total:<4d} {percent:6.1f}%")
    return suggested


def print_interval_suggestions(cmap: dict[int, str], threshold: float) -> None:
    chosen = []
    for name, start, end in print_block_summary(cmap, threshold):
        if name in TECHNICAL_READING_BLOCKS:
            chosen.append((start, end))

    if not chosen:
        return

    intervals = ",".join(f"(0x{start:04X}-0x{end:04X})" for start, end in chosen)
    print("\nTechnical-reading interval add-on:")
    print(f'  --intervals "reading,{intervals}"')


def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="backslashreplace")

    parser = argparse.ArgumentParser(description="Check Unicode coverage in a TTF/OTF before cpfont conversion.")
    parser.add_argument("font", type=Path, help="Path to a .ttf or .otf font file")
    parser.add_argument(
        "--codepoint",
        action="append",
        default=[],
        help="Extra codepoint to check, e.g. U+2113, 0x2113, 2113, or a literal character",
    )
    parser.add_argument(
        "--suggest-threshold",
        type=float,
        default=10.0,
        help="Minimum percent coverage for a block to appear in interval suggestions",
    )
    parser.add_argument("--no-blocks", action="store_true", help="Only print codepoint checks")
    args = parser.parse_args()

    cmap = load_cmap(args.font)
    print(f"Font: {args.font}")
    print(f"Mapped Unicode codepoints: {len(cmap)}")

    codepoints = list(DEFAULT_CODEPOINTS)
    codepoints.extend(parse_codepoint(cp) for cp in args.codepoint)
    codepoints = sorted(set(codepoints))
    print_codepoint_checks(cmap, codepoints)

    if not args.no_blocks:
        print_interval_suggestions(cmap, args.suggest_threshold)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
