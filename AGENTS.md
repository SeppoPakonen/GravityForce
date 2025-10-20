# Agents Documentation

This file tracks agents and their capabilities that we're working with in this project.

## Qwen Code Agent
- **Role**: Primary development agent for code modifications and debugging
- **Capabilities**: 
  - Full file system access and modification
  - Shell command execution
  - Code analysis and refactoring
  - Debugging and problem solving
- **Current Task**: Gravity Strike data file loading issue diagnosis and resolution

## Task Tracking
We're currently working on reverse engineering the asset sources to recreate data files without using the Allegro 4 data file system. This involves:
1. Identifying all source assets in the project structure
2. Understanding how Allegro 4 data files were originally created
3. Creating a compilation flag to use source resource files directly
4. Eventually figuring out dat file recompilation

## Completed Activities
- ✅ Diagnosed root cause: Allegro 5 wrapper has stubbed datafile functions (return nullptr)
- ✅ Identified all required data files and their asset indexes
- ✅ Located complete source assets in project tree
- ✅ Documented asset reverse engineering plan
- ✅ Implemented direct asset loading framework with compile flag
- ✅ Modified build system to enable direct asset loading by default
- ✅ Enhanced debugging output to show direct asset loading attempts

## Current Focus
- 🔄 Reverse engineering asset sources to recreate data files without using Allegro 4 data file system
- 🔄 Implementing compile flag to bypass data files and load assets directly
- 🔄 Creating asset loading functions that can load directly from individual files

The Qwen agent is tracking this work and will move completed items to the DONE section when finished.