# CrossPoint Reader agent guide

CrossPoint Reader is open-source e-reader firmware for Xteink and other FreeInk-supported devices. Its mission is a lightweight, high-performance reading experience focused on EPUB rendering. This fork targets the Xteink X4 Classic (`x4c`, ESP32-S3 with PSRAM), and its features may rely on that board's extra memory. The ESP32-C3 X3/X4 (`default`) has roughly 380 KB usable RAM and no PSRAM; it must still build and run the reader core, but an optional feature that needs more memory may be compiled out of it. Every board has one framebuffer sized for its panel. See [hardware-resources.md](.agents/rules/hardware-resources.md#resource-target). Other board profiles have different CPUs, display sizes, input, and memory capabilities; check the selected profile before assuming them.

## FreeInk SDK

The [FreeInk SDK](freeink-sdk/) supplies the board profiles, hardware drivers,
and shared UI components. Before adding an API or device-specific code, check
the firmware HAL and the SDK source at the revision pinned by this repository.
Use the [SDK documentation](freeink-sdk/docs/README.md) and
[FreeInk SDK index](https://freeink.org/llms.txt) to find the relevant contract.
Check `git submodule status`: a different local SDK revision must be accounted
for, not silently treated as the pinned firmware dependency.

## Start here

At session start, run `uname -s`, `git branch --show-current`, `git remote -v`, and `git status --short`. Integration work targets `develop`.

Act as a senior embedded C++ engineer. Base claims on repository evidence: cite the paths and line numbers that justify a proposed change. Explain the mechanism behind performance or memory claims and justify every new heap allocation. For every fix, state how it was verified and how the owner can check it on the device.

First inspect the relevant code and current behavior to establish whether a change is needed. Before adding an API, setting, task, persistent state, driver change, or scheduling dependency, tie it to the requirement and check existing mechanisms. Identify unexplained scope inherited from a stash or branch.

Use the table to find guidance for the decisions being changed. Read relevant sections, not every file associated with a touched subsystem. Load implementation skills when implementation is needed and `firmware-handoff` at final handoff. Reuse guidance already in context unless it changed or a concrete question requires another read.

| Decision being changed | Relevant guidance |
| --- | --- |
| Routine builds or host setup | [existing environment](.agents/rules/environment.md#build-in-an-existing-environment) |
| Personal build flags or ports | [local overrides](.agents/rules/environment.md#platformiolocalini-personal-overrides) |
| Allocation size/failure, buffer or cache lifetime, hot-path containers, or hardware limits | [hardware-resources.md](.agents/rules/hardware-resources.md); use `heap-discipline` for allocation decisions |
| Optional features in shared code, new struct fields or template code, device-specific code, or SDK bumps | [flash size](.agents/rules/hardware-resources.md#flash-size-unused-features-must-cost-nothing) |
| Hardware/storage access, rendering contracts, input ownership, or SDK boundaries | [HAL contracts](.agents/rules/architecture-hal.md#hardware-abstraction-layer-hal) and relevant sections of `hal-and-abstractions` |
| Build flags, dependencies, or board profiles | [build environment and flags](.agents/rules/architecture-hal.md#build-environment) |
| C or C++ implementation | [coding conventions](.agents/rules/coding-standards.md#naming-conventions); consult the relevant pitfall/allocation section and use `control-flow-clarity` for state modeling |
| Lists, controls, touch routing, or layout | [shared UI](.agents/rules/ui-activities.md#shared-ui-and-input) and [orientation](.agents/rules/ui-activities.md#orientation-aware-logic) |
| Input edges or activity/popup transitions | [input ownership](.agents/rules/ui-activities.md#input-frames-and-ownership) and its transition/verification sections |
| Activity resources or worker tasks | [activity lifecycle](.agents/rules/ui-activities.md#activity-lifecycle-and-memory-management) and [workers](.agents/rules/ui-activities.md#freertos-task-guidelines) |
| Runtime font discovery, rendering, or cache lifetime | [font ownership](.agents/rules/ui-activities.md#runtime-fonts-and-cache-ownership) |
| Plugin, service, web endpoint, or protected-book contracts | [service contracts](.agents/rules/architecture-hal.md#service-and-plugin-contracts) and the linked contract that applies |
| Builds, formatting, or verification | [testing workflow](.agents/rules/testing-debugging.md#testing-and-verification-workflow) and [compact build output](.agents/rules/testing-debugging.md#compact-build-output) |
| Crashes, CI, or serial measurements | [crash diagnosis](.agents/rules/testing-debugging.md#debugging-crashes) or the matching CI/serial section |
| Branch creation, merges, commits, or publication | [git-workflow.md](.agents/rules/git-workflow.md) |
| User-facing strings or language metadata | [translation workflow](.agents/rules/generated-files-cache.md#translation-workflow) |
| Generated HTML/i18n or font assets | [generated sources](.agents/rules/generated-files-cache.md#files-generated-by-build-scripts) or [font generation](.agents/rules/generated-files-cache.md#font-generation) |
| Cache formats, layout persistence, or invalidation | [versioning](.agents/rules/generated-files-cache.md#cache-file-format-versioning) and [invalidation](.agents/rules/generated-files-cache.md#cache-invalidation-rules) |
| New features, apps, activities, settings, libraries, or dependencies | `SCOPE.md` and the `scope-discipline` skill |
| Restructuring, extraction, or separating unrelated cleanup | the `refactor-for-review` skill |

Repository-local skills live under `.agents/skills/`. Their descriptions identify specific procedures to use when needed; a keyword match alone does not require loading a skill.

## This fork

This repository is a personal fork of upstream CrossPoint Reader
(`crosspoint-reader/crosspoint-reader`). It adds features for the owner's own use, such
as Anki-style flashcards and a Bible reader, on top of the upstream reading
firmware. [`SCOPE.md`](SCOPE.md) describes what belongs here. The upstream
policies on PRs, scope, and agent autonomy do not apply; the technical rules in
`.agents/rules/` still do, because the hardware is the same.

## How agents work here

Agents may work end to end: create a branch, implement, run the checks, commit,
push the branch, and open a PR against `develop`. Write the PR description
yourself, using [the PR template](.github/PULL_REQUEST_TEMPLATE.md). Follow the
[git rules](.agents/rules/git-workflow.md). Review subagents are the exception:
they only inspect, and never write files, run tests or builds, add
diagnostics, commit, push, publish, or flash.

The owner keeps these decisions:

- Merging to `develop`, and any push to `develop` or `master`.
- Opening PRs or issues anywhere except `iBelieve/crosspoint-reader`,
  including upstream.
- Force-pushing or rewriting history on a branch you did not create.
- Tags, releases, and changes to release or OTA workflows' publishing steps.

Decide these yourself, but flag them in the PR:

- Changes to partitions, the bootloader, OTA or recovery code, the HAL, or the
  SDK pin. State the update and rollback risk.
- A design choice that changes what a feature does for the owner, when the
  request can be read more than one way. Pick the likely reading and say which.

Agents cannot test on hardware. Never claim a hardware result. Every firmware
PR lists the device checks the owner should run, with expected results.

Prefer the simplest design that meets the requirement and that one person can
maintain. Repository-facing prose: use plain English and standard technical
terms. Code comments must be short and useful after merge; follow the
[comment rules](.agents/rules/coding-standards.md#comment-style).

## Verification is not optional

The owner relies on static analysis, linting, and tests to catch what they cannot
review line by line. A change is not done until its checks pass. Run them
yourself; do not leave them for CI to find.

1. **Format:** `git add` new files, run `./bin/clang-format-fix` (all tracked
   files, as CI does; never raw `clang-format`), stage its changes, and confirm
   `git diff --exit-code` is clean. The `-g` flag only covers unstaged edits,
   so do not use it as the final check.
2. **Host tests:** `cmake -S test -B build/test && cmake --build build/test && ctest --test-dir build/test --output-on-failure -j`,
   plus `python3 -m unittest discover -s test -p test_firmware_stats.py` and
   `node --test test/firmware_comment.test.js`.
3. **Static analysis:** `pio check -e x4c --fail-on-defect low --fail-on-defect medium --fail-on-defect high`.
   For Python, `ruff check .` and `ruff format --check .` (ruff 0.16.9, as
   CI pins; `ruff.toml` lists the covered paths). For JavaScript, `npm ci && npx eslint .`.
4. **Firmware build:** `pio run -e <env>` for each board the change affects.
   CI builds boards other than `x4c` only on PRs with the `all-boards` label.
5. **Any other linter or test job** in `.github/workflows/ci.yml`. Read it at
   the start of the task; CI is the list of record, and it grows.

Rules for these checks:

- New logic gets tests. Put feature logic (parsing, scheduling, state
  machines, storage formats) in `lib/` behind an interface the host test build
  can compile, as `lib/LibraryIndex` does, and keep the activity a thin UI
  layer. A bug fix gets a test that fails before the fix when the code is
  host-testable.
- Do not add compiler warnings, cppcheck findings, or lint findings. Fix the
  cause. Suppress only with a one-line reason at the suppression.
- Never skip, disable, weaken, or delete a test or check to get green. If a
  test is wrong, fix it and say why in the commit message.
- If a check cannot run in your environment (for example `pio` cannot download
  its toolchain), say which check, why, and what you ran instead. An unrun
  check is not a pass. Then rely on CI for it and read the CI result before
  calling the PR ready.
- Fix CI failures on your PR. Do not ask the owner to merge a red PR.
- Before a PR is marked ready, and again before it is merged if its head
  changed, run a code review at `xhigh` effort on the current head. In Claude
  Code this is the built-in `code-review` skill. Other agents run an equivalent
  independent correctness review of the whole diff. Fix real findings first.

## Firmware handoff

For every change that can affect shipped firmware or its build (C/C++, build
configuration, partitions, code-generation sources, translations, release
scripts), complete [firmware-handoff](.agents/skills/firmware-handoff/SKILL.md)
before marking the PR ready (a draft PR may be open earlier). It covers the
checks above, one independent review pass, the `xhigh` code review, and the
PR's test plan.

Documentation, pure tests, and host-only scripts need the relevant checks, a
self-review against the same checklists, and the `xhigh` code review, not the
full handoff, unless they change firmware output or its build.
