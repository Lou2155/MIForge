# MIForge

MIForge is an Unreal Engine Editor plugin that automates texture compression correction, texture validation, Material Instance generation, and batch Material Instance parameter editing.


## Overview

Creating Material Instances manually can involve repetitive texture assignment, compression checks, naming, validation, and parameter editing.

MIForge streamlines this workflow by scanning selected Content Browser folders, classifying textures by naming suffix, grouping them into texture sets, validating them against preset material requirements, and generating Material Instances in bulk.


## Key Features

- Texture discovery and suffix-based classification
- Texture-set grouping
- Standard Material Instance generation
- RGB Mask Material Instance generation
- Vertex Paint Material Instance generation
- Texture compression and sRGB correction
- Batch Material Instance parameter editing
- Vertex Paint recipe saving and loading
- Validation summaries and detailed error reporting
- Existing-asset handling: Skip, Overwrite, or Create Unique
- Transaction support and generated-asset Undo/Redo


## Supported Presets

### Standard

- Supports common textures for ORM workflow,  such as:
    - Albedo
    - Normal
    - ORM
    - Emissive
    - Detail Normal

- Supports Triplanar if the user enables it

### RGB Mask

- Textures Supported:
    - Albedo
    - Normal
    - ORM
    - RGB Mask
    - Detail Normal

### Vertex Paint

- Textures Supported:
    - Albedo
    - Normal
    - ORM
    - Height

- Supports 4 material layers (Base, R, G, B) with optional height-based blending.

## Requirements

- Unreal Engine 5.6
- Windows 64-bit
- Editor build
- Both Blueprint and C++ project are supported

## Installation

1. Close Unreal Editor.
2. Copy the `MIForge` folder into your project's `Plugins` directory.
    ```text
    YourProject/
    └─ Plugins/
        └─ MIForge/
3. Enable MIForge from Edit → Plugins if it is not enabled automatically.
4. Restart Unreal Editor.


## Quick Start

1. Place textures inside a Content Browser folder.
2. Name the textures using the configured MIForge suffix rules. e.g. T_Rock_Albedo 
3. Right-click the folder in the Content Browser.
4. Select **Open MIForge**.
5. Choose a preset.
6. Select the detected texture sets.
7. Choose an output folder.
8. Click **Generate Material Instances**.


## Known Limitations

- MIForge has been developed and tested primarily in Unreal Engine 5.6.
- Batch Parameter Editor modifies shared Material Instance assets directly.
- Generated-asset Undo/Redo has not been validated against every extreme rapid-repeat case.
- Modified assets are marked dirty but are not automatically saved.
- Texture Compression Fix should not be assumed to provide complete transactional Undo.
- Very large folders may cause a synchronous scan delay.


## Architecture

MIForge uses a layered editor architecture:

```text
Slate UI
→ ViewModel and Texture Catalog
→ Validation and Generation Services
→ Unreal Editor APIs
→ Material Instance generation is divided into planning, conflict resolution, execution, and preset-specific parameter application.
```
See Architecture Documentation for the complete data flow.


## Future Plan

The ultimate goal of MIForge is to support any master material as presets to generate instances in bulk, along with a diverse preset library.
The user can also configure the setting pannel for their own custom master material, maximizing the flexibility of batch MI generation.


## Postscript

The current version is intended as a portfolio and testing release rather than a commercial production plugin.