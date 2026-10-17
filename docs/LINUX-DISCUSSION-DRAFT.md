### October 17, 2026 — AMD/Linux follow-up (PPSA26344 01.512.000)

Following the Linux bring-up reports here, our local stack now reaches the **wolf calibration with the Cross prompt, the tree scene, and “Select an Experience / Standard.”** The tree still has severe green/magenta corruption. **Playable gameplay is unverified.**

**Tested stack:** Ryzen 7 5700X, RX 9060 XT 16 GB, 32 GiB RAM, Linux/Wayland and locally modified Mesa 26.2.3 RADV. AnyPS5 starts from public revision `832247b0a09c140c436d26a7f8333ff34f3df016`, with local commits and adaptations listed in the report. These native results are **not stock current-main results**.

**Investigation, in order:**

1. A real-GPU regression on the older stack exposed EXEC copies retaining nonparticipating vertex lanes. Before/after tests cover three-vertex and full-subgroup draws, sparse masks and ordinary SGPR `-1` values. Comparing current upstream found the equivalent correction already merged in **#1264 (Celegans12)**, so we are sharing historical evidence rather than resubmitting it.
2. Adapting **#1334** reduced descriptor preparation from **28.311 to 3.953 µs/draw** in the one-snapshot microbenchmark. Wolf FPS remained approximately **0.852 vs 0.856**; this is not an FPS improvement claim.
3. Profiling found roughly **1182 MiB staging input / 1181 MiB output per presentation** at the wolf. A reduced-copy experiment improved one paired window but ended in OOM; sustained benefit remains unproven.
4. The final verification continued until failure: **frame 952, cgroup OOM**, peaking at **12 GiB RAM + about 1.68 GiB swap**, with `MALLOC_ARENA_MAX=2`. Frames **880–951** had a median presentation interval of **981.5125 ms (~1.02 FPS)**. A 4 GiB desktop reserve remained in place.

**Same-run screenshots, in presentation order** (`anyps5-yotei-20261017-083159-9b94aa`):

**840 — wolf calibration / Cross prompt**

![Wolf calibration and Cross prompt](https://raw.githubusercontent.com/danielcadev/AnyPS5/refs/heads/research/yotei-linux-evidence/assets/wolf-x-verification-frame840.png)

**880 — tree with rendering defects**

![Tree with green and magenta corruption](https://raw.githubusercontent.com/danielcadev/AnyPS5/refs/heads/research/yotei-linux-evidence/assets/tree-verification-frame880.png)

**920 — Select an Experience / Standard**

![Select an Experience and Standard option](https://raw.githubusercontent.com/danielcadev/AnyPS5/refs/heads/research/yotei-linux-evidence/assets/experience-selection-frame920.png)

**Current-main contribution:** [PR #7056](https://github.com/boykopovar/AnyPS5/pull/7056) keeps small device buffers in the existing small pool tier so large-buffer eviction does not discard them. The regression fails with the original condition; five focused checks pass with the fix, including a real-RADV buffer-range compatibility test. This is separate from #4472's VRAM-pressure retention work. No native FPS/RAM improvement is claimed for this isolated PR.

[Full report, source snapshot, logs and capture archive](https://github.com/danielcadev/AnyPS5/tree/research/yotei-linux-evidence) includes **22 used/adapted PRs with original authors**, the integrated source against its public base, and a separate 146-frame archive (640–785). The historical combined export passed applicability checks but has not been rebuilt from a fresh checkout; it is not a single upstream-ready patch. No game binaries, keys, saves, extracted shaders or pipeline caches are included.

**Fresh-base check:** the code branch also built all system libraries and the relinker, reconverted the title and cleared the import audit. A separate native run with a cold AnyPS5 cache ended in cgroup OOM after ~122 s at the 12 GiB RAM cap (1.22 GiB swap peak); no wolf/tree capture was obtained. This does not establish a regression or an FPS comparison with the older warm integrated stack. [Outcome and counters](https://github.com/danielcadev/AnyPS5/blob/research/yotei-linux-evidence/evidence/current-main-native.log).

Remaining blockers are rendering corruption and the staging/imported-memory footprint. Sharing this evidence so others can continue. AI-assisted investigation, code and write-up.
