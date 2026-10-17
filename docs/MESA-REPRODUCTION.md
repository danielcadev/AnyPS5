# Locally modified Mesa 26.2.3 used by the Yōtei research stack

The original evidence export omitted the Mesa source delta. This supplement provides
that delta against the official `mesa-26.2.3.tar.xz` release archive. It is an
experimental reproduction package, not an upstream-ready Mesa fix or a claim that
Mesa alone explains the difference between machines.

[Download the exact source patch](../patches/mesa-26.2.3-aco-local.patch).

## Changes

The complete tracked-file comparison against the original archive found five
modified files:

- `src/amd/compiler/aco_ir.cpp` and `aco_ir.h`: add the `noldsspill` ACO debug option.
- `src/amd/compiler/aco_spill.cpp`: use `IDSet(memory)` instead of
  `std::unordered_set<uint32_t>` for spill interference sets; add
  `ACO_NO_LDS_SPILL=1` to disable LDS spilling. The debug option also disables it.
- `src/compiler/nir/nir_opt_loop_unroll.c`: skip optional loop unrolling when
  `impl->ssa_alloc > (1u << 20)`, retaining the paths requiring indirect or sampler
  indirect unrolling. This is an experimental heuristic, not a proven general fix.
- `src/compiler/nir/tests/loop_unroll_tests.cpp`: four boundary/forced-unroll
  regression cases accompanying the NIR change.

In particular, setting `ACO_NO_LDS_SPILL=1` on an unmodified driver is not a
substitute for the source change that implements that variable here. The NIR
change is active without an environment switch.

## Build separately from the system driver

Use Mesa 26.2.3, rather than assuming this patch applies unchanged to 26.2.4.
The host needs Mesa's usual build dependencies (Meson, Ninja, compiler, Python
modules, libdrm, X11/Wayland development packages, etc.). Resolve these using your
distribution. Run the following in a separate work directory; `PATCH` must name
the downloaded patch using an absolute path.

```sh
PATCH=/absolute/path/to/mesa-26.2.3-aco-local.patch
curl -fLO https://archive.mesa3d.org/mesa-26.2.3.tar.xz
tar -xf mesa-26.2.3.tar.xz
cd mesa-26.2.3
patch --dry-run -p1 < "$PATCH"
patch -p1 < "$PATCH"
meson setup ../mesa-yotei-build \
  --buildtype=release \
  -Dvulkan-drivers=amd -Dgallium-drivers= \
  -Dplatforms=x11,wayland -Dglx=disabled -Degl=disabled \
  -Dgbm=disabled -Dgles1=disabled -Dgles2=disabled \
  -Dopengl=false -Dllvm=disabled -Damd-use-llvm=false \
  -Dbuild-tests=false -Dtools=
ninja -C ../mesa-yotei-build -j2
```

These are the original build's explicit project options. No system install is
needed. Select the generated development ICD only for the test process:

```sh
export VK_DRIVER_FILES="$(realpath ../mesa-yotei-build/src/amd/vulkan/radeon_devenv_icd.x86_64.json)"
export ACO_NO_LDS_SPILL=1
export MALLOC_ARENA_MAX=2
vulkaninfo --summary
# Launch your existing capped AnyPS5 command from this environment.
```

Check that the ICD file exists and that `vulkaninfo` reports the expected Mesa
26.2.3 RADV driver before testing. Ensure your launch wrapper preserves
`VK_DRIVER_FILES` rather than replacing it with the system ICD. Keep the existing
RAM/swap caps and leave memory for the desktop. No compiled Fedora driver binary
is supplied: building locally avoids assuming binary compatibility with CachyOS.

## Evidence and limits

The saved October 17 runtime receipt explicitly selects the development ICD from
this local build and sets `ACO_NO_LDS_SPILL=1`. The local RADV library and NIR
object were rebuilt after the NIR source change and before those runs.

For this supplement, the patch was applied to fresh copies of all five original
archive files and the resulting files were compared byte-for-byte with the local
sources. No new full build, test-suite run, or game run was performed for this
export. The four test cases are included as source; their inclusion is not a new
test-pass claim.

Our evidence uses PPSA26344 **01.512.000** and a warm local Mesa cache. The peer
report uses **01.008.000** and stock Mesa 26.2.4. Those differences remain relevant.
Do not infer that this patch alone fixes OOM, rendering corruption, or gameplay.
No game data, extracted shaders, or caches are included.

Mesa retains its upstream licenses; see the original [Mesa license documentation](../LICENSE-Mesa.rst) and the license notices in each source file. AI-assisted export and write-up.
