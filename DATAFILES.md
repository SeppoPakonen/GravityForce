# Data File Recreation Plan

## Current Status

The Gravity Strike project has several pre-built Allegro 4 data files in the `dat/` directory:
- `gsmenu.dat` - Menu assets (11,133 bytes)
- `gs.dat` - Game assets (135,521 bytes)
- `ships.dat` - Ship sprites (5,513 bytes)
- `fonts.dat` - Fonts (47,473 bytes)
- `gssnd.dat` - Sounds (various sizes)
- Plus others: crc.dat, keys.dat, text.lua, etc.

These files are encrypted with password "tanja" and are in Allegro 4 binary format.

## Investigation Results

1. **No Asset Creation Tools Found**: The project does not include the original Allegro 4 datafile creation tools
2. **No Build Scripts**: No scripts exist to recreate the data files from source assets
3. **Encrypted Format**: The data files are password-protected binary files in Allegro 4 format
4. **Missing Documentation**: No clear documentation on how the original data files were created

## Available Assets

Despite the missing data file creation tools, we do have plenty of source assets:
1. **Individual Images**: Thousands of .tga, .bmp, .png files in ships/, levels/, and other directories
2. **Configuration Files**: .ini, .cfg, .sc files with game settings
3. **Scripts**: Lua scripts for game logic
4. **Sound Files**: Individual .wav files for audio
5. **Map Files**: .mar files for level maps

## Regeneration Strategy

### Option 1: Install Allegro 4 and Use Original Tools (Recommended)
1. Install Allegro 4 development libraries:
   ```bash
   # On Debian/Ubuntu
   sudo apt-get install liballegro4-dev
   
   # On Fedora/RHEL
   sudo dnf install allegro4-devel
   ```
   
2. Locate Allegro 4 datafile tools:
   - `grabber` - GUI tool for creating data files
   - `dat2c` - Command line tool to convert data files to C source
   - `packfile` - Utility to create data files from individual assets

3. Recreate data files using the original tools

### Option 2: Write Custom Data File Creator
Create a custom tool that:
1. Reads individual asset files
2. Packages them into Allegro 4 format data files
3. Applies the correct password protection ("tanja")
4. Follows the correct data structure layout

### Option 3: Direct Asset Loading (Already Implemented)
Continue with the current approach of loading assets directly from individual files:
1. Enhance the `load_datafile()` function to actually load from source files
2. Create mapping from datafile indices to individual files
3. Maintain full compatibility with existing game code

## Detailed Asset Mapping

Based on the header files, we know what's in each data file:

### gsmenu.dat (Menu Assets) - From gsmd.h
Indices 0-26:
- Crosshairs (indices 0-4)
- Palette (index 5) 
- Ship images (indices 6-11)
- Title graphic (index 12)
- Weapon images (indices 13-26)

### fonts.dat (Fonts) - From fonts.h
Indices 0-9:
- Multiple fonts with different styles

### gssnd.dat (Sounds) - From gssndat.h
Indices 0-41:
- Many sound effects

### gs.dat (Game Assets)
- Maps and game objects

### ships.dat (Ship Sprites)
- Ship sprite definitions

## Recommended Approach

Continue with **Option 3: Direct Asset Loading** as it:
✅ Uses assets we already have
✅ Doesn't require installing legacy libraries
✅ Maintains full functionality
✅ Allows progressive implementation
✅ Eliminates dependency on broken Allegro 4 data loader

The framework is already in place with:
- `-DDIRECT_ASSET_LOADING` compile flag
- Enhanced debugging output
- Stubbed `load_datafile()` function that logs attempts
- Asset path verification

Next steps:
1. Implement actual direct asset loading in `load_datafile()` when `DIRECT_ASSET_LOADING` is defined
2. Create mapping functions from datafile indices to individual files
3. Load assets directly from source files instead of datafiles
4. Test incremental loading to prove the approach works