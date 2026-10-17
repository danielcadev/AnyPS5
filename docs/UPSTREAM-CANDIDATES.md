# Upstream contribution candidates

## Small device-buffer retention: submitted current-main contribution

Branch: `fix/small-device-buffer-retention`, based on `ef674f5af3e4d0fe44fda10e46d36ead2c262c0e`.

The isolated correction routes device allocations smaller than 1 MiB through the existing small tier, preserving shared-pool and disabled-staging behavior. Large scratch-buffer eviction no longer discards the retained small allocation. The production change, regression and two configuration controls touch three files.

The original condition fails the new retention regression; the corrected branch passes five focused checks, including the existing real-GPU pooled-buffer-range test on AMD RX 9060 XT / locally modified RADV 26.2.3 / Fedora 44. Conventions and 27 test-registration checks also pass. Full-suite and Windows validation were not run. No native-title performance benefit is claimed.

[Submitted PR text](PR-DRAFT.md), [production delta](../patches/small-device-buffer-retention.patch), and [test results](../evidence/small-device-tests.log).

Related open [#4472](https://github.com/boykopovar/AnyPS5/pull/4472) changes pressure-based retention, not this size-tier selection. Legacy scratch reuse is already covered by merged #1004 / #2233 and was excluded to preserve current exact Vulkan buffer-size semantics. No dependency is declared.

## Vertex EXEC: overlap with an existing upstream contribution

Material: `patches/vertex-exec-copy-local.patch`, `patches/VertexExecCopy.cpp`, and the before/after GPU logs. These establish behavior on the older local stack.

[PR #1264](https://github.com/boykopovar/AnyPS5/pull/1264), authored by Celegans12 and merged October 8 as `915e3b753fe1975dd977e2a0108aa10a98afe1b9`, already addresses EXEC copies retaining lanes absent from the host subgroup. Current upstream `5f61879a97dd5268f1b183857c9144d6b94b23cf` routes EXEC reads through `hostExecWord`, which intersects the raw word with a participating-lane ballot. Its generic `sMovB64` body therefore does not imply the correction is missing. The upstream regression is `ExecHostLanes.cpp`; [#1565](https://github.com/boykopovar/AnyPS5/pull/1565) subsequently added a vertex-subgroup capability guard.

The local vertex-specific delta is not presented as a new upstream fix. A fresh current-main failure or a demonstrated missing regression would be required before proposing a separate PR. The AMD local test has not been rerun against that fresh base. The local patch includes comments that also need adapting to upstream conventions if reused. Status: **historical local evidence, upstream overlap identified; no new production PR justified yet**.

## Descriptors: an existing upstream contribution

Material: `patches/descriptor-metadata-adaptation.diff` and `evidence/descriptor-*-bench.log`.

This is a local adaptation of [PR #1334](https://github.com/boykopovar/AnyPS5/pull/1334), not a new optimization to resubmit under another author's name. The AMD/Linux measurement can be shared with attribution and its stated scope. Microbenchmark improvement must not be described as a general FPS improvement.

## Rejected candidates

The shared formatted-BDA span lookup passed GPU correctness tests, but enlarged the hot shader and did not demonstrate a convincing performance benefit. The prior source was restored. PR #6921's page-accessibility cache does not apply to the local Linux implementation: its arena/cache population path is Windows-only here. Neither is represented as an adopted optimization.

## Submission format and Git state

A discussion update belongs as a new comment at the end of the existing Linux thread, with status, tested stack, changes, measurements, screenshots, remaining blockers and AI assistance. Code proposals belong in separate, focused PRs using upstream's What / Tested / Checklist template.

The production change is committed in a separate clean worktree and submitted as [PR #7056](https://github.com/boykopovar/AnyPS5/pull/7056), head `506c0159b886c0e69b9fe428353a73a4a4ba3ac8`. The personal fork and production branch have been published. The original modified research checkout is preserved. The report and images are hosted on the separate `research/yotei-linux-evidence` branch of the personal fork for the Linux discussion; they are excluded from the production PR, following upstream contribution policy.
