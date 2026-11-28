## Testing and Debugging

### Build Commands

Use the [existing environment](environment.md#build-in-an-existing-environment).
Run the selected `pio run -e <environment>` target; `pio run` selects `default`.
Host suites use `pio run -t unit-tests` or their existing CMake/CTest commands.

### Compact build output

The main agent records full build output in a task-specific log, preserving the
process exit status. Report target, outcome, and log path; read bounded error
context on failure. Full compiler output stays in the log unless needed to
diagnose an error.

For an active build, use waits of 10-30 seconds rather than repeated one-second
polls. Inspect a short progress/error excerpt when needed and the final result
on completion. Keep user updates within 60 seconds. A sandbox/cache-lock failure
is an environment issue, not a completed firmware build.

### Code Quality

Use `./bin/clang-format-fix` on every host (`-g` covers only unstaged edits), including formatter diagnostics;
never invoke or probe raw `clang-format`. Run `pio check -e x4c` and
`bin/clang-tidy-check` on every C/C++ change. The full required list is in
[Verification is not optional](../../AGENTS.md#verification-is-not-optional);
`.github/workflows/ci.yml` is the list of record for linters and test jobs.
New warnings and findings are fixed, not left for later.

### Testing and Verification Workflow

[firmware-handoff](../skills/firmware-handoff/SKILL.md) owns the firmware review
pass, final checks, and the PR test plan. The main agent owns tests, builds,
and diagnostic code. Reviewers inspect source and recorded results only.

Add or update host tests for every behavior change the host build can reach.
While iterating, rerun the smallest meaningful checks for the changed contract;
before marking a firmware or build PR ready, run the full verification list;
documentation and host-only changes run the checks they can affect. Reuse green results until
a change or failure invalidates them. Test-only, diagnostic,
documentation, and host-only changes use relevant lighter checks unless they
alter firmware output/build. Check ignored outputs before staging, using the
[generated-file rules](generated-files-cache.md#files-generated-by-build-scripts).

Device checks in the PR test plan cover changed behavior, relevant orientations/remapping,
and resource/cache effects. Changed input, display, or layout paths cover all
four orientations: portrait, inverted, landscape CW, and landscape CCW.
Measure on `x4c`, and on each other release board that still compiles the
change (including the C3 when it compiles the change), or give a labeled estimate for boards you cannot
measure. An `x4c` reading alone does not show C3 or OnePage fit. Target: more than 50 KB free internal heap and no leaks;
measure internal/PSRAM heaps and largest blocks separately across relevant repeated
book, dictionary, font, or WiFi transitions. Free bytes alone do not prove an
allocation will fit. For layout changes, back up progress and verify affected
cache regeneration, including TXT/Markdown and partial-build resume when relevant.

### Debugging Crashes

Use serial traces to establish the failure path before changing resources.

- Heap/OOM: measure `HalMemory` internal/PSRAM statistics and largest blocks
  before/after the operation; account for peak coexistence, allocation failure,
  and activity cleanup. See [resource rules](hardware-resources.md#the-resource-protocol).
- Stack: inspect call chains and `uxTaskGetStackHighWaterMark()`; adjust stack
  or buffer placement from evidence, not a copied task size.
- Task lifetime: follow [activity cleanup](ui-activities.md#activity-lifecycle-and-memory-management)
  and shutdown before destruction; inspect shared-state synchronization.
- Cache corruption: reset only affected data after backup and check the
  [format contract](generated-files-cache.md#cache-file-format-versioning).
- Watchdog: inspect blocking I/O and loops against the configured timeout;
  preserve task scheduling/yield behavior when changing the work.

### CI/CD Pipeline Awareness

Inspect current matrices under `.github/workflows/` before claiming board
coverage. Preserve pinned core/packages and patches. Release/RC board-specific
asset naming must agree with OTA discovery. Fix CI failures on your PR before
calling it ready.

CI builds only `x4c` (X4 Classic) by default. To build every board, add the
`all-boards` label to the PR; it stays in effect for later pushes. Do this for
changes to shared board code, the HAL, the SDK pin, or `platformio.ini`; for
code that x4c does not compile (other `FREEINK_DEVICE_*` blocks, or
capabilities x4c lacks such as `FREEINK_CAP_TOUCH`); for features gated out of
some boards or shared code that grows static RAM; and for work meant for
upstream. The required Test Status check shows the latest run to finish, so
before merging, wait for the labeled run's Test Status and check that the PR's
firmware comment lists every board. cppcheck covers only `x4c` either way.
Releases still build every board.

### Serial Monitor Options

Use `python3 scripts/debugging_monitor.py` for enhanced logs or
`pio device monitor` / the IDE monitor. Device access and measurements remain
within the authorized task.

### Live Debugging Patterns

Use `LOG_DBG` for measurements, `LOG_LEVEL=2` for debug logging, and
`logSerial.flush()` when a crash may lose buffered output. Task stack diagnostics
use `uxTaskGetStackHighWaterMark()`; heap diagnostics follow `HalMemory`, not
a combined free-byte reading. The enhanced monitor plots PSRAM separately when
those log values are supplied.
