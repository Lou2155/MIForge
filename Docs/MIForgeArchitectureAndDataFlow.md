# MIForge architecture and data-flow audit

MIForge’s refactor is structurally complete: the former monolithic widget/generator responsibilities are now separated into presentation state, catalog services, domain validation, generation planning/execution, parameter appliers, and workflow coordinators.

The release assessment is:

- **Reasonable to call the refactor done:** yes.
- **Suitable for internal testing or a clearly labeled beta:** yes.
- **Ready for a public portfolio download as 1.0:** not quite. Resolve the three High-severity release risks in section 11 first.

---

# 1. Executive architecture summary

From the user’s perspective, MIForge is an Unreal Editor plugin that:

- Discovers and classifies textures from Content Browser folders.
- Groups textures into named texture sets.
- Validates those sets against Standard, RGB Mask, or Vertex Paint material contracts.
- Generates Material Instance Constants with configurable conflict handling.
- Fixes texture compression and sRGB settings.
- Batch-edits parameters across selected actors’ Material Instances.
- Saves and restores Vertex Paint layer recipes.
- Supports transactional generation, batch editing, and generated-asset undo/redo.

The principal architectural layers are:

1. **Editor integration**
   - FMIForgeModule
   - Content Browser and Actor context menus
   - Nomad tab registration

2. **Slate presentation**
   - SMainTabWidget
   - Preset panels, asset browser, validation panel, rows, recipe and layer UI

3. **Application/presentation state**
   - FMIForgeMainTabViewModel
   - FMIForgeTextureCatalog

4. **Domain logic**
   - Texture classification and grouping
   - Preset definitions
   - Rule-driven validation

5. **Generation application services**
   - Coordinator
   - Planner
   - Conflict resolver
   - Executor
   - Parameter appliers

6. **Unreal Editor integration services**
   - Asset Registry
   - Asset Tools
   - Material Editing Library
   - Editor Asset Library
   - Transaction system

7. **Persistence**
   - UMIForgeSettings and DefaultMIForge.ini
   - Vertex Paint recipe JSON
   - Generated and modified Unreal assets

The main design pattern is a pragmatic **View–ViewModel–Service pipeline**:

~~~text
Slate widgets
    ↓ commands / ↑ pull-based state
FMIForgeMainTabViewModel + FMIForgeTextureCatalog
    ↓
Validator / builder / generation services
    ↓
Unreal Editor APIs and assets
~~~

The refactor moved several responsibilities out of SMainTabWidget and the old generator:

- Selection, options and validation state moved into FMIForgeMainTabViewModel.
- Asset discovery and Asset Registry monitoring moved into FMIForgeTextureCatalog.
- UI layout moved into focused Slate panels.
- Generation orchestration moved into FMIForgeGenerationCoordinator.
- Read-only preflight moved into FMIForgeGenerationPlanner.
- Conflict handling moved into FMIForgeMaterialInstanceResolver.
- Mutation moved into FMIForgeGenerationExecutor.
- Parameter assignment moved into three preset-specific appliers.
- Parent-material contracts moved into FMIForgePresetDefinitions.

Delegates reduce coupling between state owners and UI consumers. The ViewModel does not directly manipulate list views or panel widgets; it broadcasts state events, and interested widgets decide how to refresh.

---

# 2. Source tree and component responsibilities

## Plugin-level structure

| Path | Responsibility |
|---|---|
| MIForge.uplugin | Editor-only plugin metadata and content/plugin dependency declaration. |
| Config/DefaultMIForge.ini | Default suffix and compression mappings. |
| Content/MasterMaterialPresets | Standard, RGB Mask and Vertex Paint master materials, example instances, functions and placeholder textures. |
| Resources/Icons | Slate and plugin icons. |
| Docs/RefactorBaseline.md | Pre-refactor compatibility baseline and known limitations. |
| Source/MIForge/Public | Shared data types and externally visible module/plugin declarations. |
| Source/MIForge/Private | Implementations and internal UI/application classes. |
| Private/Catalog | Texture catalog and Asset Registry refresh service. |
| Private/Generation | Planning, conflict resolution, execution and parameter application. |
| Private/Presets | Material and Vertex Paint contracts. |
| Private/UIs | Focused Slate panels and the main ViewModel. |
| Private/Tests | Unit and editor integration automation specifications. |
| Private/MIForgeBatchAdjustMIParams | Batch Material Instance parameter editing. |

## Module and main UI

| Component | Role and owned state | Calls and callers | Boundary/lifetime |
|---|---|---|---|
| FMIForgeModule — Source/MIForge/Public/MIForge.h | Module startup/shutdown, menu extensions, tab spawners, selected Content Browser folders and pending batch model. | Unreal creates it. It creates the main and batch tabs and invokes compression/batch workflows. | Module-lifetime regular C++ object. It should not own main-tab business state. |
| FMIForgeStyle — Source/MIForge/Public/MIForgeStyle.h | Static Slate style-set registration. | Called by FMIForgeModule. | Static shared style set. It should only resolve and register visual resources. |
| SMainTabWidget — Source/MIForge/Public/MainTabWidget.h | Main composition shell. Owns shared ViewModel, catalog, preset option objects and preset panel switcher. | Created by FMIForgeModule; creates all focused main-tab panels. | Slate-owned. It no longer scans, validates or generates directly. |
| FMIForgeMainTabViewModel — Private/UIs/MIForgeMainTabViewModel.h | Authoritative preset, generation options, target path, conflict policy, selection, validation and Vertex Paint layer state. | Mutated by panels and rows; calls builder, validator and preset definitions; broadcasts state delegates. | Shared regular C++ object using TSharedFromThis. It should not own Slate widgets or Unreal tabs. |
| SMIForgeAssetBrowserPanel | Search/filter presentation, texture and set lists, source-folder selection and row construction. | Reads catalog and ViewModel; subscribes to both. | Slate-owned. Its filters and widget references are presentation state. It should not own scanned assets. |
| SMIForgeStandardPresetPanel | Standard options, target/conflict sections, validation summary and generation command. | Reads/writes ViewModel; submits a request to the coordinator. | Slate-owned. No asset mutation. |
| SMIForgeRGBMaskPresetPanel | RGB Mask options and generation command. | Same pattern as Standard. | Slate-owned. No asset mutation. |
| SMIForgeVertexPaintPresetPanel | Vertex Paint generation controls and local instance-name text. | Reads ViewModel layer stack and submits a vertex request. | Slate-owned. The custom name is intentionally UI-local. |
| SMIForgeVertexPaintLayerStackPanel | Layer slots, thumbnails, drag/drop, assign/clear commands and recipe controls. | Mutates ViewModel and owns a recipe manager. | Slate-owned. Does not validate or generate itself. |
| SMIForgeValidationSummaryPanel | Pull-based validation counts and detail popup construction. | Reads the ViewModel through Slate attributes/callbacks. | Slate-owned. Does not own validation results. |
| SMIForgeTargetFolderSection | Target output path picker. | Writes ViewModel::SetTargetPath. | Slate-owned presentation component. |
| SMIForgeExistingAssetOptions | Skip/Overwrite/Create Unique selector. | Writes ViewModel::SetIfMIExists. | Slate-owned presentation component. |
| STextureTableRow | Individual texture row and checkbox. | Reads and mutates ViewModel selection. | List-view row lifetime. |
| STextureSetTableRow | Texture-set row, status icon, checkbox and drag source. | Validates status for display; executes a selection delegate. | List-view row lifetime. |
| SMIForgeTextureSetDropTarget | Accepts an MIForge texture-set drag operation for one Vertex Paint layer. | Executes FMIForgeOnTextureSetDropped. | Slate-owned. |
| FMIForgeTexSetDragDropOperation | Carries one shared texture set through Slate drag/drop. | Created by a texture-set row and consumed by a layer drop target. | Temporary shared drag operation. |
| PopupWindowCreator | Creates modal/general popup windows and text-input dialogs. | Used by validation and recipe UI. | Static utility; should not retain application state. |

## Discovery, classification and validation

| Component | Role and owned state | Calls and callers | Boundary/lifetime |
|---|---|---|---|
| FMIForgeTextureCatalog — Private/Catalog/MIForgeTextureCatalog.h | Owns selected source folders, scanned texture list, texture-set list, Asset Registry handles and queued-refresh ticker. | Created by SMainTabWidget; calls scanner, classifier and builder; observed by asset browser. | Shared regular C++ service using TSharedFromThis. |
| FMIForgeAssetScanner | Synchronously enumerates texture assets in folders. | Called by catalog and compression workflow; calls UEditorAssetLibrary. | Stateless temporary service. |
| FMIForgeTextureClassifier | Maps asset-name suffixes to EMIForgeTextureType, extracts base names and loads UTexture2D size. | Called by catalog and compression workflow; reads UMIForgeSettings. | Stateless temporary service. |
| FMIForgeTextureSetBuilder | Groups recognized texture records by base name and texture type. | Called by catalog, validator texture mode and ViewModel generation input construction. | Stateless temporary service. |
| FMIForgeValidator | Rule-driven Standard, RGB and Vertex Paint validation and summary aggregation. | Called by ViewModel, asset filtering, rows and planner’s Vertex Paint preflight. | Stateless domain service. It does not mutate assets. |
| FMIForgePresetDefinitions | Static Standard, RGB and Vertex Paint material contracts. | Used by validator, ViewModel, planner and appliers. | Function-local static definitions with process lifetime. |

## Material Instance generation

| Component | Role | Calls and callers | Boundary |
|---|---|---|---|
| FMIForgeGenerationCoordinator | Wraps generation in a transaction, records created assets for undo, logs results, navigates Content Browser and builds summary text. | Called by preset panels; calls generator. | Application workflow layer. It should not validate parameter contracts or mutate parameters directly. |
| FMIForgeMaterialInstanceGenerator | Small public facade over planner and executor. | Called by coordinator. | Stateless facade. |
| FMIForgeGenerationPlanner | Read-only preflight: preset lookup, target validation, parent loading, actual parent-parameter inspection, texture loading and plan construction. | Called by generator; calls definitions, validator and Material Editing Library. | Must not create or modify assets. |
| FMIForgeMaterialGenerationPlan | Collection of valid Standard/RGB items plus planning failures. | Produced by planner and consumed by executor. | Temporary value object. |
| FMIForgeVertexPaintGenerationPlan | Validated parent, layer stack, options and desired Vertex Paint name. | Produced by planner and consumed by executor. | Temporary value object. |
| FMIForgeMaterialInstanceResolver | Resolves Skip, Overwrite or Create Unique and creates new MIC assets through Asset Tools. | Called by executor. | Owns naming/conflict policy, not parameter mutation. |
| FMIForgeGenerationExecutor | Prepares MICs for mutation, dispatches appliers, updates results, marks packages dirty and deletes failed newly created assets. | Called by generator. | Mutation layer. |
| FMIForgeStandardParameterApplier | Assigns Standard textures and static switches. | Called by executor. | No asset creation, cleanup, counters or transaction orchestration. |
| FMIForgeRGBMaskParameterApplier | Assigns RGB textures, optional ORM/detail normal and switches. The emissive-channel option is switch-only. | Called by executor. | Same boundary as Standard applier. |
| FMIForgeVPParameterApplier | Assigns Vertex Paint layer textures, optional G/B switches and dependency-based height inputs. | Called by executor. | Same boundary as other appliers. |

MIForgeGenerationPlan.cpp, MIForgeTypes.cpp and MIForgeBatchParameterTypes.cpp currently contain no behavioral implementation; their types live in headers.

## Compression, batch, recipes, settings and undo

| Component | Responsibility |
|---|---|
| MIForgeUtilities::FixTextureCompressionInSelectedFolders | Scans and classifies textures, looks up expected compression settings, calls Modify, changes compression/sRGB, updates the resource and marks packages dirty. |
| MIForgeSelectionCollector | Walks selected actors, mesh components and material slots; collects MIC slots and groups them by root parent. |
| FMIForgeBatchParamModelBuilder | Extracts scalar, vector, texture and static-switch parameters; detects mixed values and reads grouping/sort metadata. |
| SMIForgeParentMtlGroupPicker | Lets the user select one root-parent group when several are present. |
| SMIForgeBatchParameterEditor | Holds editable parameter-row copies, search/group UI and applies checked rows inside a transaction. |
| FMIForgeVertexPaintRecipeManager | Converts R/G/B layer assignments to/from JSON using soft asset paths. |
| UMIForgeSettings | Unreal-managed UDeveloperSettings CDO for suffix and compression maps. |
| UMIForgeGenerationUndoRecord | Transactional transient UObject tracking created asset paths and deferred undo deletion state. |
| MIForgeUtilities | Cross-cutting editor UI, Content Browser, notification and asset-deletion helpers. It remains a broad utility collection rather than one cohesive service. |

---

# 3. High-level dependency structure

The implemented dependency direction is:

~~~text
FMIForgeModule
    ↓
Slate composition and panels
    ↓
FMIForgeMainTabViewModel / FMIForgeTextureCatalog
    ↓
Domain and generation services
    ↓
Unreal Editor APIs and asset objects
~~~

More specifically:

- The module knows about Slate tabs and top-level workflows.
- Slate panels know about the ViewModel, catalog and coordinator.
- The ViewModel knows about shared data models, the validator, set builder and preset definitions.
- The catalog knows about scanner, classifier, builder and Asset Registry.
- The generation facade knows only the planner and executor.
- The planner knows definitions, validation and read-only material/asset APIs.
- The executor knows the resolver, appliers and mutation APIs.
- The resolver knows Asset Tools and Editor Asset Library.
- Parameter appliers know definitions and Material Editing Library.
- Domain services do not know about Slate.

There is no major runtime circular ownership dependency. The apparent UI/ViewModel relationship is one-way in ownership:

- Widgets hold strong shared pointers to the ViewModel.
- ViewModel delegates use AddSP, which stores a weak Slate binding rather than strongly owning widgets.

Some direct dependencies remain:

- Rows directly mutate ViewModel selection.
- The asset browser directly reads the catalog.
- Preset panels directly construct generation requests and coordinator instances.
- MIForgeUtilities is used across unrelated subsystems.
- Internal generation and UI details sometimes live under Public, increasing the apparent public API surface.

Delegates primarily replace ViewModel-to-widget and catalog-to-widget calls. They are not used to abstract the domain pipeline; scanner, validator, planner and executor interactions are normal synchronous function calls.

---

# 4. Ownership and lifetime

| Object/state | Ownership and lifetime |
|---|---|
| FMIForgeModule | Created and owned by Unreal’s module manager. Persists for the loaded module lifetime. |
| Main SDockTab | Owned by the global tab manager and Slate. |
| SMainTabWidget | Owned by the tab’s Slate widget tree. |
| FMIForgeMainTabViewModel | Created with MakeShared by SMainTabWidget. The main widget and child panels hold TSharedPtrs. |
| FMIForgeTextureCatalog | Created with MakeShared by SMainTabWidget. Main widget and asset browser hold it. It uses TSharedFromThis. |
| Texture records and sets | Catalog arrays own TSharedPtr<FMIForgeTextureInfo> and TSharedPtr<FMIForgeTextureSet>. Selection and Vertex Paint slots share those objects. |
| ViewModel delegates | AddSP subscriptions are weak with respect to the Slate receiver. The asset browser also explicitly calls RemoveAll(this) in its destructor. |
| Catalog Asset Registry callbacks | Registry holds lambdas containing TWeakPtr<FMIForgeTextureCatalog>. Async game-thread callbacks pin before access. Handles are explicitly removed by Shutdown. |
| Catalog ticker | Captures a weak catalog pointer and is removed by Shutdown. |
| Vertex Paint panel delegate | Uses AddSP, so destruction of the panel does not leave a dereferenceable receiver, although the stale weak binding remains until the publisher dies. |
| UMIForgeSettings | Unreal-owned class-default object returned by GetMutableDefault. |
| Material and texture assets | Unreal-managed UObjects. Generation services pass raw pointers synchronously; they do not own those objects. |
| Generation plans | Temporary stack values. They hold shared texture sets and raw parent-material pointers for the synchronous planner→executor call. |
| Generation result arrays | Raw UObject pointers valid for the immediate coordinator transaction/result workflow; they are not persistent ownership. |
| UMIForgeGenerationUndoRecord | Transient transactional UObject retained by the transaction buffer. Its deferred callback uses TWeakObjectPtr. |
| Batch targets | Use TWeakObjectPtr for actors, components, materials and MICs so an open batch editor does not keep destroyed editor objects alive. |
| Batch editor tab reference | TWeakPtr<SDockTab> prevents a tab/widget ownership cycle. |
| Recipe manager | Value member of the Vertex Paint layer panel. Owns recipe structs; persisted textures are soft paths rather than hard UObject references. |
| Static preset definitions | Function-local static values lasting for the process lifetime. |
| Coordinator Content Browser ticker | Captures only a copied target-path string, so it is independent of panel/coordinator lifetime. |

### Remaining lifetime concerns

The catalog is well protected: weak registry callbacks, weak async tasks, a weak ticker capture and explicit unbinding.

The main remaining lifetime risk is at module scope. The Content Browser extender is registered with CreateRaw(this), and the ToolMenus callback and menu action capture or reference the module. Shutdown unregisters tab spawners but does not remove those menu registrations. That can leave callbacks pointing at an unloaded module during plugin reload or hot reload.

Slate lambdas that capture [this] are generally owned by child widgets in the same widget tree, making them safe under normal Slate destruction. The modal recipe-name popup also prevents ordinary interaction with the parent while its callback is pending.

---

# 5. Delegate map

| Delegate/event | Declaration/publisher | Subscriber/receiver | Trigger and result | Type and lifetime |
|---|---|---|---|---|
| FMIForgeOnPresetChanged | ViewModel header; SetPreset broadcasts the new preset. | SMainTabWidget::HandlePresetChanged; SMIForgeAssetBrowserPanel::HandlePresetChanged. | Switches the preset panel; VP forces Texture Sets mode and updates filters. | Multicast, AddSP, weak-safe. |
| FMIForgeOnGenerationOptionsChanged | ViewModel; option setters call NotifyGenerationOptionsChanged. | Asset browser. | Validation is recomputed first; status-filtered sets are refreshed. | Multicast, AddSP. |
| FMIForgeOnInputModeChanged | ViewModel; SetInputMode broadcasts the new mode. | No production subscriber found. | Currently useful for tests/future consumers; visible switching is handled directly by the asset-browser control. | Multicast; currently an unused production notification. |
| FMIForgeOnSelectionChanged | ViewModel; every effective selection mutation broadcasts after validation. | Asset browser. | Requests both list views to refresh checkbox state. | Multicast, AddSP. |
| FMIForgeOnValidationChanged | ViewModel; every RefreshValidation broadcasts. | No production subscriber found. | Validation widgets currently pull state directly through attributes instead. | Multicast; exposed but production-unused. |
| FMIForgeOnVertexPaintChanged | ViewModel; layer assignment/clearing broadcasts after validation. | Vertex Paint layer panel. | Replaces four layer thumbnail widgets. | Multicast, AddSP. |
| FMIForgeOnTextureCatalogRefreshStarted | Catalog Refresh. | Asset browser. | Clears selected textures and sets before rebuilding catalog objects. | Multicast, AddSP; explicitly removed in browser destructor. |
| FMIForgeOnTextureCatalogChanged | Catalog after rebuilding arrays. | Asset browser. | Refilters textures/sets and calls RebuildList for both list views. | Multicast, AddSP; explicitly removed. |
| FMIForgeOnTextureSetCheckStateChanged | TextureSetTableRow.h; row executes it. | Asset browser’s HandleTextureSetCheckStateChanged. | Selects/unselects the set and mirrors its relevant individual textures. | Single-cast Slate event, created with CreateSP. |
| FMIForgeOnTextureSetDropped | MIForgeTextureSetDropTarget.h; drop target executes it. | Vertex Paint layer panel lambda. | Calls ViewModel::AssignTextureSetToVertexLayer. | Single-cast return-value delegate scoped to widget. |
| FOnPathSelected | MIForgeUtilities.h; path picker executes it. | Source-folder or target-folder Slate callback. | Updates catalog folder scope or ViewModel target path and dismisses the menu. | Single-cast copied into the picker callback. |
| FMIForgeOnParentMaterialGroupChosen | Parent group picker header. | Module lambda. | Opens/replaces the batch editor for the selected group. | Single-cast, modal-window lifetime. |
| Content Browser path extender | Content Browser module. | FMIForgeModule::CustomMenuExtender. | Builds the Open MIForge and Fix Compression menu entries and stores selected paths. | Engine delegate registered with CreateRaw; currently not unregistered. |
| Main/batch tab spawn delegates | Global tab manager. | FMIForgeModule::OnSpawnMIForgeMainTab and OnSpawnBatchParameterEditorTab. | Constructs the requested tab. | Raw module delegates; tab spawners are explicitly unregistered. |
| ToolMenus startup callback/action | UToolMenus. | Module lambda and OpenBatchParameterAdjusterFromSelection. | Adds and executes the Actor context-menu batch command. | Long-lived callback; no owner-based unregister currently. |
| Asset Registry OnAssetAdded | Asset Registry. | Catalog weak lambda. | Copies FAssetData to a game-thread task and queues a refresh if relevant. | Multicast engine event; handle removed on shutdown. |
| Asset Registry OnAssetRemoved | Asset Registry. | Catalog weak lambda. | Same flow as asset-added. | Multicast engine event; handle removed. |
| Catalog refresh ticker | Core ticker. | Weak catalog lambda. | Coalesces registry activity into one next-tick full refresh. | Single ticker handle; explicitly removed. |
| Generated-asset deletion ticker | Core ticker. | Weak UMIForgeGenerationUndoRecord lambda. | Deletes assets next tick unless redo has restored bAssetsShouldExist. | Weak UObject capture. |
| Content Browser navigation ticker | Core ticker. | Value-capturing lambda. | Synchronizes the browser to the output folder after generation. | One-shot; no owning-object capture. |
| Slate button/check/search callbacks | Individual Slate widgets. | Owning panel or shared row item. | Mutate local UI state, ViewModel state, or execute a workflow. | Widget-scoped; mostly [this], CreateSP, or shared item captures. |
| Color picker OnColorCommitted | Batch editor. | Lambda holding the parameter-row shared pointer. | Updates the editable vector value. | Picker-scoped. |
| Recipe text-input confirmation | PopupWindowCreator. | Vertex Paint panel lambda. | Saves the recipe and refreshes combo options. | Modal-window scoped callback. |

Delegates are chiefly used where the publisher should not know which Slate widget will react. Direct calls remain preferable for the deterministic domain and generation pipeline.

---

# 6. Core data models

| Model | Meaning and data flow |
|---|---|
| EMIForgeTextureType | Classification enum: Unknown, Albedo, Normal, ORM, Emissive, Detail Normal, RGB and Height. |
| FMIForgeTextureInfo | Classified FAssetData, paths, suffix/base name, type and dimensions. Created by classifier; consumed by catalog, builder, UI, validation and generation. |
| FMIForgeTextureSet | Set name and map from texture type to texture record. Built from classified records and shared across UI, validation and generation. |
| FMIForgeTextureRequirement | One required/optional texture expectation used in validation output. |
| FMIForgeTextureSetValidationResult | Per-set successful, missing-required, missing-optional and unrecognized entries plus bCanGenerate. |
| FMIForgeValidationSummary | Aggregate set counts and detailed results used by Standard/RGB UI. |
| FMIForgeGenerationOptions | Standard/RGB preset, target, conflict policy and preset switches. Temporary generation request state. |
| FMIForgeVertexPaintGenerationOptions | Vertex Paint target, conflict policy and optional custom name. |
| FMIForgeGenerationResult | Created/updated/skipped/failed counts, created/affected object arrays and messages. |
| FMIForgeMaterialInstanceResolution | Resolver output: MIC pointer, Created/Updated/Skipped/Failed action and message. |
| FMIForgeMaterialGenerationRequest | Texture sets plus Standard/RGB options, assembled by a preset panel. |
| FMIForgeVertexPaintGenerationRequest | Copy of layer stack plus Vertex Paint options. |
| FMIForgeGenerationOutcome | Coordinator result plus user-facing summary; HasChanged means created or updated. |
| FMIForgePlannedMaterialItem | Valid desired name, parent material, source texture set and copied options. |
| FMIForgeMaterialGenerationPlan | Planned Standard/RGB items plus planning-failure count/messages. |
| FMIForgeVertexPaintGenerationPlan | One validated Vertex Paint output plan. |
| FMIForgeTextureBinding | Texture type→parent parameter name plus required/optional/gating option. |
| FMIForgeStaticSwitchBinding | Preset option→static-switch parameter name. |
| FMIForgeMaterialPresetDefinition | Parent material path and Standard/RGB bindings. |
| FMIForgeVertexPaintLayerDefinition | Layer metadata, requirement, texture parameter map and optional enable switch. |
| FMIForgeVertexPaintLayerSlot | Runtime layer metadata and shared assigned texture set. |
| FMIForgeVertexPaintLayerStack | Base, R, G and B slots. Authoritative copy lives in the ViewModel. |
| Vertex Paint validation structs | Per-layer and aggregate readiness/status data. |
| FMIForgeVertexPaintRecipeLayer | Serialized set name and soft texture paths. |
| FMIForgeVertexPaintRecipe | Recipe name plus R/G/B recipe layers. Base is intentionally not serialized. |
| FTextureExtension | Configurable suffix array for one named texture type. |
| FMIForgeTextureCompressionSettings | Expected Unreal compression enum and sRGB state. |
| FMIForgeMaterialSlotTarget | Weak actor/component/material/MIC/root-parent references for one mesh slot. |
| FMIForgeMaterialParentGroup | All targets sharing one root material, plus actor/slot/MIC counts. |
| FMIForgeMaterialSlotCollectionResult | Complete result of actor selection inspection and grouping. |
| FMIForgeBatchParameterRow | Editable scalar/vector/texture/switch value, metadata, apply flag and mixed-value flag. |
| FMIForgeBatchParameterModel | Chosen parent group and its parameter rows/counts. |
| UMIForgeGenerationUndoRecord | Transactional UObject containing created asset soft paths and bAssetsShouldExist. |

---

# 7. End-to-end data flows

## A. Opening the MIForge main tab

1. Unreal loads FMIForgeModule::StartupModule.
2. The module initializes FMIForgeStyle, installs the Content Browser path-menu extender and Actor context-menu command, and registers main and batch Nomad tab spawners.
3. Right-clicking a Content Browser folder calls CustomMenuExtender, which copies the folder paths into FMIForgeModule::SelectedFolderPaths.
4. “Open MIForge Tab” calls FGlobalTabmanager::TryInvokeTab.
5. OnSpawnMIForgeMainTab creates an SDockTab and SMainTabWidget.
6. SMainTabWidget::Construct creates FMIForgeMainTabViewModel and FMIForgeTextureCatalog.
7. The ViewModel initializes Vertex Paint slot metadata from FMIForgePresetDefinitions::GetVertexPaint and runs empty-state validation.
8. The catalog receives the selected source folders and synchronously calls Refresh.
9. It scans, classifies and groups assets before the asset-browser subscriber exists. This is safe because the completed catalog arrays are subsequently read during browser construction.
10. The main widget constructs the Standard, RGB, Vertex Paint and asset-browser panels.
11. The main widget subscribes to OnPresetChanged; the asset browser subscribes to ViewModel and catalog delegates.
12. The Vertex Paint layer panel loads recipe JSON during construction and subscribes to OnVertexPaintChanged.

The source folders and output target are distinct. The selected Content Browser folder initializes the scan scope; the output target remains empty until chosen by the user.

## B. Selecting folders and scanning textures

1. The asset browser opens a Content Browser path picker.
2. On selection it clears set selection, sets the catalog folder paths and queues a refresh.
3. The next-tick catalog refresh broadcasts OnRefreshStarted.
4. The asset browser clears all texture and set selections.
5. FMIForgeAssetScanner::FindTexturesInFolders calls UEditorAssetLibrary::ListAssets, resolves FAssetData and keeps classes derived from UTexture.
6. FMIForgeTextureClassifier::ClassifyTextures reads suffix mappings, finds the first matching suffix, assigns type/base name and loads each UTexture2D to obtain dimensions.
7. FMIForgeTextureSetBuilder::BuildTextureSets groups recognized textures by base name and type.
8. The catalog replaces its shared texture/set arrays and broadcasts OnCatalogChanged.
9. The asset browser rebuilds filtered arrays, computes visible set status and calls RebuildList.
10. Scanning itself does not populate ViewModel validation state. Validation occurs when the user selects a texture or set.
11. Selection mutates the ViewModel, which calls RefreshValidation before broadcasting OnSelectionChanged.
12. Validation panels pull the new summary, while the asset browser requests checkbox/list refreshes.

Thus, **catalog state and application selection state are deliberately separate**.

## C. Changing presets or settings

### Preset changes

1. The preset selector calls ViewModel::SetPreset.
2. The ViewModel updates CurrentPreset.
3. It recomputes Standard, RGB or Vertex Paint validation.
4. It broadcasts OnValidationChanged and then OnPresetChanged.
5. SMainTabWidget switches the active preset panel.
6. The asset browser updates set filters and, for Vertex Paint, forces Texture Sets input mode.

### Generation-option changes

1. A checkbox calls a ViewModel setter such as SetUseBaseORMTexture.
2. The setter calls NotifyGenerationOptionsChanged.
3. The ViewModel recomputes validation.
4. It broadcasts OnGenerationOptionsChanged.
5. The asset browser recalculates status-filtered sets.

The RGB “Enable Emissive Channel” option only affects a static switch. It does not activate or require an Emissive texture.

The RGB ORM rule is correctly conditional:

- bUseBaseORMTexture true: ORM is required.
- false: ORM is inactive and is neither required nor applied.

### Developer Settings changes

UMIForgeSettings is read when classification, filtering or compression runs. There is no settings-changed delegate:

- Changing suffix mappings does not automatically rescan an already open catalog.
- Filter option arrays are built during asset-browser construction.
- A manual refresh or reopened tab is needed for the complete UI to reflect new suffix categories.
- Compression mappings are read afresh when Fix Texture Compression is invoked.

## D. Generating Material Instances

1. The preset panel verifies that the output target is not empty and the current validation summary has no errors.
2. Standard/RGB calls BuildGenerationTextureSets. Texture Sets mode returns selected shared sets; Individual mode copies selected textures and groups them through FMIForgeTextureSetBuilder.
3. The panel builds a generation request and calls FMIForgeGenerationCoordinator.
4. The coordinator opens an FScopedTransaction.
5. FMIForgeMaterialInstanceGenerator invokes the planner.
6. The planner resolves the preset definition, validates /Game target scope, loads the plugin master material, reads actual parent texture/static-switch names, validates FAssetData and texture loads, creates desired names and validates package names.
7. For Vertex Paint, the planner also validates the stack, actual layer parameters/switches, Base and R assignment, Albedo and dependency-based Height mappings, then derives the default or custom name.
8. The executor processes each plan item.
9. FMIForgeMaterialInstanceResolver returns Skipped, an existing MIC for Overwrite, a unique destination or a newly created MIC through IAssetTools::CreateAsset.
10. The executor sets transactional flags and calls Modify.
11. Overwrite clears existing parameters and resets the parent.
12. The appropriate applier assigns textures, static switches and optional Vertex Paint layer switches.
13. No scalar parameters are assigned by MI generation. Scalars/vectors belong to the batch editor.
14. On success the MIC package is marked dirty.
15. The asset is not explicitly saved, preserving transactional behavior.
16. Asset Tools handles creation and Asset Registry registration.
17. Newly created assets are collected for custom undo tracking.
18. If nothing was created or updated, the transaction is cancelled.
19. The coordinator logs messages, queues Content Browser navigation and returns summary text.
20. The panel displays created/updated/skipped/failed counts.

Vertex Paint recipes are upstream UI state only. Generation does not resolve recipe JSON; the recipe manager reconstructs the ViewModel layer stack before generation.

## E. Fixing texture compression

1. The Content Browser folder menu stores selected paths.
2. “Fix Texture Compression” calls MIForgeUtilities::FixTextureCompressionInSelectedFolders.
3. The normal scanner and classifier discover and classify textures.
4. UMIForgeSettings::TextureCompressionSettingsMap supplies expected compression and sRGB values.
5. Unknown types or types without a mapping are skipped.
6. Each texture is loaded.
7. If its state differs, Texture->Modify is called, CompressionSettings and SRGB are assigned, UpdateResource is called and the package is marked dirty.
8. A notification reports updated and skipped counts.
9. Assets are not explicitly saved.
10. No FScopedTransaction surrounds this command. Reliable Undo should not be assumed.

## F. Batch Material Instance parameter editing

1. The Actor context menu calls OpenBatchParameterAdjusterFromSelection.
2. MIForgeSelectionCollector iterates selected actors, mesh components and material slots.
3. Only direct UMaterialInstanceConstant slot materials are accepted.
4. ResolveRootParentMaterial walks the instance-parent chain to the root material.
5. Targets are grouped by root-parent pointer and actor/slot/MIC counts are calculated.
6. Zero groups shows a message, one opens directly, and multiple groups open the parent-group picker.
7. FMIForgeBatchParamModelBuilder obtains scalar, vector, texture and static-switch names.
8. It uses the first MIC as the displayed starting value and compares effective values to detect mixed state.
9. It reads material group names and sort priorities.
10. SMIForgeBatchParameterEditor copies rows, groups/sorts them and filters by parameter, group or type.
11. The user checks rows and edits values.
12. Apply opens an FScopedTransaction.
13. Unique MICs are deduplicated, so a shared MIC is changed once.
14. Each MIC is marked transactional, calls Modify, receives checked assignments and is marked dirty.
15. Unreal’s transaction system records the changes.

The current behavior edits shared MIC assets directly. It does not duplicate an MIC per actor or slot.

## G. Recipe JSON loading and saving

1. SMIForgeVertexPaintLayerStackPanel::Construct calls LoadRecipesFromDisk.
2. The manager reads <ProjectSavedDir>/MIForge/VertexPaintRecipes.json.
3. Missing files are treated as an empty valid recipe list.
4. Invalid reads or malformed root JSON return false.
5. Each valid recipe contains RecipeName plus LayerR, LayerG and LayerB.
6. Each layer contains a set name and texture type/path objects.
7. Unknown texture-type strings and malformed entries are skipped.
8. Saving converts assigned R/G/B sets to soft paths and serializes the full recipe array.
9. Saving an existing name overwrites that recipe.
10. Loading copies the current stack and replaces only R/G/B assignments.
11. Base is deliberately preserved and is not recipe data.
12. Soft paths are synchronously loaded; unloadable textures are skipped.
13. A layer becomes empty if none of its textures can load.
14. The reconstructed stack is sent through SetVertexPaintLayerStack, causing validation and thumbnail refresh.

There is no schema version. Compatibility is tolerant at field level, but future structural migrations are not formally versioned.

## H. Undo and Redo for generated assets

1. The coordinator opens an FScopedTransaction.
2. Existing MICs call Modify for normal property undo.
3. Created assets are converted to unique FSoftObjectPaths.
4. A transient transactional UMIForgeGenerationUndoRecord is created.
5. Paths are assigned, it calls Modify, and bAssetsShouldExist becomes true.
6. On Undo, Unreal restores bAssetsShouldExist to false and PostTransacted receives an UndoRedo event.
7. If no delete is queued, bDeleteQueued becomes true.
8. A next-tick callback captures a TWeakObjectPtr to the record.
9. Before deleting, it verifies that the record exists, redo has not restored bAssetsShouldExist and deletion remains queued.
10. Redo before the callback restores bAssetsShouldExist, clears the pending delete and leaves the asset alive.
11. Otherwise Asset Registry lookup is attempted, loaded-object resolution is the fallback, valid MI_-prefixed MICs are deleted and package paths are rescanned.
12. Redo relies on Unreal’s transaction system to restore/recreate asset state.

The baseline correctly labels rapid repeated Undo/Redo as an edge-case limitation. The undo record does not explicitly close open asset editors before deletion.

## I. Asset Registry updates and automatic refresh

1. The catalog binds weak lambdas to OnAssetAdded and OnAssetRemoved.
2. Each callback copies FAssetData into a game-thread AsyncTask.
3. The task pins the weak catalog.
4. IsAssetRelevant accepts exact selected-folder matches and descendants.
5. Any relevant asset class—not only textures—queues a refresh.
6. QueueRefresh coalesces events with bRefreshQueued.
7. A next-tick weak callback runs one full scan/classify/build pass.
8. OnRefreshStarted clears current selection.
9. OnCatalogChanged causes the browser to refilter and rebuild lists.

The catalog does not bind OnAssetUpdated or OnAssetRenamed. Rename/move refresh relies on Unreal emitting corresponding remove/add activity. The baseline says this was manually exercised, but the implementation does not explicitly guarantee it.

---

# 8. State change and UI refresh model

Authoritative state is divided intentionally:

| State | Owner |
|---|---|
| Preset/options/target/conflict policy | FMIForgeMainTabViewModel |
| Selected textures and sets | ViewModel |
| Validation results and summaries | ViewModel |
| Vertex Paint layer stack | ViewModel |
| Scanned source folders and catalog items | FMIForgeTextureCatalog |
| Search text and filters | Asset browser panel |
| Active widget-switcher indices | Main widget/asset browser |
| Vertex Paint custom output name | Vertex Paint preset panel |
| Recipe combo selection and in-memory recipe list | Vertex Paint layer panel/recipe manager |
| Content Browser-selected folders before tab spawn | FMIForgeModule |
| Batch editor input | Module pending model, then batch editor value copy |
| Suffix/compression configuration | UMIForgeSettings CDO |
| Recipe persistence | JSON in Project Saved |

Slate mainly uses a **pull model**:

- Text, status and enabled-state attributes read ViewModel state when Slate evaluates them.
- Validation results are not copied into every panel.
- The validation panel therefore does not need to subscribe to OnValidationChanged.

Delegates handle refresh operations that Slate cannot infer automatically:

- OnSelectionChanged requests list refreshes.
- OnCatalogChanged rebuilds list children because catalog item pointers were replaced.
- OnVertexPaintChanged replaces thumbnail content.
- OnPresetChanged changes active preset/list modes.
- OnGenerationOptionsChanged recalculates set filters.

The principal synchronization risks are:

- Developer Settings changes have no live notification.
- The custom Vertex Paint output name is outside the ViewModel.
- Recipe selection is local to one panel.
- OnInputModeChanged and OnValidationChanged currently have no production subscribers.
- Catalog refresh replaces objects and clears selection; this avoids dangling selections but can be disruptive.
- Opening MIForge from a different folder while its tab is already open only focuses the existing tab; it does not reconstruct it with newly stored module paths.

---

# 9. Flowcharts

## Diagram 1: Overall architecture

~~~mermaid
flowchart TB
    subgraph Entry["Editor integration"]
        Module["FMIForgeModule"]
        Menus["Content Browser and Actor menus"]
        Tabs["Nomad tab spawners"]
    end

    subgraph View["Slate presentation"]
        Main["SMainTabWidget"]
        Panels["Preset, asset browser, validation and layer panels"]
        BatchUI["SMIForgeBatchParameterEditor"]
    end

    subgraph State["Application state"]
        VM["FMIForgeMainTabViewModel"]
        Catalog["FMIForgeTextureCatalog"]
    end

    subgraph Domain["Domain and application services"]
        Scan["AssetScanner / Classifier / TextureSetBuilder"]
        Validator["FMIForgeValidator"]
        Definitions["FMIForgePresetDefinitions"]
        Generation["Coordinator → Generator → Planner → Executor"]
        Batch["SelectionCollector / BatchParamModelBuilder"]
    end

    subgraph Persistence["Configuration and persistence"]
        Settings["UMIForgeSettings"]
        Recipes["FMIForgeVertexPaintRecipeManager / JSON"]
        Undo["UMIForgeGenerationUndoRecord"]
    end

    subgraph Unreal["Unreal Editor APIs"]
        EditorAPI["EditorAssetLibrary / AssetRegistry / AssetTools"]
        MaterialAPI["MaterialEditingLibrary"]
        Transactions["Transaction system"]
    end

    Module --> Menus
    Module --> Tabs
    Tabs --> Main
    Tabs --> BatchUI
    Main --> Panels
    Main --> VM
    Main --> Catalog
    Panels --> VM
    Panels --> Generation
    Panels --> Recipes
    Catalog --> Scan
    VM --> Validator
    VM --> Definitions
    Generation --> Definitions
    Generation --> Validator
    BatchUI --> Batch
    Scan --> Settings
    Scan --> EditorAPI
    Generation --> EditorAPI
    Generation --> MaterialAPI
    Generation --> Transactions
    Generation --> Undo
    Batch --> MaterialAPI
    BatchUI --> Transactions
    Recipes --> EditorAPI
    VM -.->|state delegates| Panels
    Catalog -.->|catalog delegates| Panels
    EditorAPI ==>|Asset Registry events| Catalog
    Transactions ==>|Undo/Redo events| Undo

    subgraph Legend["Legend"]
        L1["Caller"] -->|direct call| L2["Callee"]
        L3["Publisher"] -.->|delegate notification| L4["Subscriber"]
        L5["Unreal"] ==>|engine event| L6["Plugin"]
    end
~~~

## Diagram 2: Texture scan to UI refresh

~~~mermaid
flowchart TD
    User["User selects source folder"] --> Browser["SMIForgeAssetBrowserPanel"]
    Browser -->|"SetFolderPaths + QueueRefresh"| Catalog["FMIForgeTextureCatalog"]
    Catalog -->|"next-tick Refresh"| Started["OnRefreshStarted.Broadcast"]
    Started -.-> Clear["Asset browser clears selections"]
    Catalog --> Scanner["FMIForgeAssetScanner"]
    Scanner --> EditorLib["UEditorAssetLibrary::ListAssets / FindAssetData"]
    EditorLib --> AssetData["Texture FAssetData array"]
    AssetData --> Classifier["FMIForgeTextureClassifier"]
    Settings["UMIForgeSettings suffix mappings"] --> Classifier
    Classifier --> Classified["FMIForgeTextureInfo array"]
    Classified --> Builder["FMIForgeTextureSetBuilder"]
    Builder --> Sets["FMIForgeTextureSet array"]
    Classified --> CatalogState["Catalog texture items"]
    Sets --> CatalogState
    CatalogState --> Changed["OnCatalogChanged.Broadcast"]
    Changed -.-> Rebuild["Refilter and RebuildList"]
    Rebuild --> Rows["Texture and texture-set rows"]
    Rows -->|"SelectTexture / SelectTextureSet"| VM["FMIForgeMainTabViewModel"]
    VM -->|"RefreshValidation"| Validator["FMIForgeValidator"]
    Validator --> Summary["Validation summary stored in ViewModel"]
    Summary --> SelectionEvent["OnSelectionChanged.Broadcast"]
    SelectionEvent -.-> ListRefresh["RequestListRefresh"]
    Summary -.-> Pull["Validation panel pulls current summary"]
~~~

## Diagram 3: Material Instance generation

~~~mermaid
flowchart TD
    Input["Preset panel + ViewModel state"] --> UIGate["Target and validation UI checks"]
    UIGate --> Request["Material or Vertex Paint generation request"]
    Recipe["Vertex Paint recipe manager"] -.->|"optional upstream layer-stack source"| Input
    Request --> Coordinator["FMIForgeGenerationCoordinator"]
    Coordinator --> Transaction["FScopedTransaction"]
    Coordinator --> Generator["FMIForgeMaterialInstanceGenerator"]
    Generator --> Planner["FMIForgeGenerationPlanner"]
    Definitions["FMIForgePresetDefinitions"] --> Planner
    Validator["FMIForgeValidator for VP logical validation"] --> Planner
    Parent["Load parent and inspect actual parameters"] --> Planner
    Planner --> Plan["Generation plan + planning failures"]
    Plan --> Executor["FMIForgeGenerationExecutor"]
    Executor --> Resolver["FMIForgeMaterialInstanceResolver"]
    Resolver --> Policy{"Skip / Overwrite / Unique?"}
    Policy -->|Skip| Skipped["Skipped result"]
    Policy -->|Overwrite| Existing["Existing MIC"]
    Policy -->|Unique or new| AssetTools["IAssetTools::CreateAsset"]
    AssetTools --> NewMIC["Registered new MIC"]
    Existing --> Prepare["Transactional Modify; clear parameters; set parent"]
    NewMIC --> Prepare
    Prepare --> Dispatch{"Preset"}
    Dispatch --> Standard["FMIForgeStandardParameterApplier"]
    Dispatch --> RGB["FMIForgeRGBMaskParameterApplier"]
    Dispatch --> VP["FMIForgeVPParameterApplier"]
    Standard --> MaterialLib["UMaterialEditingLibrary"]
    RGB --> MaterialLib
    VP --> MaterialLib
    MaterialLib --> Dirty["MarkPackageDirty; no explicit save"]
    Dirty --> Result["FMIForgeGenerationResult"]
    Skipped --> Result
    Result --> UndoRecord["Record created paths in UMIForgeGenerationUndoRecord"]
    Result --> BrowserNav["Queue Content Browser navigation"]
    Result --> Notification["Summary notification"]
    UndoRecord --> Transaction
~~~

## Diagram 4: Batch parameter editor

~~~mermaid
flowchart TD
    Actors["Selected viewport actors"] --> Collector["MIForgeSelectionCollector"]
    Collector --> Components["Inspect UMeshComponent instances"]
    Components --> Slots["Inspect material slots"]
    Slots --> MICs["Accept UMaterialInstanceConstant slots"]
    MICs --> Roots["Resolve root parent materials"]
    Roots --> Groups["FMIForgeMaterialParentGroup array"]
    Groups --> Count{"Number of parent groups"}
    Count -->|0| NoTargets["Show no editable MIC message"]
    Count -->|1| Builder["FMIForgeBatchParamModelBuilder"]
    Count -->|many| Picker["SMIForgeParentMtlGroupPicker"]
    Picker --> Builder
    Builder --> Extract["Extract scalar, vector, texture and switch names"]
    Extract --> Mixed["Compare effective values across unique MICs"]
    Mixed --> Metadata["Read group name and sort priority"]
    Metadata --> Model["FMIForgeBatchParameterModel"]
    Model --> Editor["SMIForgeBatchParameterEditor"]
    Editor --> Search["Group, sort and search rows"]
    Search --> Edit["User edits values and checks bApply"]
    Edit --> Apply["Apply clicked"]
    Apply --> Transaction["FScopedTransaction"]
    Transaction --> Unique["Deduplicate shared MIC pointers"]
    Unique --> Modify["Set transactional + Modify"]
    Modify --> Assign["Set checked parameters"]
    Assign --> Dirty["Mark MIC packages dirty"]
    Dirty --> Notification["Show applied parameter/MIC counts"]
~~~

## Diagram 5: Undo and Redo state flow

~~~mermaid
stateDiagram-v2
    [*] --> Generated: Generation succeeds
    Generated: Asset exists
    Generated: bAssetsShouldExist = true
    Generated --> UndoEvent: User requests Undo
    UndoEvent: PostTransacted receives UndoRedo
    UndoEvent: bAssetsShouldExist = false
    UndoEvent --> DeleteQueued: bDeleteQueued = true\nqueue next-tick deletion
    DeleteQueued --> StillExists: Redo occurs before ticker
    StillExists: bAssetsShouldExist = true
    StillExists: bDeleteQueued = false
    StillExists --> Generated: Pending deletion exits
    DeleteQueued --> DeleteCheck: Ticker executes
    DeleteCheck --> Generated: Record invalid or assets should exist
    DeleteCheck --> Deleted: Still queued and assets should not exist
    Deleted: Asset Registry notified
    Deleted: DeleteObjectsUnchecked
    Deleted: Package paths rescanned
    Deleted --> Generated: Redo restores transaction state
~~~

## Diagram 6: Delegate communication

~~~mermaid
flowchart LR
    subgraph Publishers["Publishers"]
        VM["FMIForgeMainTabViewModel"]
        Catalog["FMIForgeTextureCatalog"]
        SetRow["STextureSetTableRow"]
        DropTarget["SMIForgeTextureSetDropTarget"]
        Picker["SMIForgeParentMtlGroupPicker"]
        Registry["IAssetRegistry"]
    end

    subgraph Events["Delegates and events"]
        Preset["OnPresetChanged"]
        Options["OnGenerationOptionsChanged"]
        Input["OnInputModeChanged"]
        Selection["OnSelectionChanged"]
        Validation["OnValidationChanged"]
        Vertex["OnVertexPaintChanged"]
        RefreshStart["OnRefreshStarted"]
        CatalogChanged["OnCatalogChanged"]
        SetChecked["FMIForgeOnTextureSetCheckStateChanged"]
        Dropped["FMIForgeOnTextureSetDropped"]
        GroupChosen["FMIForgeOnParentMaterialGroupChosen"]
        AssetEvents["OnAssetAdded / OnAssetRemoved"]
    end

    subgraph Subscribers["Subscribers"]
        Main["SMainTabWidget"]
        Browser["SMIForgeAssetBrowserPanel"]
        LayerPanel["SMIForgeVertexPaintLayerStackPanel"]
        Module["FMIForgeModule"]
        CatalogReceiver["FMIForgeTextureCatalog weak callback"]
        None["No production subscriber"]
    end

    VM -.-> Preset
    VM -.-> Options
    VM -.-> Input
    VM -.-> Selection
    VM -.-> Validation
    VM -.-> Vertex
    Preset -.-> Main
    Preset -.-> Browser
    Options -.-> Browser
    Selection -.-> Browser
    Input -.-> None
    Validation -.-> None
    Vertex -.-> LayerPanel
    Catalog -.-> RefreshStart
    Catalog -.-> CatalogChanged
    RefreshStart -.-> Browser
    CatalogChanged -.-> Browser
    SetRow -.-> SetChecked
    SetChecked -.-> Browser
    DropTarget -.-> Dropped
    Dropped -.-> LayerPanel
    Picker -.-> GroupChosen
    GroupChosen -.-> Module
    Registry ==>|engine events| AssetEvents
    AssetEvents -.-> CatalogReceiver
~~~

---

# 10. Architectural strengths

1. **The main widget is now a real composition shell.** It creates and arranges components instead of owning scanning, validation and generation logic.
2. **Generation responsibilities are clearly separated.** Planner, resolver, executor and appliers have distinct mutation boundaries.
3. **Planning occurs before mutation.** The planner checks real parent-material contracts and actual texture loads, not just logical validation.
4. **Preset definitions provide one material-contract vocabulary.** Validator, planner and appliers all refer to the same definitions.
5. **UI state has a clear authoritative owner.** Selection and generation options no longer live independently across panels.
6. **Catalog lifetime handling is strong.** Weak registry callbacks, game-thread dispatch, refresh coalescing and explicit handle removal are appropriate Unreal patterns.
7. **The refactor improves testability.** Classifier, builder, validator, definitions, ViewModel, catalog, planner and generator have separate tests.
8. **Partial planning is supported.** A bad set does not prevent the planner from producing valid items for others, although the current UI gate does not expose it.
9. **Preset-specific mutation code is isolated.** Extending a preset no longer requires expanding one large generation function.
10. **Shared MIC behavior in the batch editor is explicit.** Unique MIC pointers are deduplicated before mutation.
11. **Recipe persistence uses soft paths.** JSON recipes do not keep textures hard-referenced indefinitely.
12. **Created-asset undo handles rapid redo-before-delete.** bAssetsShouldExist and bDeleteQueued address the immediate deferred-deletion race.

---

# 11. Remaining risks and technical debt

| Severity | Issue and evidence | Why it matters | Release decision |
|---|---|---|---|
| **High** | Overwrite clears parameters before application. If apply fails, no old-state restoration occurs and the coordinator can cancel the transaction because no update was recorded. | A rare late failure could leave a user MIC cleared or partially modified without a useful undo entry. | **Fix or prove safety with a failure-injection test before release.** |
| **High** | Generated-asset undo uses soft paths and unchecked deletion; it does not close asset editors or verify identity beyond MIC type and MI_ prefix. | Rapid Undo/Redo is documented as limited. Path reuse or open editors increase edge-case risk. | Document for beta; stress-test and harden before public 1.0. |
| **Medium** | FMIForgeTextureSetBuilder discards unknown textures and never populates UnrecognizedTextures. Individual validation adds unknown counts regardless of the ignore option. | “Ignore Unrecognized Textures” does not operate consistently in normal flows. | Fix before release if the option remains visible or advertised. |
| **Medium** | Asset-browser status filtering uses Standard validation for every non-RGB preset, while rows use Vertex Paint validation in VP mode. | The filter and row icon can disagree. | Fix before release. |
| **Medium** | The catalog binds only OnAssetAdded and OnAssetRemoved. | Rename/move/update behavior depends on Registry event combinations. | Existing manual testing reduces risk, but explicit coverage is preferable. |
| **Medium** | Every relevant asset event triggers a full synchronous rescan, and classification loads every UTexture2D for dimensions. | Large folders or rapid imports can stall the editor. | Can wait for beta if folder sizes are modest; benchmark before wider release. |
| **Medium** | IsAssetRelevant does not check asset class. | Adding/removing any asset in scope rescans all textures. | Performance debt; can wait if scans remain fast. |
| **Medium** | BuildSummaryFromTextureSets counts warning sets only for missing optional textures, although unrecognized textures also produce Warning status. | Summary totals can disagree with row status. | Fix before release if unrecognized reporting is retained. |
| **Medium** | Standard/RGB panels reject the whole command when any set has errors, while the planner supports partial valid batches. | The planner’s partial-batch capability is unreachable from the standard UI. | Decide and document intended behavior. |
| **Medium** | Reopening MIForge from another Content Browser folder while the tab is open only focuses the existing tab. | New module-selected paths are not pushed into the existing catalog. | Worth fixing before a polished release. |
| **Medium** | Compression changes call Modify without an FScopedTransaction. | Users may expect Undo, but reliable MIForge undo is not provided. | Document clearly or add a transaction. |
| **Medium** | Recipes have no schema/version field, and load/save failures are not surfaced by the UI. | Corrupt files or future format changes can silently remove recipe availability. | Can wait for beta; version before evolving the schema. |
| **Medium** | Batch editing directly changes shared MIC assets. | One MIC referenced by many actors/assets changes everywhere. | Current behavior is explicit, but must be documented. |
| **Low** | Static-switch, texture and vector batch edits do not consistently clear bHasMixedValue in the display. | “Multiple Values” can remain after choosing a new common value. | UI polish. |
| **Low** | Coordinator logs every result message with Error verbosity, including normal skipped-existing messages. | Expected Skip behavior can appear as an error. | Fix during release polish. |
| **Low** | Overlapping scan folders are not deduplicated; duplicate texture types retain the last encountered texture. | Results may depend on enumeration order. | Document or deduplicate later. |
| **Low** | Suffix classification uses TMap iteration and first match. | User-configured overlapping suffix rules could be nondeterministic. | Add explicit ordering if custom rules expand. |
| **Low** | Several internal implementation classes are under Public. | This enlarges the apparent API and rebuild surface. | Can wait. |
| **Low** | MIForge.uplugin has empty description/docs/support metadata and README is minimal. | The package does not communicate installation, workflow, limitations or compatibility professionally. | Complete before portfolio release. |
| **Low** | MIForgeUtilities combines unrelated UI, logging, browser, compression and deletion helpers. | It remains a broad dependency and weak boundary. | Can wait until another feature forces separation. |

## Automation coverage gap

The source includes focused tests for:

- Texture classifier
- Texture-set builder
- Validator
- Preset definitions
- ViewModel
- Catalog lifecycle
- Generation planner
- Material Instance generator

Based on the previously completed test runs, those refactored paths are green. No dedicated automation specification was found for:

- Module unload/menu unregistration
- Installed-plugin resource resolution
- Compression transactions
- Recipe persistence/corrupt JSON
- Batch editor application
- Slate end-to-end synchronization
- Rapid generated-asset Undo/Redo
- Overwrite failure after parameters have been cleared

Those gaps align closely with the remaining release risks.

## Release recommendation

The present state should be labeled:

> **Refactor complete — release candidate for controlled beta testing.**

Before publishing a downloadable 1.0 portfolio release:

1. Correct installed-plugin resource resolution.
2. Safely unregister module menus/extenders.
3. Make overwrite failure atomic or prove safe rollback with failure injection.
4. Run a packaged-plugin smoke test in a clean project.
5. Document shared-MIC editing, unsaved dirty assets, compression undo and generated-asset undo limitations.

---

# 12. Suggested architecture narrative for documentation

## General audience

MIForge is an Unreal Editor workflow plugin that turns consistently named texture assets into ready-to-use Material Instances. It discovers textures, understands their naming conventions, groups and validates them, then safely applies them to Standard, RGB Mask or Vertex Paint master materials. It also provides texture-compression correction, reusable Vertex Paint recipes and batch Material Instance editing.

## Technical Artist or engineer

MIForge uses a layered editor architecture. Slate widgets are focused on presentation and forward commands to a shared FMIForgeMainTabViewModel, while FMIForgeTextureCatalog independently owns asset discovery and Asset Registry monitoring. Stateless domain services classify textures, construct sets and validate them against centralized preset definitions. Material Instance generation follows a staged pipeline: a planner performs read-only validation against the real parent material, a resolver handles Skip/Overwrite/Create Unique policy, an executor owns mutation and cleanup, and preset-specific appliers assign textures and static switches. Generation and batch edits use Unreal transactions, while a transient UMIForgeGenerationUndoRecord adds deferred deletion behavior for newly generated assets.

## 60–90 second interview explanation

> MIForge is an Unreal Editor plugin that automates Material Instance creation from texture naming conventions. The main architectural problem was that the original main widget and generator owned too many responsibilities, so I refactored the plugin into focused layers. Slate panels now handle presentation, a shared ViewModel owns preset options, selection and validation state, and a separate catalog owns scanning and Asset Registry refresh. The domain layer classifies textures, groups them into sets and validates them against centralized preset definitions. Generation is a pipeline: the planner performs read-only preflight against the actual master material, the resolver handles existing-asset policy, the executor performs mutation, and dedicated appliers handle Standard, RGB Mask and Vertex Paint parameters. Delegates keep the ViewModel and catalog independent from specific widgets, and weak callbacks protect deferred Asset Registry and ticker work. The result is easier to test, maintain and extend, while still supporting Unreal transactions, generated-asset undo, recipes, compression correction and batch Material Instance editing.
