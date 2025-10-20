# Asset Reverse Engineering Plan

## Problem Identified
The Gravity Strike program fails to load its data files because:

1. The Allegro 5 wrapper implements stubbed functions for data loading:
   ```cpp
   DATAFILE *load_datafile(const char *filename) {
       (void)filename;
       // Not implemented - need a real implementation based on Allegro 5 data system
       return nullptr;  // <- This is why all data files fail to load!
   }
   ```

2. The data files are in Allegro 4 format, but the system only has Allegro 5 libraries installed.

## What Asset Files We Have Available

### Data Files (in `/home/sblo/GravityForce/dat/`):
1. `gsmenu.dat` - Menu assets (weapons, crosshairs, ships, title, palettes)
2. `gs.dat` - Game assets (likely maps, game objects, etc.)
3. `ships.dat` - Ship sprites and definitions
4. `fonts.dat` - Font files
5. `gssnd.dat` - Sound samples
6. Plus others: crc.dat, fonts.dat, keys.dat, ships.dat, text.lua

### Asset Indexes (in `/home/sblo/GravityForce/src/`):
1. `gsmd.h` - Defines index numbers for menu data file (weapon images, crosshairs, etc.)
2. `fonts.h` - Defines font asset indexes
3. `gssndat.h` - Defines sound asset indexes

### Raw Source Assets:
1. Individual ship sprites in `/home/sblo/GravityForce/ships/` (many .tga/.shp files)
2. Level assets in `/home/sblo/GravityForce/levels/mission/*/frapela/` (many .tga/.bmp files)
3. Map files ( .mar extension - Allegro 4 maps)
4. Lua scripts for game logic
5. Configuration files (.ini, .cfg, .sc)

### Extensive Asset Collection:
- **Ships**: Hundreds of ship sprite files (.tga/.shp) in the ships/ directory
- **Levels**: Multiple level directories with asset images (.tga/.bmp) in levels/mission/*/
- **Sounds**: Many .wav files scattered throughout the project
- **Maps**: .mar map files for level layouts
- **Fonts**: Likely embedded in the data files but potentially available as system fonts
- **Graphics**: Thousands of individual image files for game elements

## Current Progress

### ✅ Implemented Direct Asset Loading Framework
1. **Added compile flag** `-DDIRECT_ASSET_LOADING` to Makefile
2. **Modified build.sh** to indicate direct asset loading is enabled by default
3. **Updated allegro5_wrapper.cpp** to log when direct asset loading is attempted
4. **Enhanced debugging output** to show exactly which files are being loaded

### 🔄 Current Status
When running with `-DDIRECT_ASSET_LOADING`, we can see:
```
DIRECT_ASSET_LOADING: Attempting to load './dat/gsmenu.dat' as direct asset file
DIRECT_ASSET_LOADING: File basename is 'gsmenu.dat'
```

This confirms our framework is working, but we're still returning `nullptr` from `load_datafile()`.

## Reverse Engineering Strategy

We can solve this problem by implementing one of these approaches:

### Option 1: Implement Proper Data File Loading (Recommended)
Create a compile flag that bypasses the Allegro 4 data file system entirely:

1. Add a compile flag (e.g., `-DLOAD_FROM_SOURCE_FILES` or `-DDIRECT_ASSET_LOADING`)
2. Modify the code to load assets directly:
   - Load images from .tga/.bmp/.png files instead of datafiles
   - Load fonts from font files directly
   - Load sounds from individual .wav/.ogg files
   - Load configuration from .ini/.cfg files directly

### Option 2: Fix the Allegro 5 Wrapper
1. Implement proper `load_datafile()` function that can read Allegro 4 format files
2. Parse the binary format of Allegro 4 data files
3. Convert to Allegro 5 equivalents at runtime

### Option 3: Recreate Data Files
1. Install Allegro 4 and build the original tools
2. Use the original Allegro 4 grabber tool to recreate data files
3. Or write custom tools to recreate the data files

## Immediate Action Items

1. **Add compile flag support** to bypass data files and load from source files directly
2. **Identify all asset loading points** in the code that currently use datafiles
3. **Create asset loading functions** that can load directly from individual files
4. **Test incremental loading** to prove the approach works

## Asset Mapping

From our investigation, we know what's in each data file:

### gsmenu.dat (Menu Assets)
From `gsmd.h`:
- Crosshairs (crosshair, crosshair2, etc.) - indices 0-4
- Palette (menupal) - index 5
- Ship images (ship1_1, ship2_1, etc.) - indices 6-11
- Title graphic (title) - index 12
- Weapon images (w_acid through w_shrinker) - indices 13-26

### fonts.dat (Fonts)
From `fonts.h`:
- Multiple fonts with different styles (font_bank through font_verdana) - indices 0-9

### gssnd.dat (Sounds)
From `gssndat.h`:
- Many sound effects (snd_beambull through snd_thrusters) - indices 0-41

## Recommended Approach

We will implement Option 1: Add compile flag support to load assets directly from source files.

1. Create a new compile flag: `-DDIRECT_ASSET_LOADING`
2. Modify asset loading code to detect this flag
3. When enabled, load assets directly from individual files instead of datafiles
4. Maintain backward compatibility with existing code

This approach:
✅ Doesn't require installing Allegro 4
✅ Uses assets we already have
✅ Maintains full game functionality
✅ Allows progressive development
✅ Avoids dealing with legacy data file formats

## Asset Availability Confirmation

We have confirmed extensive source assets that can be used directly:

### Ship Assets:
- Hundreds of ship sprite files (.tga/.shp) in ships/ directory
- Each ship has both definition (.shp) and image (.tga) files

### Level Assets:
- Multiple level directories with asset images (.tga/.bmp) in levels/mission/*/
- Each level has dozens of asset images for backgrounds, characters, etc.

### Sound Assets:
- Many .wav files scattered throughout the project
- Sound effects for various game events

### Map Assets:
- .mar map files for level layouts
- Configuration files for game settings

The project has a comprehensive collection of source assets that can be used to recreate the functionality without relying on the broken Allegro 4 data file system.