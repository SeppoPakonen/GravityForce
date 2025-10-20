# Gravity Strike Development Tasks

## Completed Tasks
- ✅ Fixed inverted conditional logic in main.cpp that causes the program to fall through
- ✅ Added verbose flag to help explain internals of the program
- ✅ Built the project to test the changes
- ✅ Ran the program with verbose flag to confirm it works
- ✅ Added more verbose messages to gsmain.cpp init function to track detailed initialization
- ✅ Improved mouse initialization to handle remote/headless environments more gracefully
- ✅ Added more detailed verbosity to data file loading to diagnose why 'Can't load menu datafile' occurs
- ✅ Added -v2 flag to dump loaded variables and data structures to identify corrupted data
- ✅ Investigated how data files are generated and identified regeneration methods
- ✅ Identified root cause: Allegro 5 wrapper has stubbed data file functions that return nullptr
- ✅ Modified build.sh to enable direct asset loading by default
- ✅ Updated allegro5_wrapper.cpp to log direct asset loading attempts

## In Progress
- 🔧 Reverse engineer asset sources to recreate data files without using Allegro 4 data file system
- 🔧 Implement compile flag to bypass data files and load assets directly from source files
- 🔧 Create asset loading functions that can load directly from individual files

## Planned Work
- Test incremental loading to prove the approach works
- Eventually figure out dat file recompilation for full compatibility

## Key Finding
The core issue is that `/home/sblo/GravityForce/src/allegro5_wrapper.cpp` contains stub implementations of critical data file functions:
```cpp
DATAFILE *load_datafile(const char *filename) {
    (void)filename;
    // Not implemented - need a real implementation based on Allegro 5 data system
    return nullptr;  // <-- This is why all data files fail to load!
}
```

The project contains all necessary source assets (images, sounds, fonts, configurations) but lacks proper data file loading implementation. See ASSETS.md for detailed asset mapping.