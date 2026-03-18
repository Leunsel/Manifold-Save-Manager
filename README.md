# Manifold Save Manager

Manifold Save Manager is a desktop tool for managing game saves, profiles, backups, and restore workflows.

It is designed to provide a structured and modular way to organize save data for different games and profiles, while keeping backup handling simple and accessible.

## Features

- Manage multiple game definitions
- Create and edit profiles per game
- Configure save scope rules
- Create and browse backups
- Restore backups when needed
- View backup details and preview included files
- Dockable ImGui-based interface
- Activity log for important actions

## Project Structure

The project is organized into separate modules for cleaner maintenance and easier expansion.

Typical areas include:

- **Core**  
  Backend logic for save management, backups, profiles, and game definitions

- **UI**  
  ImGui-based interface and application windows

- **Platform**  
  Platform-specific functionality such as file handling, explorer integration, and window/runtime setup

- **Math / Utility helpers**  
  Supporting functionality used by UI elements and animations

## Goals

The main goals of this project are:

- keep save management organized
- make backup and restore operations easy to use
- support multiple games and profiles
- provide a modular codebase that is easy to extend

## Roadmap

### Near term
- Finish the current refactor and reduce UI/state coupling
- Clean up helper ownership and shared utility functions
- Improve build stability and remove migration leftovers
- Stabilize backup creation, deletion, refresh, and restore flows

### Mid term
- Improve the docked UI workflow and window layout
- Add filtering, sorting, and better navigation for games, profiles, and backups
- Refine scope rule editing and backup detail previews
- Add stronger validation and safer destructive actions

### Longer term
- Add import/export for game and profile configurations
- Add backup retention policies and workflow presets
- Improve documentation, screenshots, and onboarding
- Prepare the first stable public release

## Tech Stack

This project currently uses:

- C++
- Dear ImGui
- Windows API
- DirectX 11

## Status

This project is currently under active development.  
Features, structure, and UI may continue to change over time.

## Building

Open the solution in Visual Studio and build the project in your preferred configuration.

Depending on your local setup, you may need:

- a Windows environment
- Visual Studio with C++ desktop development tools
- DirectX 11 SDK/runtime support
- Dear ImGui source files and required backend files

## Notes

This repository may include experimental UI changes, refactoring work, and prototype functionality while the tool evolves.

## Credits

Created by **Leunsel**  
Contributor: **Cfemen**
