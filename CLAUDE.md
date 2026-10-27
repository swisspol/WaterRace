Read "_OSX_PORT_/WALKTHROUGH.md" to learn a linear walkthrough of the original Mac OS 9 source code (`Source/`). It intentionally covers only the original OS 9 code — do not add port details there; document the macOS port in this file instead.

# Repository overview

- `Source/` — the original 2001 Mac OS 9 game: C++, CodeWarrior, Apple RAVE/Open Transport/InputSprocket. It can't be built on modern macOS. Treat it as **read-only reference**. WALKTHROUGH.md documents this tree.
- `WaterRace Data/` — the original game data: MetaFiles (Ships, Locations, Characters, Core) plus `Interface`/`Sounds` resource forks saved as `*.rsrc_fork`. It is the source of truth for all extracted or ported assets.
- `_OSX_PORT_/` — the in-progress modern macOS port and the asset tooling. **All active work happens here.**

# The macOS port (`_OSX_PORT_/MacOS`)

A Swift Package (macOS 13+). It has two targets:
- `Sources/MacOS/main.swift` — the AppKit bootstrap. It sets up a 640×480 window and menu, loops `Music.mp3` with AVAudioPlayer, and hosts `LogoIntroView`.
- `Sources/MetalRenderer/MetalRenderer.m` (one large Objective-C file, ~2300 lines) — `LogoIntroView`, a Metal + CVDisplayLink renderer that **parses the original MetaFiles directly at runtime**. It does not use the glTF exports. It contains:
  - Big-endian readers (`mf_u32`/`mf_f32`/`mf_4cc`), the tag-table walk, and PICT/PackBits/QuickTime-JPEG texture decoding.
  - `loadAssets` (Logo Intro scene + `kType_Animation` keyframes + lens flares), `loadModelFromPath` (ships/characters, skeleton skinning via `parseSkeletonAnimFromData`/`updateAnimBufs`), and `loadTerrainFromPath` (`terr` files: `tmsh`/`smsh` patches + `mega` instances, sky sphere).
  - A first-person camera for terrains: WASD/arrow keys move, a click captures the mouse, Esc releases it.

Build and run. The binary resolves `Assets/` and `Music.mp3` from the **current directory**, so run it from `_OSX_PORT_/MacOS`:
```sh
cd _OSX_PORT_/MacOS
swift build
.build/debug/MacOS                 # logo intro; any key → scene 1
.build/debug/MacOS boats           # viewer: characters | boats | levels; any key cycles models
.build/debug/MacOS levels -test    # -test auto-quits after 5s (use for smoke tests; check NSLog output)
```

`MacOS/Assets/{Boats,Characters,Levels}` and `Assets/Logo Intro` are byte-identical copies of the data forks in `WaterRace Data/`, regrouped by type. Leah, G-Boat, Cote d'azur and Practice Zone come from `Core/`.

## Conventions to keep when porting engine math
- MetaFile data is big-endian with mac68k 2-byte alignment. Matrices are WR row-vector 4×3 (`v' = v × M`, row 3 = translation). Convert to Metal by transposing; see the comments near `mf_xform_pt`/`mf_mat_concat`.
- Animation time is in centiseconds (`kTimeUnit = 100`). All-zero quaternions mean identity. Skeleton keyframes store Euler angles (X→Y→Z) in the quaternion fields.
- When behavior is unclear, look up the original implementation in `Source/Infinity Engine/` (e.g. `Cinematic Utils.cp`, `Quaternion.cp`, `MetaFile Read.cp`) and match it.

# Asset extraction tools (`_OSX_PORT_/*.py, *.sh`)

These are pure-Python (stdlib) offline exporters. Their outputs go to `_OSX_PORT_/` (`textures/` and `models/` are gitignored; `assets/*.glb`, `Interface/`, `Sounds/` are committed):
- `extract_interface_sounds.py` — reads the `.rsrc_fork` files and writes `Interface/PICT_<id>.png|jpg` and `Sounds/*.wav`.
- `extract_textures.py` + `decode_pict.py` (via `extract_all_textures.sh`) — writes MetaFile textures to `textures/`.
- `metafile_to_gltf.py` (via `extract_all_assets.sh`) — writes `.glb` files to `assets/`. It imports its parsers from `metafile_to_obj.py`, so don't delete that file. Despite its docstring, it writes the `.glb` to the **current directory**.
- `metafile_to_obj.py` (via `extract_all_models.sh`) — writes OBJ meshes to `models/`.
- The `*_all_*.sh` scripts expect `_OSX_PORT_/.venv/bin/python3`, which does not exist in the repo. Create it, or run with the system `python3`.

The docstrings in `extract_textures.py` hold the most concise description of the MetaFile header/tag-table layout.
