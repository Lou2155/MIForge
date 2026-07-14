# MIForge Refactor Baseline

- Engine: Unreal Engine 5.6
- Configuration: Development Editor / Win64
- Baseline commit: b07fc78552ce306157370e9ef5d8e0057a539fe8
- Build result:
1>------ Build started: Project: MIForge_PluginDev, Configuration: Development_Editor x64 ------
Failed to restore D:\Program Files\UE5\UE_5.6\Engine\Source\Programs\AutomationTool\AutomationUtils\AutomationUtils.Automation.csproj (in 97 ms).
Failed to restore D:\Program Files\UE5\UE_5.6\Engine\Source\Programs\AutomationTool\Gauntlet\Gauntlet.Automation.csproj (in 97 ms).
NuGet package restore failed. Please see Error List window for detailed warnings and errors.
1>Using bundled DotNet SDK version: 8.0.300 win-x64
1>Running UnrealBuildTool: dotnet "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MIForge_PluginDevEditor Win64 Development -Project="D:\UE5_Projects\MIForge_PluginDev\MIForge_PluginDev.uproject" -WaitMutex -FromMsBuild -architecture=x64
1>Log file: C:\Users\lou05\AppData\Local\UnrealBuildTool\Log.txt
1>Creating makefile for MIForge_PluginDevEditor (command line arguments changed)
1>Building MIForge_PluginDevEditor...
1>Using Visual Studio 2022 14.38.33145 toolchain (C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130) and Windows 10.0.22621.0 SDK (C:\Program Files (x86)\Windows Kits\10).
1>[Adaptive Build] Excluded from MIForge unity file: MainTabWidget.cpp, MIForge.cpp, MIForgeAssetScanner.cpp, MIForgeGenerationUndoRecord.cpp, MIForgeMaterialInstanceGenerator.cpp, MIForgeSettings.cpp, MIForgeStyle.cpp, MIForgeTexSetDragDropOperation.cpp, MIForgeTextureClassifier.cpp, MIForgeTextureSetBuilder.cpp, MIForgeTextureSetDropTarget.cpp, MIForgeTypes.cpp, MIForgeUtilities.cpp, MIForgeValidator.cpp, MIForgeVertexPaintRecipeManager.cpp, PopupWindowCreator.cpp, TextureSetTableRow.cpp, TextureTableRow.cpp, MIForgeBatchParameterTypes.cpp, MIForgeBatchParamModelBuilder.cpp, MIForgeSelectionCollector.cpp, SMIForgeBatchParameterEditor.cpp, SMIForgeParentMtlGroupPicker.cpp
1>Determining max actions to execute in parallel (8 physical cores, 16 logical cores)
1>  Executing up to 8 processes, one per physical core
1>Using Unreal Build Accelerator local executor to run 27 action(s)
1>  Storage capacity 40Gb
1>---- Starting trace: 260714_114229_vs36888 ----
1>UbaServer - Listening on 0.0.0.0:1345
1>------ Building 27 action(s) started ------
1>[1/27] Compile [x64] MIForgeBatchParameterTypes.cpp
1>[2/27] Compile [x64] MIForgeSettings.cpp
1>[3/27] Compile [x64] MIForgeAssetScanner.cpp
1>[4/27] Compile [x64] MIForgeBatchParamModelBuilder.cpp
1>[5/27] Compile [x64] MIForgeSelectionCollector.cpp
1>[6/27] Compile [x64] MIForgeGenerationUndoRecord.cpp
1>[7/27] Compile [x64] MIForgeMaterialInstanceGenerator.cpp
1>[8/27] Compile [x64] MIForgeStyle.cpp
1>[9/27] Compile [x64] MIForgeTypes.cpp
1>[10/27] Compile [x64] MIForge.cpp
1>[11/27] Compile [x64] MIForgeTexSetDragDropOperation.cpp
1>[12/27] Compile [x64] MIForgeTextureClassifier.cpp
1>[13/27] Compile [x64] MIForgeTextureSetDropTarget.cpp
1>[14/27] Compile [x64] MIForgeTextureSetBuilder.cpp
1>[15/27] Compile [x64] MIForgeValidator.cpp
1>[16/27] Compile [x64] PopupWindowCreator.cpp
1>[17/27] Compile [x64] Module.MIForge.cpp
1>[18/27] Compile [x64] MIForgeVertexPaintRecipeManager.cpp
1>[19/27] Compile [x64] MIForgeUtilities.cpp
1>[20/27] Compile [x64] TextureSetTableRow.cpp
1>[21/27] Compile [x64] TextureTableRow.cpp
1>[22/27] Compile [x64] SMIForgeParentMtlGroupPicker.cpp
1>[23/27] Compile [x64] SMIForgeBatchParameterEditor.cpp
1>[24/27] Compile [x64] MainTabWidget.cpp
1>[25/27] Link [x64] UnrealEditor-MIForge.lib
1>[26/27] Link [x64] UnrealEditor-MIForge.dll
1>[27/27] WriteMetadata MIForge_PluginDevEditor.target (UBA disabled)
1>Trace written to file C:/Users/lou05/AppData/Local/UnrealBuildTool/Log.uba with size 10.7kb
1>Total time in Unreal Build Accelerator local executor: 28.68 seconds
1>
1>Result: Succeeded
1>Total execution time: 38.36 seconds
========== Build: 1 succeeded, 0 failed, 11 up-to-date, 0 skipped ==========
========== Build completed at 11:42 AM and took 40.521 seconds ==========
- Compiler warnings: None
- Test asset folder: /Game/MIForge_RefactorTest

## Known defects
1. Generated-asset Undo/Redo may be unstable in edge cases
    -Rapid repeated Undo/Redo operations may produce unexpected behavior.
    -Asset deletion is deferred to a later tick to reduce transaction-related crashes.
    -Status: Known limitation.
2. Batch parameter editing does not currently support Mode 2
    -Assign duplicated unique MI behavior has not been implemented.
    -Status: Unimplemented feature, not part of the current compatibility baseline.

## Manual regression results
| Area | Baseline cases |
|---|---|
| Asset discovery | Open MIForge from a selected Content Browser folder |
| Classification | Albedo, Normal, ORM, RGB, Emissive, Detail Normal, Height, and unknown textures |
| Filtering | Search, texture-type filter, status filter, and input-mode switching |
| Live refresh | Add, rename, move, and remove a texture while the tab is open |
| Standard | Minimum required textures; optional emissive/detail normal; triplanar |
| RGB Mask | Required RGB texture; optional ORM; emissive channel; detail normal |
| Vertex Paint | Base only; R; R+G; R+G+B; missing required texture |
| Existing asset | Skip, Overwrite, and Create Unique |
| Transactions | Generate, undo, redo, and repeated undo/redo |
| Recipes | Save, load, overwrite, delete, and restart the editor |
| Batch editing | One parent group, multiple parent groups, and no valid selection |

