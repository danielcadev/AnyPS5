# Local integration source snapshot

This combined source delta preserves the existing local research stack as of October 17, 2026: PR adaptations, local fixes, diagnostics and test sources. See [PR attribution and local status](PR-INTEGRATION.md).

This is review material, not a proposed single upstream PR. It uses an older base and includes diagnostics and experiments without proven native benefit. The combined export has not been rebuilt from a fresh checkout; existing focused test results are recorded separately. One private diagnostic output path was replaced in the exported patch with `/tmp/anyps5-shader_%llx_%zu.spv`. The working source was not changed.

## Bases and application

- Public AnyPS5 base: `832247b0a09c140c436d26a7f8333ff34f3df016`. The delta includes local commits through `d4fbcb3933a96704f00379f2507d9d72397c8881` and subsequent working-tree changes.
- SPIRV-Tools submodule base: `7f2d9ee926f98fc77a3ed1e1e0f113b8c9c49458`.
- `patches/local-integrated-stack.patch`: tracked source changes plus all 16 currently untracked C++ source/header files under `core/`.
- `patches/spirv-tools-local.patch`: the SPIRV-Tools submodule delta, including the #5511 adaptation.

Use a separate checkout at the recorded base with its original submodules initialized. Apply the AnyPS5 patch there with `git apply /absolute/path/to/local-integrated-stack.patch`; apply the SPIRV-Tools patch from inside `3rdparty/SPIRV-Tools`. Both patches passed `git apply --cached --check` against temporary indexes of their recorded bases. This verifies applicability, not compilation or gameplay. The working source tree and real Git indexes were untouched.

The original export omitted Mesa; the [Mesa supplement](MESA-REPRODUCTION.md) now supplies its source patch and build instructions, but no precompiled driver. Desktop setup, game content, shader cache and launch automation are not provided. The runtime still requires a compatible Mesa/RADV stack and the normal AnyPS5 build/relink workflow. Do not apply these patches on current `main` or combine them with the individual excerpt patches: those excerpts are already included. A current-main contribution requires isolating one topic, checking existing upstream implementations and validating it independently.

AnyPS5-derived changes retain [GPL v2](../LICENSE-AnyPS5); SPIRV-Tools-derived changes retain its [Apache 2.0 license](../LICENSE-SPIRV-Tools). Original authors are credited in the PR inventory. The snapshot does not establish upstream acceptance or playable gameplay.
