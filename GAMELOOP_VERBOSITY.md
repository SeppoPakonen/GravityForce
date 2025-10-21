# Gameloop Verbosity Enhancement

## Summary

We have successfully enhanced the verbosity of the gameloop in Gravity Strike to provide better debugging information when running with the `-v3` flag.

## Changes Made

### 1. Enhanced Gameloop Messages

Modified the `play()` function in `src/gsmain.cpp` to include more detailed verbose messages:

- Added tracking of main loop iterations
- Added detailed messages for menu creation and initialization
- Added messages for different game modes (single player, two player, network)
- Added tracking of level restart conditions
- Added messages for entering and exiting the main play loop

### 2. Variable Definitions

Fixed the duplicate definition issue of `mainloop_verbose`:
- Confirmed it was already defined in `src/gsglob.cpp`
- Removed duplicate definition from `src/main.cpp`

### 3. Command Line Flag Support

Enhanced the `-v3` flag handling:
- Added proper detection and processing of the `-v3`/`--mainloop-verbose` flag
- Added clear console messages when the flag is active

## Testing Results

The enhanced verbosity is working correctly:

1. When running with `-v -v2 -v3` flags, the message "Main loop verbose output enabled (-v3 flag)" appears
2. Detailed gameloop messages are now displayed showing:
   - Main loop iterations
   - Menu creation and initialization
   - Game mode transitions
   - Level loading and restart conditions

## Verification

The build process completes successfully without errors, and the runtime output shows the enhanced verbosity messages as expected.

This enhancement will help with debugging gameloop issues by providing clearer visibility into the game's execution flow.