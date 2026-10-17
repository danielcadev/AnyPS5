# Evidence and claim provenance

The available project chats “AnyPS5: recuperar 3D visible y gameplay”, “AnyPS5: continuar hacia 3D en Yōtei”, “Optimizar AnyPS5 Antigravity para avanzar…” and the current history were used as an index. Claims were checked against local source, logs and images. “Revisión del progreso” returned content references without its assistant response text and was not used as proof. Personal conversations were not exported.

## Included material

| File | Source in the original workspace | What it establishes |
|---|---|---|
| `assets/wolf-frame800.png` | `recovery/anyps5-migration/runs/anyps5-yotei-20261017-070914-76a8a3/frame_000.png` | Visible wolf; capture configured from frame 800 |
| `evidence/vertex-exec-before.log`, `vertex-exec-after.log` | `recovery/anyps5-migration/two-data-fixes/vertex-exec-copy-{before,after}.log` | GPU regression failure before and success after |
| `evidence/vertex-native-excerpt.log` | Run `anyps5-yotei-20261015-082541-3c194f`, `console.log` | 119856-index draw in submission 4810 and presentation 776 |
| `evidence/descriptor-*-bench.log` | `recovery/anyps5-migration/pr1334-local/{baseline,candidate}-bench.log` | Three timing rounds and matching checksums |
| `evidence/frame-intervals.csv` | Run named in each row | All 20 samples for each implementation/window; medians can be recalculated |
| `evidence/wolf-profile-excerpt.log` | Run `anyps5-yotei-20261017-071350-269163` | Six-presentation GPU window, overlapping ranges and CPU counters |
| `patches/vertex-exec-copy-local.patch` | Local AnyPS5 diff | Narrow production delta, not an upstream-validated branch |
| `patches/VertexExecCopy.cpp` | Local driver regression | Test source dependent on the project's harness |
| `patches/descriptor-metadata-adaptation.diff` | Local adaptation of PR #1334 | Review material with upstream attribution |

Small logs replace the private workspace prefix with `$WORKSPACE`. Excerpts are selected by run, frame or event identifiers without rewriting their measurements. The CSV contains every sample in each published comparison window. The new documentation run saved frames 640–785 inclusive (146 PNGs), then ended in OOM at a 12 GiB peak and 496586752 bytes of peak swap. `evidence/capture-sequence.csv` maps each frame by the actual dump and FrameTiming log records. `evidence/screenshots.csv` identifies the older tree and Cross captures. All screenshots are copied unchanged from their archived PNGs.

## Current-main production contribution

`patches/small-device-buffer-retention.patch` is isolated from the historical stack and based on `ef674f5af3e4d0fe44fda10e46d36ead2c262c0e`. The focused checks and native run used the preceding `main` `f05fc7d953d30160cb05411875b2824fd10acc15` plus the same correction; the final rebase only adds upstream #7055, a LoadExec test cleanup with no production changes. `evidence/small-device-tests.log` records all five focused tests passing after rebasing and rebuilding the affected targets. `small-device-negative.log` records the failing original-condition control on the preceding base; `small-device-gpu.log` identifies the real RADV device used by the existing buffer-range compatibility check. The newly merged base change affects packed-f16 shader translation, not the pool implementation. See [the submitted PR text](PR-DRAFT.md) for scope and limitations.

## Combined source export

`patches/local-integrated-stack.patch` contains 174 changed/added source files relative to the recorded AnyPS5 base; `patches/spirv-tools-local.patch` contains seven submodule source changes. Their bases, application procedure and single normalized diagnostic output path are documented in [LOCAL-INTEGRATION.md](LOCAL-INTEGRATION.md). Both passed applicability checks using temporary Git indexes; the real workspace/indexes were untouched. This structural check does not replace a fresh full build. [PR-INTEGRATION.md](PR-INTEGRATION.md) records original authors and states read from GitHub on October 17.

## Claims excluded or qualified

- “Descriptor tables went from 9 seconds to 4 milliseconds”: sufficient evidence for that number was not located. The measured result here is **microseconds per draw** for descriptor preparation, not SPIR-V compilation.
- Full gameplay, 20–30 FPS in the scene, or 30–60 FPS in other games: not established by these tests.
- Remaining black output is an intentional cinematic curtain: not established.
- All fixes are original, or patches are already merged under this contributor's name: not claimed. The stack includes adaptations of others' work and uncommitted local changes.
- A partially rendered tree scene, an intermediate GPU image and a correctly presented playable world are different milestones. The screenshot labels must preserve those distinctions.

## Upstream references consulted

- [Linux discussion #2884](https://github.com/boykopovar/AnyPS5/discussions/2884): destination for this report and its chronological update.
- [General discussion #675](https://github.com/boykopovar/AnyPS5/discussions/675): context from other contributors and platforms.
- [PR #1334](https://github.com/boykopovar/AnyPS5/pull/1334): descriptor metadata adaptation; consulted head `1dbf97622636cc8a38e855da4b5fc84fb277fd05`.
- [PR #1264](https://github.com/boykopovar/AnyPS5/pull/1264): merged EXEC host-lane correction by Celegans12. Current upstream `5f61879a97dd5268f1b183857c9144d6b94b23cf` was inspected through `sMovB64`, `readRawU32` and `hostExecWord`; the latter already intersects EXEC with a participating-lane ballot. Looking only at the generic `sMovB64` body would miss this existing implementation.
- [CONTRIBUTING.md](https://github.com/boykopovar/AnyPS5/blob/main/CONTRIBUTING.md): general fixes, semantic sources, tests, AI attribution and keeping investigation material outside the code tree.

## Recovered visual milestones

- `tree-rendering-defects-frame860.png`: run `anyps5-yotei-20261016-220144-0b6a0a`, dump 003, capture-after 800 / every 20. The receipt contains no late shader/data substitution probes. Visible tree content has severe rendering defects.
- `tree-partial-frame858.png`: run `anyps5-yotei-20261016-212211-038bbc`, dump 008, capture-after 730 / every 16. This run includes late draw readback/trace instrumentation; it is labeled accordingly.
- `wolf-x-frame840.png` / `wolf-x-frame860.png`: run `anyps5-yotei-20261017-061353-cc68c7`, dumps 002 / 003. Text, slider and Cross glyph are visually present.
- `assets/sequence-075645/`: run `anyps5-yotei-20261017-075645-8611dd`, dump every frame from 640. Automatic Cross remained enabled. The runtime libraries and pipeline cache were restored by the wrapper after OOM. No gameplay claim is made from this sequence.

## Current-main native smoke test — October 17

The clean production branch `d8254f8f3ea157c40c2f596893805d5d629a4e0e` (`main` `f05fc7d953d30160cb05411875b2824fd10acc15` plus the small-device retention change) built all system libraries and the relinker successfully. The title and bundled guest libc were converted afresh; the import audit found no unresolved imports. A separate runtime used those libraries with the same locally modified Mesa/RADV build, `MALLOC_ARENA_MAX=2`, `ACO_NO_LDS_SPILL=1`, a 4 GiB desktop reserve, and 12 GiB RAM / 2 GiB swap caps. No historical research PRX was substituted.

The native process ended with cgroup `oom-kill` / status 9 after approximately 122 seconds, peaking at 12884901888 bytes RAM and 1313841152 bytes swap (about 1.22 GiB). Capture was requested every ten presentations; no BMP files were produced. The wolf/tree and exact presentation count are unverified, and the timing log was disabled. Shader-cache counters recorded 1658 misses/writes, with no load/write failures in the final reported totals. The new AnyPS5 cache was cold; the existing Mesa driver cache was reused. This is not a paired comparison with the older warm integrated stack and does not isolate a regression or attribute the OOM to the pool correction.

[Sanitized outcome and shader-cache counters](../evidence/current-main-native.log). No shader request payloads or game data are included in that excerpt. The original runtime and research source were preserved, and the test service has stopped.
