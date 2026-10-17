# Engineering notes and handoff

## 1. The vertex EXEC regression

A scalar copy of the 64-bit EXEC mask could retain bits for invocations that do not participate in a vertex shader's host subgroup. In the minimal three-vertex test, the entry ballot is `0x7`, but the old path copies `0xffffffffffffffff` after EXEC is set to `-1`. Such a mask can prevent a waterfall loop from exhausting its remaining lanes.

The local `TranslationContext::sMovB64` delta uses `ballotMask(ir.GetExec())` for vertex EXEC-to-SGPR copies. Ordinary SGPR copies of literal `-1` are preserved. The regression checks the entry mask, copied mask, sparse mask and unissued vertices. It fails on the old implementation and passes on the corrected path on real AMD hardware.

A subsequent native trace records the 119856-index draw in submission 4810 and presentation of frame 776. The small regression directly demonstrates the local mask behavior; that native observation alone does not establish gameplay or eliminate every possible hang. The general Yōtei discussion already describes EXEC/waterfall failures on other hosts; this work does not claim the first discovery of that class of problem.

The corresponding problem is already addressed upstream by [PR #1264](https://github.com/boykopovar/AnyPS5/pull/1264). Current upstream masks EXEC reads in `hostExecWord`; the local regression above has not been exercised against a fresh current-main build. This evidence does not justify resubmitting the local production workaround as a new fix.

## 2. Descriptor cost versus frame rate

The PR #1334 adaptation retains CPU descriptor metadata and writes the descriptors needed by the modified bindings, avoiding a full descriptor-set copy followed by overwrites. The microbenchmark uses three rounds of 256 draws after warm-up, with one or eight snapshots; checksums match between implementations.

The medians are 28.311→3.953 µs and 30.635→4.468 µs per draw. These are not milliseconds or shader compilation times.

In the title, frames 800–819 measured 1174.310 ms versus 1168.119 ms median flip intervals, approximately 0.852 versus 0.856 FPS. That difference does not support an FPS improvement. The demonstrated benefit is limited to descriptor preparation.

## 3. Black output, partial scenes and screenshots

Several changes removed draw rejections without recovering the final image. Intermediate GPU resources also contained partial geometry. These observations must be distinguished from a correctly presented scene.

The included wolf image confirms a specific calibration screen. Archived captures now confirm the tree scene at frame 858 (instrumented run) and frame 860 (clean capture configuration), with visible rendering defects. Wolf calibration text, slider and Cross prompt are confirmed at frames 840 and 860 in a separate run. The new frame-by-frame documentation run captured frames 640–785 and then ended in OOM under the same 12 GiB cap; it did not reach 865.

Remaining black output has not been proven to be an intentional Sucker Punch cinematic fade. Reference images from another contributor or emulator are not evidence of this local stack's output. Shader/data substitutions used for diagnosis must be labeled and excluded from demonstrations of normal rendering.

RenderDoc 1.45 could not be used with the tested Wayland/`VK_EXT_external_memory_host` path. The investigation used targeted GPU readback, resource inspection and small regressions instead.

## 4. Observed costs and the memory ceiling

A profiled six-presentation wolf window showed about 1182 MiB of staging input and 1181 MiB of staging output per presentation, costing 50.49 and 61.41 ms of GPU time respectively. Aggregated GPU program time was about 304 ms per presentation. Timing ranges overlap; these categories must not be added as independent work.

Disabling the optional written-buffer shadows and optional atomic staging improved frames 785–804 from a 1274.639 to a 1034.823 ms median flip interval. Frame 800 is pixel-identical. The experiment ended in OOM at frame 804 and did not finish the later 800–819 window. It remains a promising experiment, not a demonstrated new default.

A separate footprint capture reported 9069 MiB of unique logical backing, 512 MiB of logical overlap and 178 MiB of chunk padding. Padding alone cannot account for a multi-GiB saving. DMA-buf imports can pin backing pages; a smaller chunk size does not imply that all imported memory is released.

## 5. Useful next work

1. Recover the actual local stack and compare each relevant delta with current upstream. Do not combine incompatible driver, libc and libkernel registry contracts.
2. Reduce footprint while retaining validation, synchronization and CPU/GPU coherence. The lower-staging path deserves a complete comparison with the same scene, frame window and memory conditions.
3. Isolate the vertex regression on a current base and search for equivalent implementations or open PRs before proposing it.
4. Verify the final scene and input response separately. Measure sustained gameplay only after reaching it; logos and calibration do not satisfy that requirement.

## Reduced guest-memory attempt

Advertising 11264 MiB of guest direct memory caused a failure before the first frame. A GDB capture on October 17 found a null read inside the game's allocator initialization (`.text+0x109fc8a`); the bounded run peaked at about 1.11 GiB with no swap. This is distinct from the late wolf OOM. The root cause of the allocator state remains unresolved. The default guest budget remains 13824 MiB.

## Validation limits

The included logs establish focused regressions and microbenchmark results, not a green full suite. Full Recorder testing has an R32 depth-bridge failure reproduced with both the older and adapted implementation. The full mock Graphics suite stops earlier in a heap-registration test. Neither suite is represented as entirely passing. Validation against a fresh upstream base has not been performed for this documentary handoff.
