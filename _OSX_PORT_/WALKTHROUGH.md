# WaterRace Source Code Walkthrough

WaterRace is a 3D boat racing game developed by French Touch SARL and released in early 2001 for Apple computers running Mac OS 9. The codebase is C++ compiled with CodeWarrior and consists of two major layers: the **Infinity Engine** (a general-purpose 3D engine built on Apple RAVE) and the **WaterRace game layer** built on top of it. This walkthrough traces the code from startup through the game loop in linear order, following execution and explaining each major system along the way.

---

## Table of Contents

1. [Project Layout](#1-project-layout)
2. [Startup: `main.cp`](#2-startup-maincp)
3. [Toolbox and System Checks](#3-toolbox-and-system-checks)
4. [Screen Initialization](#4-screen-initialization)
5. [Math Foundation](#5-math-foundation)
6. [Core Data Structures: `Infinity Structures.h`](#6-core-data-structures-infinity-structuresh)
7. [The MetaFile Asset Format](#7-the-metafile-asset-format)
8. [Data Tags and Ship/Terrain Descriptions](#8-data-tags-and-shipterrain-descriptions)
9. [Texture Management and VRAM](#9-texture-management-and-vram)
10. [The Infinity Rendering Engine](#10-the-infinity-rendering-engine)
11. [Shaders and the Rendering Pipeline](#11-shaders-and-the-rendering-pipeline)
12. [Terrain Structures: `Infinity Terrain.h`](#12-terrain-structures-infinity-terrainh)
13. [The Infinity Audio Engine](#13-the-infinity-audio-engine)
14. [Clocks and Timing](#14-clocks-and-timing)
15. [The InfinityPlayer Subsystem](#15-the-infinityplayer-subsystem)
16. [Data Files and Asset Discovery](#16-data-files-and-asset-discovery)
17. [Preferences](#17-preferences)
18. [Pilot Profiles](#18-pilot-profiles)
19. [The Interface System](#19-the-interface-system)
20. [World Loading: `World.cp`](#20-world-loading-worldcp)
21. [Physics Engine: `Physic Engine.cp`](#21-physics-engine-physic-enginecp)
22. [Collision Engine: `Collision Engine.cp`](#22-collision-engine-collision-enginecp)
23. [Ship and Camera Drivers](#23-ship-and-camera-drivers)
24. [InputSprocket Integration: `ISpRoutines.cp`](#24-inputsprocket-integration-isproutinescp)
25. [The Game Loop: `Game.cp`](#25-the-game-loop-gamecp)
26. [Drawing a Frame: `Drawing.cp`](#26-drawing-a-frame-drawingcp)
27. [Post-FX](#27-post-fx)
28. [The Network Engine](#28-the-network-engine)
29. [Play Modes and Progression](#29-play-modes-and-progression)
30. [The Replay System](#30-the-replay-system)
31. [Common Code Utilities](#31-common-code-utilities)
32. [Compile-Time Configuration](#32-compile-time-configuration)
33. [Shutdown](#33-shutdown)

---

## 1. Project Layout

```
Source/
  Common code/          Reusable platform utilities
    Audio/              AudioCD, MP3 player, QDesign player, Sound utils
    Interface/          Error system, dialogs, navigation services, list control
    Screen/             Screen device enumeration, gamma/fade, screen capture
    Headers/            Keys.h (keyboard scan codes)
    Miscellaneous/      CD Protection header

  Infinity Engine/      The custom 3D engine
    Core Headers/       Infinity Structures.h, Infinity Terrain.h,
                        MetaFile Structures/Data Tags, Error Codes
    Core/               Vector, Matrix, Quaternion, Textures,
                        Infinity rendering (main renderer),
                        Infinity Audio Engine, Infinity Player
    Clocks/             Variable clock, Step clock, Relative clock
    MetaFile/           MetaFile Read/Write/Utils
    Shaders/            Per-shading-mode renderers, clipping, projection
    Utils/              Camera, Cinematic, Enclosure, Model Animation,
                        Object, Shape, Skeleton, Terrain, Texture, Track utils
    Post FX/            Lens flare, moon, rain, snow, screen text

  WaterRace final/      The game itself
    Conditionals Headers/  Compile-time feature flags
    Localizable resources/ String tables
    main.cp             Entry point
    WaterRace.h         Shared game-wide types and globals
    Game.h/.cp          Core game types and main game loop
    World.cp            Level/world loading
    Drawing.cp          Per-frame scene rendering
    Physic Engine.cp/.h Ship physics simulation
    Collision Engine.cp/.h Boundary and object collision
    Drivers Camera.cp   Camera mode callbacks
    Drivers Ship.cp     Ship AI, player input, replay, remote
    ISpRoutines.cp/.h   InputSprocket wrapper
    Network Engine*.cp  Open Transport networking (client & server)
    Interface.cp        Top-level UI coordinator
    Main Menu.cp        Main menu screen
    New menu.cp         New pilot creation menu
    Pilot Menu.cp       Pilot selection/load menu
    Characters Menu.cp  Character selection
    Browser.cp          Location and ship browser
    Score.cp            Post-race score screen
    Champion.cp         Championship results screen
    Credits.cp          Credits sequence
    Pilots.cp/.h        Pilot profile save/load
    Local Play Modes.cp Game mode rules and progression
    Data Files.cp/.h    Asset discovery and preloading
    Preferences.cp/.h   User settings save/load
    Infinity.cp         RAVE engine selection and context setup
    Local Infinity Utils.cp  Terrain resolve/preview helpers
    Text Engine.cp/.h   Bitmap HUD font renderer
    Utils.cp            Miscellaneous game helpers
    Replay.cp           Race recording and playback
    Terrain Rendering.cp Terrain/sea render helpers
    Progress bar.cp     Loading progress indicator
    Demo.cp             Attract/demo mode
    Pub.cp              Post-game advertisement screens
```

---

## 2. Startup: `main.cp`

Execution begins at `main()` in [Source/WaterRace final/main.cp](Source/WaterRace%20final/main.cp). It calls `Boot_Sequence()` and then decides which top-level mode to enter (interface, demo, network-only, etc.) before calling `Shut_Down()` at the end.

The compile-time conditionals (`__MOVIE__`, `__NETWORK_CLIENT_MODE__`, `__PLAY_DEMO__`, `__EXPO_MODE__`, `__USE_INI__`) select which execution path is taken after booting. In the normal retail build the flow is:

```
Boot_Sequence()
  -> Interface_Display()   // main menu loop
Shut_Down()
```

There is also an optional `.ini` file reader (`InitFile_Load`) that can hard-wire a game mode, terrain, ship, and pilot name for automated testing or kiosk use — parsed with the hand-rolled `InitFile_ReadCommand()` scanner.

An Apple Event handler (`ODOC_Handler`) at the very start of `main()` (when `__RECORD_RACE__ && __REPLAY_AWARE__`) catches drag-drop of a replay file onto the application icon, calling `Replay_SetupGame()` and `Game_New()` directly instead of showing the menus.

---

## 3. Toolbox and System Checks

`Boot_Sequence()` performs every initialization step in strict order:

### Classic Mac OS Toolbox Initialization (`Init_Toolbox`)

```cpp
InitGraf(&qd.thePort);
InitFonts();
FlushEvents(everyEvent, 0);
InitWindows();
InitMenus();
TEInit();
InitDialogs(0L);
InitCursor();
MaxApplZone();
for(i = 0; i < 16; ++i) MoreMasters();
```

This is the canonical Mac OS 9 startup ritual. `MaxApplZone()` expands the heap to its maximum size. `MoreMasters()` called sixteen times allocates master pointer blocks upfront to avoid fragmentation when handles are later allocated.

### Appearance Manager

`Init_Appeareance()` calls `Gestalt(gestaltAppearanceAttr, ...)` to confirm the Appearance Manager is present, then `RegisterAppearanceClient()`. This is required for the Platinum look on Mac OS 8/9 dialogs. If unavailable the app shows alert 128 and quits.

### Mandatory Component Checks

Each check follows the same pattern: test for presence, display an error alert with localized strings from the resource file, return false to cause `ExitToShell()`:

- **`Rave_Available()`** — tests whether `QADrawContextNew` is resolved (not `kUnresolvedCFragSymbolAddress`). RAVE (Rendering Acceleration Virtual Engine) is Apple's hardware-accelerated 3D API. Without it the game cannot render.
- **`QT3_Available()`** — uses `Gestalt(gestaltQuickTimeVersion, ...)` to require QuickTime 3.x or later (needed for audio decompression).
- **`SoundManager3_3_Available()`** — calls `SndSoundManagerVersion()` and requires version 3.3+ for the 3D spatial audio features.
- **`InputSprocket_Available()`** — checks `ISpGetVersion()` is at least 1.3. InputSprocket is Apple's game controller abstraction API.

### Optional Driver Version Checks

If compiled with `__CHECK_FOR_ATI_DRIVERS__` or `__CHECK_FOR_3Dfx_DRIVERS__`, the code iterates RAVE engines on the main display (`QADeviceGetFirstEngine`, `QADeviceGetNextEngine`) and uses `QAEngineGestalt(kQAGestalt_Revision, ...)` to check the driver version. Known-bad versions (ATI Rage 128 fog bug at version 5.9.5, Radeon fog bug at 6.6.10, 3Dfx below 1.1.3) display a caution alert but do not block startup.

### CD Protection

With `__CD_PROTECTION__` enabled, `XCheckForCD()` searches for a CD volume whose name starts with `"WaterRace - "` (supporting multiple regional releases: German, International, Softline). If not found the game either quits or (with `__NETWORK_ONLY_IF_CD_FAILURE__`) restricts to network-only mode.

### Preference Read, Data Scan, and Subsystem Init

After the checks:

1. `Preference_Read()` — loads or creates the user preferences file.
2. `SpinningCursor_Init()` / `SpinningCursor_Start()` — shows the animated watch cursor during loading.
3. `Resolve_DataFolders()` — locates data folders relative to the app.
4. `Preload_DataFolders()` — scans ships, locations, characters, and populates `coreData` and `addOnData`.
5. `Open_DataFiles()` — opens the main interface and sound resource forks.
6. `Network_Init()` — initializes Open Transport.
7. `InputSprocket_Init()` — registers game actions with InputSprocket.
8. `Get_Screen()` — selects the target display (prompts user if `redefineScreens` is true, i.e. Command key held at launch).
9. `Screen_Init()` — switches display resolution and creates windows.
10. `TextEngine_Init()` — prepares the bitmap HUD font.
11. `InfinityPlayer_Init(true)` — creates the 3D rendering buffers and starts the audio engine.
12. `Sound_InitSFx()` — loads UI sound effects from the sound resource file.

---

## 4. Screen Initialization

`Screen_Init()` in `main.cp` manages the full-screen setup for Mac OS 9:

1. **Resolution selection** — If a preferred resolution was saved in preferences and is still available (`SL_ModeAvailable`), it is used directly. Otherwise `SL_LookForResolution()` finds the closest matching `VDSwitchInfoRec` for the target interface size (640×480 at 16-bit, defined by `kInterface_Width/Height/Depth` in `Interface.h`).

2. **Screen fade** — With `__SCREEN_FADES__`, `StartFading()` and `Fade1ToBlack()` use the Display Manager gamma table to fade the screen to black before switching resolution.

3. **Display Manager** — `DMBeginConfigureDisplays(&theDisplayState)` opens a transaction. `SL_Switch(mainGDHandle, &switchInfo)` changes the actual video mode. `DMEndConfigureDisplays` commits.

4. **Background window** — A black 32000×32000 window (`backgroundWin`) is placed at (-16000, -16000) to fill the entire desktop during gameplay, hiding the Finder.

5. **GWorlds** — Two `GWorldPtr` buffers (`bufferGWorld`, `bufferGWorld2`) at the interface resolution are created and their `PixMap`s locked. These are the CPU-side pixel buffers for 2D interface compositing.

6. **Main window** — `mainWin` is a centered `CWindow` at `kInterface_Width × kInterface_Height`, used as the target for CopyBits blits from the GWorlds.

---

## 5. Math Foundation

The math layer lives in [Source/Infinity Engine/Core/](Source/Infinity%20Engine/Core/).

### Vectors (`Vector.h/.cp`)

`Vector` is a plain `{float x, y, z}` struct. Operations include dot product, cross product, normalization, length, linear interpolation, and matrix-vector multiply. These are ordinary float operations with no SIMD — the target hardware is PowerPC G3/G4.

### Matrices (`Matrix.h/.cp`)

`Matrix` is four `Vector` rows named `{x, y, z, w}`, representing a 4×3 affine transform (the fourth column is implicit `{0,0,0,1}`). Row `w` is the translation. Operations: multiply, transpose, invert (for camera matrices), build rotation matrices from Euler angles.

### Quaternions (`Quaternion.h/.cp`)

`Quaternion` is `{float w, x, y, z}`. Used for smooth keyframe interpolation (SLERP) in cinematic animations and for transmitting ship orientation over the network without gimbal lock.

### Fast Math (`Infinity Structures.h`)

Performance-critical math is precomputed or approximated:

```cpp
// 4096-entry lookup tables for sin/cos
float _sinTable[kTrigTableResolution];
float _cosTable[kTrigTableResolution];

inline float TSin(float x) {
    return _sinTable[((long)(x * 4096 / (2*Pi))) & 4095];
}
```

`FastSqrt()` uses the PowerPC-specific `__frsqrte` instruction (reciprocal square root estimate with one Newton-Raphson refinement) to avoid the slow `sqrt()` library call:

```cpp
inline float FastSqrt(float x) {
    float isqrt = __frsqrte(x);
    float temp1 = x * -0.5;
    float temp2 = isqrt * isqrt;
    temp1 *= isqrt;
    isqrt *= 1.5;
    return 1.0 / (temp1 * temp2 + isqrt);
}
```

---

## 6. Core Data Structures: `Infinity Structures.h`

[Source/Infinity Engine/Core Headers/Infinity Structures.h](Source/Infinity%20Engine/Core%20Headers/Infinity%20Structures.h) defines the complete object model for the engine.

### Shape

The atomic renderable unit:

```cpp
struct Shape {
    long          pointCount;
    VertexPtr     pointList;       // array of {x,y,z,u,v,c} vertices
    long          triangleCount;
    TriFacePtr    triangleList;    // array of {corner[3]} index triples

    short         normalMode;      // kNoNormals or kPerVertexNormals
    ProcPtr       renderingProc;   // function pointer to shader
    long          normalCount;
    VectorPtr     normalList;

    unsigned long flags;           // kFlag_RelativePos, kFlag_MayHide, etc.
    Matrix        pos;             // local transform
    float         rotateX/Y/Z, scale;
    long          link;            // parent shape index for hierarchical transforms

    Vector        boundingBox[8];  // OBB corners
    OSType        texture;         // texture name (four-char code)
    float         alpha, difuse, specular;
    unsigned short shading;        // kShader_None/Lambert/Phong/Sprite/etc.
    TQATexture*   texturePtr;      // resolved RAVE texture pointer
    ShapePtr      parent;
    float         sphereRadius;    // bounding sphere for culling
    Vector        sphereCenter;
    OSType        id;              // e.g. 'Head', 'Prp1', 'Engn'
};
```

`Vertex` stores position plus UV coordinates `u`, `v` and a color/light coefficient `c`. `TriFace` stores three unsigned long vertex indices.

### Object

A named collection of up to 200 Shapes with its own transform:

```cpp
typedef struct {
    unsigned long shapeCount;
    ShapePtr      shapeList[200];
    Matrix        pos;
    Vector        boundingBox[8];
    float         sphereRadius;
    OSType        id;
} Object;
```

### MegaObject

The top-level game entity container, combining geometry with animation, audio, and collision:

```cpp
typedef struct {
    Object        object;          // shapes + transform
    long          skeletonCount;
    SkeletonPtr   skeletonList[];  // skinned bone hierarchies
    long          scriptCount;
    ScriptPtr     scriptList[];    // named animation scripts
    long          cameraCount;
    CameraPtr     cameraList[];    // embedded camera definitions
    unsigned long boxCount;
    CollisionBox  boxList[];       // OBB collision volumes
} MegaObject;
```

Every ship, character, and decorative world object is a `MegaObject`. `ExtendedShip` (defined in `Game.h`) wraps a `MegaObject` with game-specific fields.

### Skeleton

A hierarchical bone system:

```cpp
typedef struct {
    OSType        id;
    OSType        shapeID;         // which shape's vertices it deforms
    unsigned long nodeCount;
    Node          nodeList[40];    // up to 40 bones
    ShapePtr      shapePtr;
    ShapePoint    points[];        // per-vertex {vector, normal} in bone space
} Skeleton;
```

Each `Node` has a parent index, up to 6 children, its own `Matrix` transform, and a list of vertex indices it controls. The skeleton system enables character animation (Leah in the menus, pilot figures in ships).

### Script and Animation

A `Script` (typedef'd as `CinematicScript`) is a named timeline of keyframe animations:

```cpp
typedef struct {
    OSType        id;         // e.g. 'play', 'TbOn', 'Strt'
    OSType        type;       // kType_Shape or kType_Skeleton
    long          startTime, length, loopTime;
    unsigned long animationCount;
    AnimationPtr  animationList[32];
} Script;
```

Each `Animation` targets a specific shape or skeleton and contains an array of `Event` keyframes with time, position, and quaternion rotation. The cinematic utilities in `Cinematic Utils.cp` evaluate these at runtime by finding the surrounding keyframes and interpolating.

### State

The RAVE render context wrapper — the central object passed to all rendering calls:

```cpp
typedef struct {
    float         d, f, h, c;    // near clip, far clip, half-width, hide distance
    TQAEngine*    engine;
    TQADrawContext* drawContext;
    Rect          viewRect;
    float         viewHeight, viewWidth;
    Vector        lightVector;
    float         ambient;
    // Precomputed projection constants:
    float         p1, p2, pixelConversion;
    float         projectionOffset_X, projectionOffset_Y;
    float         zoom, invRange;
    Boolean       linearZBuffer;
    unsigned long frameTime, lastFrameTime;
    // Plus texture filter, antialiasing, z-buffer mode flags
} State;
```

---

## 7. The MetaFile Asset Format

All game content (ships, terrains, characters) is stored in a proprietary binary format called MetaFile. The reader is [Source/Infinity Engine/MetaFile/MetaFile Read.cp](Source/Infinity%20Engine/MetaFile/MetaFile%20Read.cp); the corresponding writer is [MetaFile Write.cp](Source/Infinity%20Engine/MetaFile/MetaFile%20Write.cp) (used by the editor tools distributed separately).

`MetaFile Structures.h` defines the file structure. The format uses a chunk system similar to IFF/AIFF: each chunk has a four-character OSType tag, a size, and a payload. Chunks can be nested. The reader walks the file seeking chunks by tag and dispatches to type-specific parsers.

A MetaFile for a ship contains:
- A `MegaObject` chunk (with embedded `Shape`, `Skeleton`, `Script`, `Camera`, `CollisionBox` sub-chunks)
- A `TextureSet` chunk with compressed texture images (stored as PICT handles)
- Data sub-tags (`'shds'`, `'shpp'`) with ship metadata and physics parameters

A MetaFile for a terrain contains:
- The `Terrain` structure (meshes, sea, track, enclosures, sources, cameras, sounds, animations)
- Texture sets for terrain and models
- Data sub-tags (`'trds'`) with terrain metadata

---

## 8. Data Tags and Ship/Terrain Descriptions

[Source/Infinity Engine/Core Headers/MetaFile Data Tags.h](Source/Infinity%20Engine/Core%20Headers/MetaFile%20Data%20Tags.h) defines the subtag constants and the main data structures embedded in MetaFiles.

### `Data_ShipPhysicParameter`

The most important data structure for gameplay — the complete physics specification of a ship:

- **Propulsion**: `propulsionType` (water propeller, air propeller, or water jet), whether steering is applied to thrust direction (`propulsionSteering`), maximum power, turbo power, reverse power, power change rate, idle power
- **Aerodynamics/Hydrodynamics**: frontal/lateral/floor drag areas (`area_frontal/lateral/floor`), drag coefficients (`Cx_H/L/V`, `Cw`, `Cf`)
- **Mass and inertia**: `mass`, `inertia_roll/pitch/yaw`
- **Archimedes buoyancy**: `archimedeCorrectionFactor_front/back` for adjusting the buoyancy simulation
- **Turbo**: position, duration, steering behavior
- **Rudder**: position, width, max rotation
- **Wake effect**: half-width, distances, lifetime
- **Bot AI parameters**: turn targets, power levels, turbo usage thresholds — all with names like `bot_Turn_Target`, `bot_Power_Max`, `bot_Turbo_CollisionDistance`
- **Sound**: pitch scaling ratios for engine sound relative to motor power

This structure is loaded from the `'shpp'` sub-tag in the ship's MetaFile and stored in `ExtendedShip.params`. Every physics and AI calculation reads from it, making ships fully data-driven.

### `Data_TerrainDescription` and `Data_CharacterDescription`

Metadata records embedded in terrain and character MetaFiles. They contain the real display name, localized description text, difficulty-specific point values and par times, map position coordinates, music track name, and max network player count.

---

## 9. Texture Management and VRAM

Textures live in [Source/Infinity Engine/Core/Textures.h](Source/Infinity%20Engine/Core/Textures.h) and [Infinity rendering.h](Source/Infinity%20Engine/Shaders/Infinity%20rendering.h).

A `CompressedTexture` is stored in a MetaFile as a PICT image (Apple's bitmap format) plus metadata: OSType name, width/height/depth, an optional separate alpha layer (`alphaLayerData`), and the `uncompressedSize` hint for VRAM budget estimation.

A `TextureSet` groups up to 200 textures under a single OSType ID, allowing ships to have texture variants (different liveries) by swapping texture sets at load time.

`TextureMemory` is the VRAM tracking structure:

```cpp
typedef struct {
    TQAEngine*           engine;
    long                 textureCount;
    OSType               nameList[512];     // OSType keys
    TQATexture*          textureList[512];  // RAVE texture handles
    CompressedTexturePtr storageList[512];  // CPU-side source data
} TextureMemory;
```

Key operations:

- **`VRAM_LoadTextureSet()`** — iterates a `TextureSet` and for each texture not already in VRAM calls `QATextureNew()` to upload to the hardware. Respects mipmap preferences (`kTextureFlag_MipMap`, `kTextureFlag_ForceMipMap`) and reduces texture resolution if VRAM is limited.
- **`VRAM_Purge()`** — calls `QATextureDelete()` on all uploaded textures and clears the lists.
- **`MegaObject_Resolve()`** — after a MegaObject is loaded, walks all its shapes and fills in `shape->texturePtr` by looking up `shape->texture` (the OSType name) in the `TextureMemory`. This converts name references to direct RAVE pointers for zero-overhead rendering.

---

## 10. The Infinity Rendering Engine

The main renderer is in [Source/Infinity Engine/Shaders/Infinity rendering.cp](Source/Infinity%20Engine/Shaders/Infinity%20rendering.cp) and its header [Infinity rendering.h](Source/Infinity%20Engine/Shaders/Infinity%20rendering.h).

### RAVE

RAVE (Rendering Acceleration Virtual Engine) is Apple's hardware 3D API for classic Mac OS, predating OpenGL on the platform. The key types:

- `TQAEngine` — represents a hardware or software renderer (ATI Rage, 3Dfx Voodoo, Apple Software Renderer)
- `TQADrawContext` — a rendering surface bound to a window or GDevice
- `TQATexture` — a GPU-resident texture object
- `TQAVTexture`, `TQAVGouraud` — per-vertex data structures for textured and Gouraud-shaded triangles
- `TQAIndexedTriangle` — triangle index buffer

The primary drawing calls are:
- `QASubmitVerticesTexture(ctx, count, vertices)` — uploads textured vertex data
- `QADrawTriMeshTexture(ctx, count, triangles)` — draws a triangle mesh using previously submitted vertices
- `QASubmitVerticesGouraud` / `QADrawTriMeshGouraud` — same for color-only geometry
- `QARenderStart(ctx, ...)` / `QARenderEnd(ctx, ...)` — bracket a frame

### Context Creation

`InfinityRendering_CreateContext()` calls `QADrawContextNew()` to create a hardware-accelerated draw context bound to the game window, then computes all the projection constants stored in `State`:

```
p1 = viewHeight / 2.0 / d        // perspective scale at near plane
p2 = viewWidth / 2.0 / d
projectionOffset_Y = viewHeight / 2.0
projectionOffset_X = viewWidth / 2.0
pixelConversion = (f - d) / (f * d)  // for z-buffer linearization
invRange = 1.0 / (f - d)
```

### Engine Selection

`Infinity.cp` contains `Get_GameEngine()` which calls `Get_BestRaveEngine()`. This enumerates all RAVE engines available on the selected `GDHandle` (display device) via `QADeviceGetFirstEngine` and picks the non-software one (hardware accelerator). If the user stored a preferred engine name in preferences it is used directly.

`Get_InterfaceEngine()` finds the Apple Software Renderer specifically for the menus, since hardware acceleration may not be available or desirable for 2D-heavy menu animation.

### Render Buffers

`InfinityRendering_AllocateBuffers()` allocates three shared arrays that all shaders write into before submitting to RAVE:

```cpp
TQAIndexedTriangle *trianglesBuffer;   // reused triangle index array
TQAVTexture        *vTextureBuffer;    // textured vertex staging buffer
TQAVGouraud        *vGouraudBuffer;    // Gouraud vertex staging buffer
```

All renderers fill these buffers then submit with a single RAVE call, batching vertices per shape.

---

## 11. Shaders and the Rendering Pipeline

Each `Shape` has a `renderingProc` function pointer set at load time by `Shape_ResolveRendering()`. The pointer type is:

```cpp
typedef void (*Rendering_Proc)(
    StatePtr state,
    ShapePtr shape,
    MatrixPtr globalPos,      // object's world-space matrix
    MatrixPtr negatedCamera,  // inverse camera matrix
    VectorPtr cameraPos,      // camera world position (for culling)
    ShapePtr shapeList[],     // sibling shapes (for linked shapes)
    long clip                 // kNoClipping / kZClipping / kXYClipping
);
```

The `Shape_CallRenderingProc` macro dispatches through this pointer. The shading mode determines which function is assigned:

| Shading Mode | File | Description |
|---|---|---|
| `kShader_None` | `Infinity rendering Gouraud.cp` | Flat vertex colors, no lighting |
| `kShader_Lambert` | Same | Diffuse N·L lighting per vertex |
| `kShader_Phong` | Same | N·H specular highlight added |
| `kShader_Glow` | Same | Emissive — full brightness regardless of light |
| `kShader_None` (textured) | `Infinity rendering Texture.cp` | Textured, unlit |
| `kShader_Lambert` (textured) | Same | Textured with Lambert shading |
| `kShader_Phong` (textured) | Same | Textured with Phong shading |
| `kShader_Sprite` | `Infinity rendering Sprite.cp` | World-space billboard |
| `kShader_Light` | `Infinity rendering Light.cp` | Volumetric light cone |
| `kShader_Particle` | `Infinity rendering Particle.cp` | Particle system |
| `kShader_Lens` | `Infinity rendering Lens.cp` | Lens flare element |
| `kShader_Audio` | (special) | Audio source — not rendered visually |

### Per-Shape Render Flow (Textured Example)

`Texture_DrawShape_LambertShading()` in `Infinity rendering Texture.cp`:

1. **Bounding sphere cull**: compute sphere center in view space; if entirely behind far plane or outside frustum, skip.
2. **Determine clip mode**: if sphere entirely inside frustum → `kNoClipping`; if overlapping near plane → `kZClipping`; if overlapping side planes → `kXYClipping`.
3. **Transform vertices**: multiply each vertex through `globalPos × negatedCamera` (object-to-view transform). This is a manual matrix-vector multiply loop over `shape->pointList`.
4. **Light computation**: if Lambert shading, transform the normal by the rotation part of the matrix and compute `intensity = dot(transformed_normal, state->lightVector)`, clamped and combined with `state->ambient`.
5. **Fill `vTextureBuffer`**: for each vertex, compute projected screen coordinates using `state->p1/p2/projectionOffset_X/Y` and the perspective division `1/z`, write `{x, y, z_buffer, w=1/z, r,g,b,a, u*w, v*w}` into `TQAVTexture`.
6. **Build `trianglesBuffer`**: fill `TQAIndexedTriangle` array from `shape->triangleList`.
7. **Submit to RAVE**: `QASubmitVerticesTexture(state->drawContext, count, vTextureBuffer)` then `QADrawTriMeshTexture(state->drawContext, count, trianglesBuffer)`.

Clipping (`Clipping Texture.cp`, `Clipping Gouraud.cp`) handles triangles that straddle the near plane by subdividing them into fully visible sub-triangles before submitting.

### Projection Mathematics

`Projection Utils.h` provides the screen-space projection formula. For a point at view-space position (vx, vy, vz):

```
screen_x = projectionOffset_X + p2 * vx / vz
screen_y = projectionOffset_Y - p1 * vy / vz
z_buffer = (vz - d) * invRange           // linear z
     or  = d / (vz * (d - f)) + f/(f-d)  // non-linear 1/z buffer
w        = 1.0 / vz                      // perspective-correct texture interpolation
```

---

## 12. Terrain Structures: `Infinity Terrain.h`

[Source/Infinity Engine/Core Headers/Infinity Terrain.h](Source/Infinity%20Engine/Core%20Headers/Infinity%20Terrain.h) defines all world geometry types.

### `Terrain`

The root world object. Notable fields:

- `meshList[128]` — array of `TerrainMeshPtr` (land geometry)
- `seaList[128]` — array of `SeaMeshPtr` (water surfaces)
- `trackList[8]` — array of `RaceTrackPtr` (race waypoints)
- `enclosureList[32]` — array of `CollisionEnclosurePtr` (invisible walls)
- `sourceList[64]` — positional audio emitters (sounds that play at fixed world positions)
- `soundList[64]` — ambient sounds with random timing
- `animList[64]` — `ModelAnimation` keyframe paths for animated world objects
- `cameraList[64]` — `Camera` placements for TV/spectator modes
- Lighting: `ambientLight`, `sunIntensity`, `sunAngle_X/Z`
- Sky: `skyFlags` (rain, snow, moon, lens flare), `skyRadius`, `skyHeight`, tile count
- Fog: `fogMode`, `fogColor_r/g/b`, `fogDensity`, `fogStart/End`, `clippingDistance`
- Wave parameters: `waveAmplitude`, `waveScaleX/Z`, `timeScale`, `seaMoveU/V`
- `specialItemList[8]` — definitions of pickups (bonus, turbo, joker) in this world

### `TerrainMesh` and `SeaMesh`

Both are triangle meshes with vertices, a texture reference, a diffuse lighting coefficient, and a list of `ModelReference`s — 3D model placements within that mesh's area. The key flag is `kFlag_RemoveBF` for backface culling and `kFlag_Modulate` for color multiplication.

`SeaMesh` additionally has `lastUpdateTime` to track when its wave geometry was last updated.

### `RaceTrack` and `CheckPoint`

```cpp
typedef struct {
    Str31         name;
    OSType        id;         // 'Easy', 'Medi', or 'Hard'
    long          lapNumber;
    unsigned long checkCount;
    Check         checkList[];  // each check has left/right mark vectors + flags
} RaceTrack;
```

Checkpoints are pairs of world-space `Vector` positions (left and right track markers). From these, `World_Prepare()` constructs `RaceDoor` entries in the `RaceDescription`, computing door centers, widths, normal vectors, angles, and distances to the next door.

### `CollisionEnclosure`

A closed polygon of `CollisionWall`s defining an impassable boundary. Each `CollisionWall` stores left/right edge vectors plus a precomputed `inverseMatrix` for fast point-in-wall tests. Ships are tested against these to detect when they hit the track boundary.

### `SpecialItem`

```cpp
typedef struct {
    OSType  id;
    OSType  itemType;    // 'Bnus', 'Trbo', 'Jker'
    OSType  modelID;     // which model to render for this pickup
    long    itemValue;   // points awarded for bonus items
} SpecialItem;
```

Pickups float and spin above the water surface. Their positions are tracked in `itemReferenceList` (a sparse array of `SpecialItemReference`). When a ship collides with one it is temporarily hidden and respawned.

---

## 13. The Infinity Audio Engine

[Source/Infinity Engine/Core/Infinity Audio Engine.cp](Source/Infinity%20Engine/Core/Infinity%20Audio%20Engine.cp) implements 3D spatial audio on top of the Mac Sound Manager.

The engine maintains a list of active audio channels. Each channel has:
- Type: FX (one-shot), Loop, Random, AmbientFX, AmbientLoop, CinematicFX
- A world-space position and optional `MatrixPtr` for the listener
- Min/max audible distances
- Volume, pitch, panoramic (left/right balance)

`InfinityAudio_SetMatrix(MatrixPtr matrix)` is called every frame with the camera matrix. The engine uses the listener's position and orientation to compute distance and angle to each sound source, then applies inverse-square-law falloff between `minDistance` and `maxDistance` to set the Sound Manager channel volume and stereo pan.

Ship engine sounds are installed as `AmbientLoop` sources via `InfinityAudio_InstallSound_AmbientLoop()`, attached to the ship's position. Each frame the physics engine calls `InfinityAudio_SetSoundParams()` with the current engine pitch (proportional to motor power) and volume.

The interface (menus) uses `InfinityAudio_InstallObject()` which scans a `MegaObject` for shapes with `kShader_Audio` shading mode and installs each as a positional ambient sound.

---

## 14. Clocks and Timing

Three clock types in [Source/Infinity Engine/Clocks/](Source/Infinity%20Engine/Clocks/):

- **Variable clock** — measures wall-clock time between calls using `TickCount()` (60 Hz Mac OS timer). Returns elapsed ticks since last call. Used for animation timing.
- **Step clock** — a fixed-timestep accumulator. Each call advances by exactly one step unit regardless of wall-clock time. Used for deterministic physics.
- **Relative clock** — measures time relative to a base timestamp. Used to synchronize network game clocks.

`kTimeUnit = 100` is the simulation time unit per second (100 ticks/second). All game timers (race countdown, turbo duration, replay event spacing) are expressed in units of `kTimeUnit`.

---

## 15. The InfinityPlayer Subsystem

[Source/Infinity Engine/Core/Infinity Player.cp](Source/Infinity%20Engine/Core/Infinity%20Player.cp) is a self-contained animation player used by the menu system. It loads a MetaFile, creates a RAVE context, and drives a `MegaObject` script timeline.

`InfinityPlayer_Init(true)` allocates the RAVE rendering buffers (`InfinityRendering_AllocateBuffers()`) and initializes the audio engine (`InfinityAudio_Init()`).

`InfinityPlayer_LoadAnimation()` takes a file `FSSpec`, a texture set ID, screen position and size, and loads the MetaFile into the player's private `megaObject`. It resolves textures into `videoMemory` and finds the camera shape.

`InfinityPlayer_PlayAnimation(OSType animationID)` finds the named `Script` in the `megaObject` and starts it running.

`InfinityPlayer_RunAsync(float maxFPS)` is called each menu frame. It:
1. Calls the variable clock to get elapsed time
2. Advances the current script via `CinematicScript_Run()` (updates all shape/skeleton transforms from keyframes)
3. Calls `InfinityRendering_NoClear()` then renders the object
4. Calls `QARenderEnd()` and waits for the frame timing to maintain `maxFPS`

The menus use this to drive Leah's animated 3D model: different scripts (`'Strt'`, `'LpM1'`…`'LpM7'`, `'NwPl'`, etc.) correspond to different animations (idle loops, transitions, reactions) that play in response to user interaction.

---

## 16. Data Files and Asset Discovery

[Source/WaterRace final/Data Files.cp](Source/WaterRace%20final/Data%20Files.cp) handles the game's file system layout.

### Folder Resolution (`Resolve_DataFolders`)

Using `FSSpec` and `HGetVol`/`FSMakeFSSpec` to navigate relative to the application bundle:

```
<App folder>/
  Data/
    Interface          (resource fork: buttons, PITCs, strings)
    Sounds             (resource fork: SFX)
    Ships/             (MetaFile per ship)
    Locations/         (MetaFile per terrain)
    Characters/        (MetaFile per character)
    Pilots/            (Pilot save files)
    Musics/            (audio files or CD track list)
    Core/              (built-in protected content)
  Add-Ons/             (user-installed extra ships/terrains)
```

The folder names are stored as Pascal strings in string resource `129` of the main resource file, making them localizable.

### Preloading (`Preload_DataFolders`)

Scans each folder with `FSSpec` iteration. For each MetaFile found:
1. Opens the file and reads just the data sub-tags (`'shds'`/`'trds'`/`'chds'`) and preview pictures (`'Enbl'`, `'Dsbl'`, `'Prvw'`) without loading the full 3D data.
2. Stores the `FSSpec`, the four-char OSType ID, the description structure, and the preview `PicHandle`s into `ShipPreloadedData`, `TerrainPreloadedData`, or `CharacterPreloadedData`.
3. Populates `coreData` and `addOnData` (`DataSet` structs).

This gives the browser menus instant access to names, descriptions, and preview images without the cost of loading full 3D assets.

The `kSkipChar` (`'_'`) prefix on a filename causes it to be skipped, allowing assets to be temporarily disabled.

---

## 17. Preferences

[Source/WaterRace final/Preferences.cp](Source/WaterRace%20final/Preferences.cp) saves and loads a preference file in the system Preferences folder using `FindFolder(kOnSystemDisk, kPreferencesFolderType, ...)`.

The `Preferences` struct (defined in `Preferences.h`) includes:
- `renderWidth/Height` — 3D rendering resolution (can differ from interface resolution for performance)
- `engineFlags` — bitfield: filtering, 32-bit rendering, 32-bit z-buffer, low resolution, video FX, further clipping, tri-linear filtering, software engine
- `textureFlags` — mipmap mode
- `interfaceScreen` and `gameScreen` — saved `VDSwitchInfoRec` for the chosen display mode
- `screenID` — Display Manager display ID
- `soundFXVolume`, `musicVolume`
- `raveEngineName` — optional override for the RAVE engine
- ISp control mappings

On first run the file does not exist; `Preference_Read()` returns an error code that `Boot_Sequence()` ignores (it creates defaults), and `Preference_Write()` at quit saves the new file.

---

## 18. Pilot Profiles

[Source/WaterRace final/Pilots.cp](Source/WaterRace%20final/Pilots.cp) manages per-player save files in the Pilots folder.

```cpp
typedef struct {
    unsigned long version;
    Str31         name;
    OSType        character;       // which character model to use
    OSType        mode;            // Tournament, TimeRace, Duel, FreePlay, AddOns
    short         difficulty;      // Easy, Medium, Hard
    unsigned long creationDate, lastPlayDate, playTime;
    OSType        status;          // Alive, Dead, Challenge, Champion
    long          points, numJokers;
    OSType        opponents[8];    // for Duel mode
    unsigned long raceCount;
    Race          raceList[16];    // per-location race history
} Pilot;
```

Each `Race` entry records `totalTime`, `bestLapTime`, `score`, `rank`, `maxSpeed`, and `totalDistance` for one completed race at one location.

`Pilot_Save()` writes the struct directly to an FSSpec file. `Pilot_Load()` reads it back and validates the version field. This is raw binary serialization — no encoding — which is why `#pragma options align=mac68k` padding matters.

---

## 19. The Interface System

The interface spans multiple files. The coordinator is [Source/WaterRace final/Interface.cp](Source/WaterRace%20final/Interface.cp).

### Interface Resolution

The interface always runs at 640×480 at 16-bit regardless of the game rendering resolution, as defined in `Interface.h`:
```cpp
#define kInterface_Width   640
#define kInterface_Height  480
#define kInterface_Depth   16
```

### Leah

"Leah" is the animated 3D hostess character present throughout the menus. She is a `MegaObject` loaded from a MetaFile (`leahFileName` in the data folder). `Interface_GetLeahSpec()` finds the file. `Interface_LoadAnimationWrapper()` loads it into InfinityPlayer for a given screen position and camera.

Her behavior is entirely driven by named Scripts embedded in the MetaFile:
- `'Strt'` — startup entry animation
- `'LpM1'`–`'LpM7'` — idle loop variations (randomly cycled)
- `'NwPl'`, `'LdPl'`, `'NtGm'`, `'Optn'`, `'Crdt'`, `'Quit'` — reaction animations for each menu button
- `'NEnd'`, `'NBTr'` — new-pilot flow transitions
- `'CrSt'`, `'CrRn'`, `'CrQt'` — credits sequence

### Buttons

Buttons are defined in a `'Btns'` resource (custom resource type). Each `ButtonDefinition` has a hotzone `Rect`, a draw position `Point`, and over/down PICT resource IDs. `Interface_ButtonDraw()` blits the appropriate PICT into `bufferGWorld` using CopyBits, then blits the GWorld to `mainWin`.

### Menu Flow

`Interface_Display()` is the top-level loop:

1. Call `MainMenu_Display()` — runs the main menu, returns which button was clicked
2. Based on the button:
   - **New Pilot**: `NewMenu_Start()` + `NewMenu_Run()` (choose difficulty/mode) + `PilotMenu_Display()` (enter name) + `CharacterMenu_Display()` (pick character) → `Interface_RunLocalGame()`
   - **Load Pilot**: `PilotMenu_Display()` (load existing) → same path
   - **Network**: `NetworkMenu_Display()` → `Game_New()`
   - **Options**: options dialog (video/audio settings)
   - **Credits**: `Credits_Display()`
   - **Quit**: exit loop

`Interface_RunLocalGame()` calls `Browser_Display()` to let the player pick a location and ship, then calls `Game_SetupLocal()` and `Game_New()` to start the race.

---

## 20. World Loading: `World.cp`

`World_Load(StatePtr state, FSSpec* terrainFile, long level, OSType flyThroughID)` loads the entire race environment:

1. **MetaFile read**: calls `MetaFile_ReadTerrain()` to parse the terrain MetaFile into the global `theTerrain` structure.
2. **Texture resolve**: `Terrain_Resolve()` (in `Local Infinity Utils.cp`) iterates all meshes and model references, uploads their textures to `videoMemory`, and patches `texturePtr` pointers.
3. **Model loading**: for each `ModelAnimation` in the terrain, loads the referenced model MetaFile and resolves its textures.
4. **Sky setup**: creates the sky dome geometry (a sphere with the `'sky '` texture).
5. **Collision map**: builds the 2D `collisionMap` grid. Each world-space cell of size `kCollisionMapResolution = 15.0` units stores a list of `ModelReference` pointers for objects in that cell, enabling O(1) broad-phase collision lookup.
6. **Item references**: resolves `itemReferenceList` — the positions of all pickup items.
7. **Audio**: calls `World_InstallAudio()` which installs all `Source` and `AmbientSound` entries into the audio engine.

`World_InstallShip(FSSpec* shipFile, PlayerConfigurationPtr pilotConfig, ...)` for each player:
1. Loads the ship MetaFile into an `ExtendedShip` entry in `shipList[]`
2. Resolves ship textures from `videoMemory`
3. Finds named shapes within the MegaObject by OSType ID: `'Head'`, `'Body'`, `'Prp1'`-`'Prp4'` (propellers), `'Rdr1'`-`'Rdr2'` (rudders), `'Flme'`/`'Flm2'` (turbo flames)
4. Finds named scripts: `'play'` (autorun idle animation), `'TbOn'`/`'TbOf'` (turbo on/off)
5. Installs engine and turbo loop sounds from the MetaFile's embedded audio data
6. Calls `Physic_InitShip()` to set up the `Dynamics` struct
7. Assigns the appropriate ship driver callback and camera driver

`World_Prepare()` computes `RaceDescription` from the terrain's `RaceTrack`:

```
for each checkpoint pair:
    door.middle = midpoint(leftMark, rightMark)
    door.doorWidth = distance(leftMark, rightMark)
    door.doorAngle = atan2(direction to next door)
    door.distanceToNextDoor = distance(middle, next.middle)
    precompute door.doorMatrix, door.negatedDoorMatrix
```

This race description is used by both the checkpoint detection code and the bot AI.

`World_UpdateSea()` called every frame updates the `SeaMesh` vertex heights using:
```cpp
#define WaveHeight(t, x, z, sx, sz, a) \
    ((TCos(z/sz + DegreesToRadians(t)) + TSin(x/sx + DegreesToRadians(t))) / 2.0 * a)
```
where `t` is current time, `sx/sz` are `theTerrain.waveScaleX/Z`, and `a` is `theTerrain.waveAmplitude`.

---

## 21. Physics Engine: `Physic Engine.cp`

[Source/WaterRace final/Physic Engine.cp](Source/WaterRace%20final/Physic%20Engine.cp) is the full hydrodynamics simulation.

### `Dynamics` and `Volume`

Each ship has a `Dynamics` struct:

```cpp
typedef struct {
    Vector  velocityG;           // velocity in world (global) space
    Vector  accelerationG;       // acceleration in world space
    Vector  rotationspeedL;      // angular velocity in ship (local) space
    Vector  rotationAccelerationL;
    float   targetPower, motorPower;
    short   status;              // UnderWater, OnWater, InAir
    long    upsideDownTime;      // time spent capsized (triggers reset)
    long    underWaterTime;      // time submerged (triggers reset)
    long    collisionSoundTime;  // debounce for collision audio
    Boolean colliding;
    Volume  volumeList[4];       // 4 hull volumes for buoyancy
} Dynamics;
```

`Volume` represents one of four hull cross-sections (front-left, front-right, rear-left, rear-right). Each has a bounding box, mass, frontal/floor/lateral areas, Archimedes correction factor, and the current `submergence` depth (how far it is below the water surface).

### `Physic_UpdateShip(ship, time)`

The main physics step. Called once per frame per ship:

**1. Driver input** — reads `ship->mouseH` (steering) and `ship->dynamics.targetPower` (set by the driver callback). Motor power is slew-rate limited toward target: `motorPower += (targetPower - motorPower) * power_ChangePerSec * dt`.

**2. Buoyancy** — for each of the 4 hull volumes:
- Transform the volume's center to world space
- Compute wave height at that position using `WaveHeight()`
- Compute submergence depth = waveHeight - volume.center.y
- Compute Archimedes force = `submergence × volume.volume × waterDensity × gravity × archimedeCorrectionFactor`
- Accumulate upward force and pitch/roll torques

**3. Propulsion** — compute thrust force from current `motorPower` along the ship's forward vector (or a directional vector if `kDirectionalPropulsion`). Turbo adds a large impulse in the forward direction.

**4. Hydrodynamic drag** — decompose velocity into ship-local axes, then apply:
- Frontal drag: `F = -Cx_H × area_frontal × v_forward²`
- Lateral drag: `F = -Cx_L × area_lateral × v_lateral²`
- Vertical drag (when airborne): `F = -Cx_V × area_floor × v_vertical²`
- Wave drag `Cw`, friction `Cf`

**5. Rotational drag** — `torque = -Cx_R × area_rotational × omega²` applied to each axis.

**6. Gravity** — `accelerationG.y -= 9.81 × mass` when in air.

**7. Integration** — Euler: `velocity += acceleration × dt`, `position += velocity × dt`.

**8. Orientation update** — integrates `rotationspeedL` into the ship's rotation matrix using quaternion multiplication (quaternion is built from the angular velocity vector × dt, then applied to the current orientation quaternion, then converted back to a matrix).

**9. MegaObject update** — stores new world position and rotation matrix into `ship->megaObject.object.pos`.

**10. Visual animation** — rotates propellers around their axes, tilts rudder based on steering input, tilts pilot head, shows/hides turbo flame shapes.

**11. Wake** — updates wake sprite positions: new sprites are spawned at the ship's stern position periodically and drift backward, fading out over `wakeLifeTime`.

**12. Reset detection** — if the ship has been upside down (`upsideDownTime`) or underwater (`underWaterTime`) for too long, calls `Physic_DropShip()` to teleport it back to the last checkpoint.

---

## 22. Collision Engine: `Collision Engine.cp`

[Source/WaterRace final/Collision Engine.cp](Source/WaterRace%20final/Collision%20Engine.cp) handles two categories of collision.

### Terrain Boundary Collision (`ShipEnclosure_Collision`)

The terrain's `CollisionEnclosure` arrays define closed boundary polygons (e.g., the track edges and island shores). For each ship each frame:

1. The ship's current grid cell is looked up in `collisionMap`.
2. For each `CollisionEnclosure` referenced by nearby cells, `ShipEnclosure_Collision_WithNormal()` tests the ship's bounding sphere against each `CollisionWall` using the wall's precomputed `inverseMatrix`.
3. If penetrating, a reflection impulse is computed: the velocity component along the wall normal is reversed and scaled by a restitution coefficient. A collision sound is played (with `collisionSoundTime` debounce).

### Object Collision (`Object_Collision`)

Tests two `MegaObject`s' `CollisionBox` (OBB) lists against each other. The Separating Axis Theorem is applied: for each of the 15 potential separating axes (3 face normals × 2 objects + 3×3 edge cross products) compute the projection extents. If any axis separates the boxes, no collision. If all axes overlap, a collision is detected.

`Object_Collision_WithNormal()` additionally computes the penetration normal for the response.

Ship-vs-ship collisions use this to exchange momentum when two boats collide.

### Bonus Pickup Collision (`Check_BonusCollision_Local`)

A simple sphere-vs-sphere test between the player's ship radius and each active `SpecialItemReference` position. On hit:
- **Turbo (`'Trbo'`)**: sets `ship->turboButton = true`, plays turbo sound, plays the turbo script on the ship
- **Joker (`'Jker'`)**: increments `player->hasJoker`
- **Bonus (`'Bnus'`)**: adds to `player->score`

The item is deactivated by removing it from the active reference list and respawned after a delay.

---

## 23. Ship and Camera Drivers

Drivers are hot-swappable callback function pointers set on each ship and on the camera. The indirection allows the same physics code to serve human players, bots, network clients, and replay playback.

### Ship Drivers (`Drivers Ship.cp`)

**`DriverShip_PlayerISp_Callback(ship)`** — the human player driver:
- Reads `ISpElement_GetSimpleState()` for throttle forward/reverse, turbo button
- Reads mouse H delta (or ISp analog steering element) for `ship->mouseH` (steering input)
- Clamps steering to `[-1, 1]`
- Sets `ship->dynamics.targetPower` and `ship->turboButton`

**`DriverShip_Bot_Callback(ship)`** — the AI driver, parameterized entirely by `ship->params` (the `Data_ShipPhysicParameter` loaded from the MetaFile). Every AI tuning value is data-driven:
1. Find the target door: `ship->nextDoor` index into `theRace->doorList`
2. Compute angle to door center from ship's heading
3. Steering: if `angle > bot_Turn_TargetToleranceAngle`, steer toward target. The amount of steering scales with angle between `bot_Turn_MinTargetAngle` and `bot_Turn_Max`.
4. Power: reduce power when making sharp turns (`bot_Power_LowPowerTurnAngle`), reduce power in air (`bot_Power_InAir`)
5. Turbo: fire turbo when aligned to door within `bot_Turbo_MaxAngle` and far enough away (`bot_Turbo_MinDistance`), but not if something is in the way (`bot_Turbo_CollisionDistance`)
6. Anticipation (`bot_Anticipation`): look ahead past the current door to the next one for smoother lines

**`DriverShip_Remote_Callback(ship)`** — applies network state from `networkShipBuffers[ship->shipNum]`:
- Copies velocity, acceleration, rotation speed from the last received `ShipState` packet
- Applies the received position and orientation directly (with small smoothing)

**`DriverShip_Replay_Callback(ship)`** — replays from `ship->eventBuffer[]`:
- Finds the `ReplayEvent` matching the current frame time
- Interpolates position and orientation from surrounding events
- Restores motor power and turbo state

### Camera Drivers (`Drivers Camera.cp`)

The global `cameraDriver` function pointer is called once per frame in the game loop to update `worldCamera`.

**`DriverCamera_Follow_Callback`** — the default follow camera: the camera position lerps toward a point `N` meters behind and above the local player's ship, with damping. `worldCamera.camera` is rebuilt from the resulting position and look-at direction. Camera height adjusts based on `kCameraMinHeight/MaxHeight`.

**`DriverCamera_Chase_Callback`** — tighter follow with faster response, oriented along the ship's velocity vector rather than its heading.

**`DriverCamera_PilotHead_Callback`** — places the camera at the `'Head'` shape's world-space position, oriented along the ship's forward vector.

**`DriverCamera_OnBoard_Callback`** — uses a fixed camera position from the ship's `cameraList[]` (loaded from MetaFile).

**`DriverCamera_TV_Callback`** — cycles through the terrain's fixed `cameraList[]` positions, cutting to whichever camera best frames the lead ship.

**`DriverCamera_FlyThrough_Callback`** — plays back the `flyThroughCameraAnimation` timeline from the terrain's `ModelAnimation` that has the `kFlyThroughID` id.

**`DriverCamera_Track_Callback`** — uses the track camera from the terrain, computing which fixed camera is best oriented toward the local ship.

---

## 24. InputSprocket Integration: `ISpRoutines.cp`

[Source/WaterRace final/ISpRoutines.cp](Source/WaterRace%20final/ISpRoutines.cp) wraps Apple's InputSprocket API.

`InputSprocket_Init()`:
1. Calls `ISpStartup()` to initialize the ISp subsystem
2. Defines a list of needs (abstract game actions): forward, backward, steer left/right, turbo, camera change, screenshot
3. Calls `ISpNeed_NewVirtualDevice()` and `ISpElement_NewVirtual()` to register these
4. Loads the ISp setup resource from the main resource file for default bindings
5. Calls `ISpInit()` to finalize

The mouse H delta is read separately each frame: `GetMouseCoords()` reads the raw hardware mouse displacement and `MoveMouseToCenter()` re-centers it, giving relative mouse movement as the primary steering input.

For joystick/gamepad support, ISp elements provide analog values via `ISpElement_GetSimpleState()` in fixed-point format, normalized to `[-1, 1]` range for steering.

---

## 25. The Game Loop: `Game.cp`

`Game_New()` in [Source/WaterRace final/Game.cp](Source/WaterRace%20final/Game.cp) orchestrates a complete race.

### Setup Phase

1. Show progress bar (`Progress_Init`/`Progress_Update`)
2. Find the terrain MetaFile by `terrainID` in the appropriate `DataSet`
3. `World_Load()` — load terrain, textures, models
4. `Infinity_GameInit()` — create RAVE rendering context for game resolution
5. `Infinity_SetTerrainState()` — apply lighting, fog, clipping to the render state
6. For each player in `gameConfig.playerList`:
   - Find ship MetaFile by `shipID`
   - `World_InstallShip()` — load ship, set up physics, assign driver
7. Network multiplayer: `Server_SetupGame()` or `Client_SetupGame()` — exchange player lists, synchronize clocks
8. `World_Prepare()` — build `RaceDescription` race doors
9. `World_InstallAudio()` — start ambient and world sounds
10. Load game SFX from resource file (countdown, checkpoint, finish, etc.)
11. Position camera and display countdown

### Main Race Loop

```cpp
while(run) {
    // Time update
    currentTime = GetElapsedTime();

    // Drive ships
    for each ship:
        ship.DriverShipRoutine(ship)  // input or AI

    // Physics
    for each ship:
        Physic_UpdateShip_Start(ship)  // read driver output
        Physic_UpdateShip(ship, dt)    // simulate
        Physic_UpdateShip_End(ship)    // apply to MegaObject

    // Collision
    for each ship:
        ShipEnclosure_Collision(ship)  // boundary walls
    for each ship pair:
        Object_Collision(ship_a, ship_b)  // ship-ship
    Check_BonusCollision_Local(localShip)

    // Camera
    cameraDriver()

    // Network send
    if multiplayer:
        Server_SendData(localShip) or Client_SendData(localShip)

    // World animation
    World_UpdateSea()
    ModelAnimation_Update(theTerrain.animList)
    Script_Update(ships.autoRun/currentScript)

    // Render
    QARenderStart(worldState->drawContext)
    Draw_Frame(worldState)
    QARenderEnd(worldState->drawContext)
    CopyBits(worldState->drawContext->pixMap, mainWin->portPixMap)

    // Race state
    CheckCheckpoints()
    Game_UpdateRank()

    // Check finish
    if all ships finished: break
    if forceQuit: break
}
```

### Checkpoint Detection

Each frame, for each ship, the code tests whether the ship's position has crossed its `nextDoor`:
- Transform ship position into the door's local coordinate system using `door.negatedDoorMatrix`
- If the z-coordinate is positive (ship passed through the plane) and x-coordinate is within `door.doorWidth/2`, the door is cleared
- Advance `ship->nextDoor`; if `nextDoor` wraps past the last door, increment `player->lapCount`
- Record `checkPointTime` for per-door timing

### Rank Sorting (`Game_UpdateRank`)

Ranks players by a composite key: `lapCount × (totalDoors) + doorsCompleted`. Ships that have finished are sorted by their `finishTime`.

### Post-Race

After the loop:
1. Stop all audio
2. Unload the world (`World_Unload`)
3. Record replay (`Replay_Save`) if `__RECORD_RACE__`
4. `PlayMode_UpdatePilot()` — update pilot statistics, points, jokers
5. `ScoreMenu_Display()` — show race results
6. `ChampionMenu_Display()` if applicable (tournament champion)
7. `Pilot_Save()` — persist pilot data
8. Return to `Interface_RunLocalGame()` loop for next race

---

## 26. Drawing a Frame: `Drawing.cp`

`Draw_Frame(StatePtr state)` in [Source/WaterRace final/Drawing.cp](Source/WaterRace%20final/Drawing.cp) renders the complete frame in a fixed sequence using the current `worldCamera`.

The camera matrix `worldCamera.camera` is the camera's world-space transform. The renderer needs its inverse (`negatedCamera`), computed via `Matrix_Negate()` before the draw call, plus `cameraPos` extracted from the translation row.

### Render Order

1. **Sky** — `Terrain_RenderSky()` renders the sky dome sphere around the camera position. The sky never translates, only rotates with the camera, so it appears infinitely distant.

2. **Terrain meshes** — `Mesh_RenderModels()` per `TerrainMesh`. For each mesh:
   - Backface cull test on the mesh bounding box
   - `Terrain_RenderMesh_Textured()` submits the mesh's vertex array directly to RAVE (terrain geometry doesn't go through the per-shape shader pipeline — it's submitted as large triangle meshes for efficiency)
   - For each `ModelReference` in the mesh, call `Shape_CallRenderingProc` on each shape of the referenced model's `MegaObject`

3. **Sea meshes** — `SeaMesh_RenderModels()` per `SeaMesh`. Sea geometry was updated by `World_UpdateSea()` earlier in the frame. Floating model references bob with `SeaMesh_AnimateModels()`.

4. **Special items** — `Render_SpecialItems()`: for each active item reference, rotate the item model around Y, apply the floating sine offset, then render.

5. **Ships** — for each ship in `shipList[]`:
   - Skip if the ship's bounding sphere is outside the view frustum
   - Render the ship's `MegaObject` (all shapes, including propellers, rudder, pilot, turbo flames)
   - Render wake sprites using `Texture_DrawSprites()`

6. **Post-FX** — if `videoFX` is enabled: `InfinityPostFX_RenderLensFlare()`, `InfinityPostFX_RenderMoon()`, `InfinityPostFX_RenderRain()`, `InfinityPostFX_RenderSnow()`

7. **HUD** — drawn using `QARenderStart`/`QARenderEnd` with `kQATag_ZFunction = kQAZFunction_None` (no depth test):
   - **Minimap**: `Gouraud_DrawLine()` to draw the track outline as a scaled 2D projection onto a 150×150 area in the top-right corner. Ship positions are drawn as small crosses with player-color coding.
   - **Speed**: `FastNumToString*` functions convert speed to characters; `TextEngine_DrawString()` blits to screen at fixed coordinates.
   - **Position / lap**: similar fast int-to-string conversion
   - **Race timer**: `RaceTimeToString()` inline function formats as `M:SS'HH`
   - **Countdown**: if `currentTime < raceStartTime`, displays the countdown digit with a scale effect
   - **Info text**: `player->infoText` overlay for checkpoint messages, bonus notifications, etc.
   - **GameOver text**: `GameOverText()` when the race is finished

### Text Engine

`Text Engine.cp` implements a bitmap font renderer for the HUD. `TextEngine_Init()` loads a monospace bitmap font from a PICT resource and slices it into a character table. `TextEngine_DrawString()` blits each character's pixel data directly into the RAVE back-buffer pixel map using the low-level `TQABitmap` mechanism or direct pixel writes, bypassing QuickDraw entirely for speed.

---

## 27. Post-FX

[Source/Infinity Engine/Post FX/](Source/Infinity%20Engine/Post%20FX/) contains visual effects rendered after the main 3D scene:

- **Lens Flare** (`Infinity Post FX LensFlare.cp`): Tests if the sun direction is within the view frustum. If visible, projects several billboard sprites along the line from center to sun position, with varying sizes, transparencies, and colors. Uses `Texture_DrawSpriteOnScreen()` directly in screen space.

- **Moon** (`Infinity Post FX Moon.cp`): Renders a single billboard sprite at the moon's direction vector, scaled to `skyRadius`. Uses the sky sphere's rotation to position it.

- **Rain** (`Infinity Post FX Rain.cp`): Maintains a pool of `InfinitySprite` particles moving downward. Each frame advances their position by velocity × dt, wraps when they fall out of the view volume, and renders them as short vertical line sprites.

- **Snow** (`Infinity Post FX Snow.cp`): Similar to rain but with slower, slightly drifting particles.

- **Text** (`Infinity Post FX Text.cp`): Screen-space subtitle text overlay for cinematic sequences.

---

## 28. The Network Engine

The network layer spans three files: [Network Engine.cp](Source/WaterRace%20final/Network%20Engine.cp) (shared init/utils), [Network Engine Server.cp](Source/WaterRace%20final/Network%20Engine%20Server.cp), and [Network Engine Client.cp](Source/WaterRace%20final/Network%20Engine%20Client.cp).

### Open Transport

Apple Open Transport is the Mac OS 9 TCP/IP stack. Key types:
- `EndpointRef` — a TCP connection endpoint
- `InetAddress` — an IP address + port
- `OTResult` — error/status code

The game uses non-blocking (async) OT calls with polling rather than callbacks, checking `OTLook()` and `OTRcvOrderlyDisconnect()` in the game loop.

### Protocol

All packets begin with an OSType `packetType` field followed by payload:

| Packet | Direction | Purpose |
|---|---|---|
| `'ClIf'` ClientInfo | Client→Server | Player name, ship choice, game version, password |
| `'SvIf'` ServerInfo | Server→Client | Game config, terrain, welcome message |
| `'GmIf'` GameInfo | Server→Client | Full player list once all connected |
| `'SvCl'` ServerClock | Server→Client | Clock synchronization ticks |
| `'PlRd'` PlayerReady | Client→Server | Client confirms ready to race |
| `'SvRd'` ServerReady | Server→Client | Server confirms all ready |
| `'SvST'` ServerStartTime | Server→Client | Absolute start timestamp |
| `'ShSt'` ShipState | Both | Per-frame ship position/velocity |
| `'DcRs'` DisconnectionReason | Server→Client | Why client was kicked |

### `ShipState` (20Hz, ~160 bytes per ship)

```cpp
typedef struct {
    OSType    packetType;       // 'ShSt'
    Vector    velocityG;        // world velocity
    Vector    accelerationG;
    Vector    rotationspeedL;
    Vector    rotationAccelerationL;
    Vector    position;         // world position
    Quaternion orientation;     // ship orientation
    unsigned long timeStamp;
    short     motorPower;       // fixed-point, needs conversion
    short     mouseH;           // steering input
    unsigned char turboState;
    unsigned char shipNum;      // which ship slot
} ShipState;
```

### Server Flow

`Server_Start()` opens a listening TCP endpoint. As clients connect (`Server_WaitForClients()`), the server:
1. Sends `ServerInfo` to each new client
2. Receives `ClientInfo` from each
3. Validates version and password; disconnects invalid clients
4. Once enough players connect and all are ready, builds `GameInfo` and broadcasts it
5. Sends `ServerStartTime` with a synchronized clock timestamp
6. Enters `Server_Enter()` — the race data loop, sending/receiving `ShipState` at 20Hz

During the race, `Server_SendData(localShip)` broadcasts the server's own ship state to all connected endpoints. Incoming `ShipState` from clients is written into `networkShipBuffers[shipNum]`.

### Client Flow

`Client_Connect()` opens a TCP connection to the server IP:port, sends `ClientInfo`, receives `ServerInfo`. `Client_WaitForServer()` polls for `GameInfo`, then `ServerReady`, then `ServerStartTime`.

During the race, `Client_SendData(localShip)` sends the client's own state, and the server's broadcasts are received into `networkShipBuffers`.

### Error Handling

Version mismatch (`kNetError_Client_BadVersion`), wrong password (`kNetError_Client_WrongPassword`), or a game already in progress (`kNetError_Client_GameStarted`) all result in clean disconnection with an error message.

---

## 29. Play Modes and Progression

[Source/WaterRace final/Local Play Modes.cp](Source/WaterRace%20final/Local%20Play%20Modes.cp) implements game mode rules.

### Modes

- **Tournament (`'Trnm'`)**: The main campaign. Tracks are unlocked by earning enough points. `PlayMode_GetLocationAccess()` checks pilot points against location thresholds. A "challenge" location is unlocked when the pilot earns enough for the next tier. `PlayMode_CheckForChallenge()` detects this and transitions the pilot to `kStatus_Challenge` or `kStatus_Champion`.
- **TimeRace (`'TmRc'`)**: All locations accessible, race against par time. `gameConfig.expiredTime` counts down from the terrain's `parTime[difficulty]`.
- **Duel (`'Duel'`)**: One-on-one against a specific bot character. Only that character's terrain/ship is used.
- **FreePlay (`'FrPl'`)**: All locations and ships accessible, no scoring, no save.
- **AddOns (`'AdOn'`)**: Uses `addOnData` instead of `coreData` for the location/ship lists.

### Point System

`PlayMode_UpdatePilot()` after each race:
- Computes points from race `rank`, `score`, and time bonuses/penalties
- Calls `PlayMode_GetDefaultBonus()` for mode-specific starting bonuses
- Adds points and jokers to `pilot->points` and `pilot->numJokers`
- Calls `PlayMode_CheckIfPilotStuck()` to detect impossible situations and give the pilot a free joker

### Access Control

`PlayMode_GetShipAccess()` and `PlayMode_GetLocationAccess()` return one of:
- `kAccess_OK` — available normally
- `kAccess_OK_Joker` — available by spending a joker
- `kAccess_NotEnoughPoints` — still locked
- `kAccess_NeedJoker` — locked but a joker would help
- `kAccess_Disabled` — not applicable in current mode

The browser (`Browser.cp`) grays out or hides items based on this return value.

---

## 30. The Replay System

[Source/WaterRace final/Replay.cp](Source/WaterRace%20final/Replay.cp) records and plays back races.

### Recording

When `__RECORD_RACE__` is compiled in, `Physic_UpdateShip_End()` checks whether `currentFrame % kReplayEventDistance == 0` (recording at 25Hz) and if so appends a `ReplayEvent` to `ship->eventBuffer[]`:

```cpp
typedef struct {
    unsigned short timeStamp;
    short          motorPower;
    short          mouseH;
    Boolean        turboState;
    Vector         position;
    Quaternion     orientation;
} ReplayEvent;
```

Maximum 7500 events per ship (~5 minutes of racing at 25Hz). The buffer is allocated from `NewPtr` at game start.

`Replay_Save()` writes all recorded events to a MetaFile with type `'RPly'`. The file can be drag-dropped onto the application icon to play back (the `ODOC_Handler` Apple Event handler in `main.cp` detects this).

### Playback

`Replay_SetupGame(FSSpec* file)` reads the MetaFile, allocates event buffers, and sets all ships to use `DriverShip_Replay_Callback`. During playback, the driver interpolates between recorded keyframes by finding the events bracketing the current `frameTime` and linearly interpolating `position` and SLERPing `orientation`.

---

## 31. Common Code Utilities

### Screen Management (`ScreenLib.cp/.h`)

`SL_ScanGDevice()` enumerates all video modes for a `GDHandle` using Display Manager `DMGetDisplayModeParameters()`. `SL_GetDeviceResolution()` presents a dialog for manual selection. `SL_Switch()` calls `DMSetDisplayMode()` to change the video mode. `SL_PickUpScreen()` presents a dialog listing all monitors for multi-monitor selection.

### Gamma / Screen Fades (`ColorGamma.cp/.h`)

`StartFading()` captures the current gamma table. `Fade1ToBlack()` smoothly sets the gamma ramp to all zeros over multiple ticks using `SetDeviceAttribute()` with `gdDevType` gamma data. `FadeToGamma()` restores the original table. Used at startup and shutdown for smooth black transitions.

### Audio Files (`QDesign Player.cp`, `MP3 Player.cp`, `Audio CD.cp`)

Three music backends selectable by compile flags:
- `__USE_AUDIO_CD_TRACKS__`: plays audio from CD tracks via the audio CD driver
- `__USE_AUDIO_FILES__`: plays QDesign-compressed audio files via QuickTime's QDesign codec
- Neither: no background music

`AudioCD_PlayAudioTrack_Wrapper()` in `Utils.cp` finds the correct CD track from the music file name mapping.

### Error System (`Error system.cp`)

`Wrapper_Error_Display(short actionID, OSErr errorID, ...)` looks up a localized error string using the `actionID` index into a string resource list, appends the numeric `errorID`, and displays a `StandardAlert()`. If `fatal = true`, calls `ExitToShell()` after the alert.

### Navigation Services (`Navigation Services.cp/.h`)

Wraps the Navigation Services API for modern open/save dialogs (introduced in Mac OS 8.5), used for the replay file open dialog in movie-capture mode.

### Text Engine (`Text Engine.cp/.h`)

`TextEngine_Init(short maxChars, short screenWidth)` loads a bitmap font from resource. `TextEngine_DrawString()` renders a Pascal string directly into the RAVE draw context's pixel buffer, bypassing QuickDraw entirely. Used exclusively for the in-game HUD.

---

## 32. Compile-Time Configuration

[Source/WaterRace final/Conditionals Headers/](Source/WaterRace%20final/%20Conditionals%20Headers/) contains a matrix of configuration headers. `Conditionals.h` is the master include that selects among them. Different builds include different files:

- `Conditionals - GM - English/French/German/Italian.h` — retail builds per language
- `Conditionals - Demo.h`, `Conditionals - Demo - FCH.h`, `Conditionals - Demo - GRM.h` — limited demo builds
- `Conditionals - Network Client.h` — network-only client build
- `Conditionals - Debug.h` — debug build with additional rendering overlays
- `Conditionals - Bots.h` — bot/AI testing configuration
- `Conditionals - Expo.h` — kiosk/expo mode (auto-starts a fixed game)
- `Conditionals - Movie.h` — screenshot capture mode for creating trailers
- `Conditionals - VR.h` — VR goggles output mode

Each header defines macros like:
```cpp
#define __DEMO_MODE__                   0  // or 1
#define __RECORD_RACE__                 1
#define __REPLAY_AWARE__                1
#define __CD_PROTECTION__               1
#define __USE_AUDIO_CD_TRACKS__         1
#define __USE_AUDIO_FILES__             0
#define __SCREEN_FADES__                1
#define __CHECK_FOR_ATI_DRIVERS__       1
#define __BOTS_CAN_PICKUP_TURBOS__      0
#define __AUTOPILOT__                   0
#define __NETWORK_ONLY_IF_CD_FAILURE__  1
#define __UNIT_SYSTEM__                 kUnit_Meters
#define __PATCH_PREFERENCES__           0
```

This system allows a single source tree to build the full game, the demo, a network-only client, a movie capture tool, and kiosk variants without any source changes — just by including a different conditionals file.

---

## 33. Shutdown

`Shut_Down()` in `main.cp` reverses the startup in order:

1. Fade the screen to black (optional, `__SCREEN_FADES__`)
2. `SpinningCursor_Quit()`
3. `TextEngine_Quit()` — free font data
4. `Sound_QuitSFx()` — release SFX channels
5. `InfinityPlayer_Quit()` — dispose RAVE contexts, free render buffers, stop audio engine
6. `Network_Quit()` — close Open Transport endpoints
7. `InputSprocket_Quit()` — unregister ISp
8. `Screen_Quit()` — restore original video mode, dispose windows and GWorlds, restore gamma
9. `AudioCD_Stop()` or `QDPlayer_Stop()` — stop background music
10. `InitCursor()` — restore arrow cursor
11. `Close_DataFiles()` — close resource forks
12. `Preference_Write()` — save preferences
13. `Preload_ReleaseMemory()` — free preloaded asset metadata
14. `ExitToShell()` — return control to the Finder

The `__SECURE_CRASH__` build adds a `terminate()` handler (CodeWarrior's C++ exception termination hook) that performs the same cleanup if the program crashes, ensuring the screen resolution is always restored even after an unhandled exception. The `properlyQuit` flag prevents the crash handler from running cleanup twice if the normal shutdown path was already taken.
