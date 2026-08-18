#include "theme.h"

UiTheme current_theme = THEME_BLUE;

// One palette per UiTheme value, same order as the enum, so get_theme_colors()
// can index straight into this array.
static const ThemeColors THEME_PALETTES[] = {
  // THEME_BLUE (original/default)
  { 0x4c8cb9, 0xa6cdec, 0x5e9bc8, 0xFFFFFF, 0xe4ffff, 0xb9ecff },
  // THEME_RED
  { 0xb9524c, 0xecb2a6, 0xc8705e, 0xFFFFFF, 0xffe8e4, 0xffd0b9 },
  // THEME_GREEN
  { 0x4cb95e, 0xa6ecb2, 0x5ec880, 0xFFFFFF, 0xe4ffe8, 0xb9ffd0 },
  // THEME_DARK
  { 0x2b2f38, 0x454b58, 0x3a3f4b, 0xFFFFFF, 0xd8dee9, 0xaab2c0 },
};

const ThemeColors &get_theme_colors() {
  int idx = (int)current_theme;
  if (idx < 0 || idx >= (int)(sizeof(THEME_PALETTES) / sizeof(THEME_PALETTES[0]))) {
    idx = THEME_BLUE;
  }
  return THEME_PALETTES[idx];
}
