### What

Small device-local buffers currently share the device tier with large scratch allocations. Releasing two buffers that together exceed that tier's byte budget evicts a previously retained 256-byte buffer, so the next small request allocates again.

Route device allocations with a capacity below 1 MiB to the existing small tier. Keep the shared-pool mode and disabled-staging mode unchanged, along with existing byte/slot budgets, memory-property keys, allocation lifetimes and exact Vulkan buffer sizes.

Add a regression through `DeviceBuffer` that forces large-buffer eviction, checks reuse of the small allocation and verifies final object cleanup. Separate processes cover shared pooling and disabled staging because the configuration is cached per process.

Related: #4472 changes VRAM-pressure retention; this PR changes size-based tier selection and has no dependency on it. The broader Linux/Yōtei investigation is tracked in discussion #2884. No title FPS or RAM improvement is claimed for this isolated change.

### Tested

Branch base: `ef674f5af3e4d0fe44fda10e46d36ead2c262c0e` (`main`). The focused tests, complete runtime build and native smoke test used `main` `f05fc7d953d30160cb05411875b2824fd10acc15` plus this correction; the subsequent rebase only adds upstream #7055, which changes the LoadExec test cleanup, with no production changes. Fedora 44 x86-64, GCC 16.2.1, Release, default `ANYPS5_ENABLE_SPIRV_TOOLS=OFF`.

- `agc_buffer_pool`, `agc_buffer_pool_small_device_shared` and `agc_buffer_pool_small_device_no_staging`: passed.
- Negative control: restore only the original `tierFor` condition, rebuild and run `APS5_STAGING_POOL_MIB=256 agc_buffer_pool_tests --small-device-retention`. It exits 1 with both retention/reallocation assertions failing. Restore the correction and rebuild for the final passing run.
- `agc_device_memory_fallback`: passed.
- `agc_driver_pooled_buffer_range`: passed on AMD Radeon RX 9060 XT 16 GB, locally modified Mesa 26.2.3 RADV, Fedora Linux. This real-GPU compatibility test checks exact buffer sizes and robust reads beyond the requested range; the new eviction regression itself uses the existing mock Vulkan allocator.
- `tools/check_conventions.py`: passed with the proposed title/body.
- `tools/tests/test_test_registration.py`: 27 tests passed.
- Built all system libraries (`libs`) and the relinker, reconverted PPSA26344 01.512.000 and checked imports: no unresolved imports. A separate cold-AnyPS5-cache native run ended in cgroup OOM after about 122 s at the 12 GiB RAM cap (1.22 GiB swap peak). No wolf/tree capture or title performance benefit was established; this is not a paired pool comparison. The original runtime was preserved.

Validation is scoped to buffer allocation and retention. The full test suite and Windows were not run.

### Checklist

- [x] Based on current `main`; no other open PR implements the same functions
- [x] One topic per PR; follow-ups go in a new PR
- [x] No comments in code except technical debt
- [x] Unimplemented paths throw; no silent stubs added
- [x] No notes, investigation `.md` files or images added to the repository
- [x] New third-party code is a submodule built from source; none added
- [x] Generic behaviour, not specific to one title
- [x] Depends on: none
- [x] Fixes: none
- [x] Unblocks: none
- [x] GPUs tested: AMD Radeon RX 9060 XT 16 GB; locally modified Mesa 26.2.3 RADV; Fedora 44 Linux x86-64 (`pooled_buffer_range` compatibility test)
- [x] AI-assisted: yes
