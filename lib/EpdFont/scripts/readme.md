# Symbol-added SD fonts

These host-side tools help build `.cpfont` SD fonts with broader symbol coverage
for books that use technical or fiction symbols, such as `ℓ`, `λ`, arrows, math
operators, number forms, and dingbats.

## 1. Check the source TTF/OTF

First inspect what the source font actually contains. A `.cpfont` cannot include
glyphs that are missing from the original font.

```powershell
python lib\EpdFont\scripts\check-ttf-coverage.py D:\U\ERICK\DE\Bookerly-Regular.ttf
```

Useful output:

- `Codepoint checks` tells you whether specific symbols such as `U+2113` and
  `U+03BB` exist in the source font.
- `Symbol block coverage` shows which Unicode symbol blocks are worth adding.
- `Technical-reading interval add-on` prints an interval string that can be
  appended to `fontconvert_sdcard.py --intervals`.

Run the same check for bold and italic TTFs if the final `.cpfont` will bundle
multiple styles. A symbol has to exist in each source style if you want it
available in each rendered style.

## 2. Generate symbol-added `.cpfont` files

Use `fontconvert_sdcard.py` with the normal `reading` preset plus extra ranges
that the TTF coverage check showed are useful.

Example for three Bookerly styles:

```powershell
python lib\EpdFont\scripts\fontconvert_sdcard.py `
  --regular D:\U\ERICK\DE\Bookerly-Regular.ttf `
  --bold D:\U\ERICK\DE\Bookerly-Bold.ttf `
  --italic D:\U\ERICK\DE\Bookerly-Italic.ttf `
  --sizes 12,14,16,18 `
  --intervals "reading,(0x2100-0x214F),(0x2150-0x218F),(0x2190-0x21FF),(0x2200-0x22FF),(0x2300-0x23FF),(0x2460-0x24FF),(0x25A0-0x25FF),(0x2600-0x26FF),(0x2700-0x27BF),(0xFB00-0xFB4F)" `
  --output-dir D:\U\ERICK\DO\XTEINK4\.fonts\BookerlyV2
```

Recommended fiction/technical add-on ranges:

```text
U+2100-U+214F  Letterlike Symbols
U+2150-U+218F  Number Forms
U+2190-U+21FF  Arrows
U+2200-U+22FF  Math Operators
U+2300-U+23FF  Misc Technical
U+2460-U+24FF  Enclosed Alphanumerics
U+25A0-U+25FF  Geometric Shapes
U+2600-U+26FF  Misc Symbols
U+2700-U+27BF  Dingbats
U+FB00-U+FB4F  Alphabetic Presentation Forms
```

Avoid adding very large emoji or pictograph blocks unless the TTF coverage check
shows useful coverage and you accept larger files.

## 3. Verify the generated `.cpfont` files

Check the generated folder before copying it to the device:

```powershell
python lib\EpdFont\scripts\check-cpfont-coverage.py `
  D:\U\ERICK\DO\XTEINK4\.fonts\BookerlyV2 `
  --codepoints 0x2113,0x03BB,0x002B,0x0056,0x03C0,0x2192,0x221E,0x2260,0x00B0
```

For bundled multi-style `.cpfont` files, require every present style to include
each symbol:

```powershell
python lib\EpdFont\scripts\check-cpfont-coverage.py `
  D:\U\ERICK\DO\XTEINK4\.fonts\BookerlyV2 `
  --all-styles `
  --codepoints 0x2113,0x03BB,0x002B,0x0056,0x03C0,0x2192,0x221E,0x2260,0x00B0
```

`OK` means the codepoints are present in the `.cpfont` interval table. `MISSING`
means the font was generated without that symbol, either because the interval was
not requested or because the source TTF/OTF did not contain the glyph.

## 4. Device check

After copying the generated font folder to the SD card:

1. Select the new font family in the reader.
2. Clear the EPUB/cache if the old font output is still visible.
3. Reopen the EPUB and check the target passage.

For CSS text decorations such as strikethrough, enable embedded styles. CSS
strikethrough is drawn by the reader and does not require `U+0336 COMBINING LONG
STROKE OVERLAY` in the font.
