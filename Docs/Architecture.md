# MIForge Architecture

MIForge is an Unreal Editor C++ plugin for turning conventionally named texture sets into persistent Material Instance Constant assets, validating them before generation, and batch-editing existing material instances used by selected actors.

Its architecture separates editor presentation, workflow state, validation, and asset mutation. Slate widgets collect artist intent; plain C++ models describe that intent; the generation pipeline performs Unreal asset operations only after preflight checks.

## High-Level Architecture

| Area | Responsibility |
|---|---|
| **UI** | Native Slate tabs and panels expose folder selection, texture-set views, presets, validation summaries, generation options, Vertex Paint layers, and batch parameter controls. UI callbacks translate artist choices into model updates or workflow requests. |
| **Validation** | Preset-aware rules evaluate required, optional, unsupported, and unrecognized texture roles for UI feedback. Generation then performs a second preflight against live assets, target-package rules, and the parent material's real parameter interface. |
| **Generation** | A plan–resolve–execute pipeline validates the destination and parent-material contract, resolves asset-name conflicts, creates or reuses a Material Instance Constant, and applies preset-specific texture and static-switch overrides. |
| **Batch editing** | A separate actor-selection workflow collects existing Material Instance Constants, groups them by their ultimate non-instance parent, builds a mixed-value parameter model, and applies only explicitly checked edits. |
| **Asset transactions** | Existing assets call `Modify()` before mutation and generation is grouped under `FScopedTransaction`. Newly created assets are additionally tracked by soft object path so Undo can remove them through a custom transactional record. Changed packages are marked dirty rather than automatically saved. |

```text
Content Browser folder
        |
        v
Texture Catalog -----> Slate Views
        |                   |
        v                   v
Classifier / Set Builder -> View Model -> Validation
                                      |
                                      v
                           Generation Request
                                      |
                         Plan -> Resolve -> Execute
                                      |
                                      v
                      Material Instance Constant
                      + dirty Unreal package
```

## Core Responsibilities

### Editor integration and UI composition

`FMIForgeModule` owns editor-level integration: Content Browser actions, actor context-menu actions, tab registration, and startup/shutdown cleanup. `SMainTabWidget` is the composition root for the main workflow and gives its child panels a shared catalog and view model.

The Slate layer is intentionally presentation-focused. It displays state, forwards user choices, and starts commands, but it does not own texture classification, preset rules, or Material Instance mutation.

### Catalog, model, and validation

`FMIForgeTextureCatalog` owns the current folder scope and the discovered texture/set snapshots. It coordinates scanning, classification, grouping, and refresh events.

`FMIForgeMainTabViewModel` owns transient workflow state: selected textures or sets, active preset, optional-feature flags, output path, collision policy, validation summaries, and Vertex Paint layer assignments. Multiple panels read and update the same state without directly depending on one another.

`FMIForgeValidator` converts preset requirements and current inputs into explanation data rather than a single Boolean. The UI can therefore show why a set is ready, warning-only, or blocked.

### Generation pipeline

Generation is split into focused stages:

- **Planner:** validates the target path, loads the preset parent material, checks its real parameter interface, validates required texture data, and produces a non-mutating plan.
- **Resolver:** implements Skip, Overwrite, and Create Unique behavior and creates persistent `UMaterialInstanceConstant` assets through Asset Tools when required.
- **Executor:** prepares assets transactionally, dispatches preset-specific parameter application, records results, and marks successful packages dirty.
- **Preset appliers:** translate Standard, RGB Mask, Decal, or Vertex Paint contracts into concrete `UMaterialEditingLibrary` calls.

This pipeline is best-effort rather than atomic: one item can fail while other valid items remain changed. The transaction provides one Undo scope, not database-style rollback.

### Batch parameter editing

Batch editing is not routed through the generation pipeline because it starts from scene usage rather than texture sets. The selection collector traverses selected actors, mesh components, and material slots; retains valid Material Instance Constants; and groups them by a compatible parent-material interface.

The batch model builder enumerates scalar, vector, texture, and static-switch parameters, samples effective values across unique instances, and marks mixed values. The editor applies only checked rows to still-valid weak targets inside one transaction.

## Data Flow

### Material Instance generation

```text
Selected folder paths
  -> recursive texture asset scan
  -> FAssetData snapshots
  -> suffix-based semantic classification
  -> base-name texture-set grouping
  -> preset-aware validation
  -> artist selection and generation options
  -> generation request
  -> parent/parameter/target preflight plan
  -> conflict resolution
  -> MIC creation or overwrite preparation
  -> preset texture and switch application
  -> MarkPackageDirty + result summary
```

Presets are compiled material contracts: they define the parent material path, expected texture parameters, static switches, and required or optional roles. Vertex Paint recipes are a separate convenience layer that stores R/G/B layer assignments as soft paths and reconstructs layer inputs before generation.

### Batch editing

```text
Selected actors
  -> mesh components and material slots
  -> unique Material Instance Constants
  -> grouping by ultimate parent interface
  -> shared parameter model + mixed-value detection
  -> artist checks explicit rows
  -> transactional parameter writes
  -> dirty MIC packages
```

## Key Design Decisions

### Model/View separation

Slate widgets have short, interaction-driven lifetimes and are expensive places to hide workflow rules. Keeping the catalog and view model outside individual panels gives every panel one consistent selection and validation state, makes rule behavior testable without constructing Slate, and prevents one widget from becoming the implicit owner of the entire tool.

### Preflight before mutation

The planner checks target syntax, parent availability, parameter names, required textures, and preset compatibility before asset creation or overwrite. This reduces the chance of creating a persistent asset and only then discovering that the master-material contract cannot be satisfied.

### Asset logic outside UI callbacks

Asset creation, conflict handling, parameter application, and failure accounting live behind generation services. UI code only builds a request and presents the outcome. This keeps editor presentation changes from silently changing asset semantics and allows the same mutation path to be exercised by automation tests.

### Transaction-aware created-asset Undo

Editing an existing UObject and creating a new asset are different lifecycle problems. `Modify()` can record changes to an existing object, but a newly created Content Browser asset also needs its existence handled during Undo. MIForge records created asset identities as `FSoftObjectPath` values on a transactional UObject and responds through `PostTransacted`.

The design uses soft paths rather than retained raw pointers because the operation crosses transaction callbacks and editor frames. It also leaves packages dirty instead of saving automatically, preserving the artist's opportunity to inspect or Undo before committing files.

## Unreal-Specific Lifecycle

### Asset discovery and refresh

The scanner uses Editor Asset Library queries to obtain `FAssetData`, avoiding immediate UObject ownership for catalog rows. Texture objects are loaded only where live data such as dimensions or parameter assignment is required.

The catalog listens to Asset Registry add/remove events for its folder scope. Registry callbacks are marshalled to the game thread and coalesced through a ticker before rebuilding the catalog. Delegate handles and pending ticker work are removed during shutdown so callbacks cannot outlive the catalog.

### Game-thread ownership

Slate state, UObject loading and mutation, Asset Tools creation, transactions, Material Editing Library calls, and Content Browser synchronization run on the editor/game thread. The current scan and generation paths are synchronous; this simplifies transaction ordering but can block the editor for large content sets.

### Transactions and dirty packages

`FScopedTransaction` defines one user-facing Undo step for generation or batch Apply. Existing Material Instance Constants receive transactional flags and `Modify()` before their overrides are changed. `MarkPackageDirty()` indicates that content changed, but MIForge does not automatically save or perform source-control checkout.

### `PostTransacted` and deferred deletion

`UMIForgeGenerationUndoRecord::PostTransacted` observes Undo/Redo state for assets created by generation. When an Undo means those assets should no longer exist, deletion is queued to the next tick instead of running inside the transaction callback.

The deferred callback:

1. Re-resolves each soft path through the Asset Registry or loaded-object fallback.
2. Rejects invalid, unreachable, non-MIC, or non-`MI_` objects.
3. Notifies the Asset Registry before unchecked object deletion.
4. Deletes the resolved created assets and synchronously rescans affected package paths.

If Redo occurs before the queued deletion runs, the transactional state cancels that pending deletion. Deferring destructive work avoids mutating editor asset state while Unreal is still processing the transaction notification and reduces stale-registry and repeated-Undo hazards.

## Current Boundaries

MIForge is an editor-focused, portfolio-scale implementation. Preset contracts are compiled rather than project-authored data assets; scanning is synchronous; generation is partially successful rather than atomic; packages are left dirty; and source-control checkout, automatic saving, long-operation progress, and cancellation are outside the current implementation.
