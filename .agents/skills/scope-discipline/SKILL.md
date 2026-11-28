---
name: scope-discipline
description: Fork feature scope and resource gate. Use when adding features, apps, activities, libraries, settings, dependencies, or expanding the firmware surface.
---

# Scope Discipline

This fork is a reading and study device on constrained hardware. `SCOPE.md` is
the source of truth for what is in and out. Read it before adding surface.
Interactive apps such as flashcards and a Bible reader are in scope; the
hardware limits are not negotiable.

## The gate

Before adding a feature, app, activity, lib, setting, or dependency, answer in
order:

1. **What does `SCOPE.md` say?** In scope: proceed. "Ask first" (browsers,
   RSS, media playback, PDF, background Wi-Fi or tasks): state the cost and ask
   the owner before building it. Out: say so and stop.
2. **What does the owner use it for?** State the concrete use on the device.
   "Might be useful" is not a requirement.
3. **What resources does it need?** Account for steady-state and peak RAM,
   the largest required heap block, Flash/OTA slot headroom, and SD storage.
   Budget against `x4c`, and leave a feature out of boards it does not fit
   (see the [resource target](../../rules/hardware-resources.md#resource-target)).
   Check internal RAM and PSRAM separately for the selected board. Label estimates and measurements, and
   leave missing measurements pending. Use `scripts/firmware_size_history.py`
   and `scripts/script_profile_mem.sh` for build footprint comparisons.
   Runtime heap and power claims require device measurements.
4. **Can an existing mechanism meet the requirement?** Follow the reuse ladder
   below before adding a code path.
5. **What stays active, and for how long?** Describe network and task
   lifetimes, retry limits, idle sleep, and cleanup on failure and exit.
   Follow the [activity and power rules](../../rules/ui-activities.md#freertos-task-guidelines).
   Keep network ownership within the defined flow, including its child screens.
   Release it when that flow ends.
6. **What must the owner maintain?** Explain the cost of new dependencies,
   settings, and failure paths alongside the concrete benefit.

If a request fails the gate, push back with the specific reason and the
`SCOPE.md` basis, and offer the alternative that fits. Make the call and say
why; do not just hand over a menu.

## New apps

- Put the app's logic in its own `lib/` directory with host tests; keep the
  activity a thin UI layer.
- Keep app data and state on the SD card in a documented, versioned format.
  Migrate user state (review history, progress) on format changes; never
  discard it.
- Touch shared upstream code only where needed (menu entry, hook), so upstream
  merges stay easy.
- Release all app memory and tasks when the app exits.

## Surface awareness

Before writing new code, trace the affected flow and stop at the first option
that fully meets the requirement:

1. An existing setting, activity, configuration, or documentation change.
2. An existing helper, type, or interface in this codebase.
3. An appropriate standard-library facility within the resource budget.
4. An existing SDK capability through the HAL, or an installed dependency.
5. The minimum new code with a clear owner and a demonstrated need.

Leave speculative hooks and future scaffolding for a concrete requirement.

Each new activity adds code and lifecycle responsibilities. Its resident RAM
depends on its implementation and lifetime. Include retained parents in the
peak budget when activities are pushed onto the stack.
Default to extending an existing activity or setting before adding a new screen.
Explain why the new screen is needed.

## Settings are not free

A new setting is a field to persist, migrate, validate, translate, and render,
plus combinatorial test burden. Add one only when users genuinely need the
choice; otherwise pick a sensible fixed default.

## Self-review

- [ ] Checked against `SCOPE.md`; asked the owner first if it is on the ask-first list.
- [ ] Stated the concrete use, not a generic "useful."
- [ ] Accounted for RAM, largest-block, Flash/OTA, storage, power, and maintenance costs and explained why the benefit is worth them.
- [ ] Labeled estimates, measurements, and pending device checks.
- [ ] Defined network/task lifetimes, retry limits, sleep behavior, and failure/exit cleanup.
- [ ] Checked whether an existing activity/setting/doc already covers it.
- [ ] App logic is in `lib/` with host tests.
- [ ] New setting (if any) is justified by a real user need, not added
      "just in case."
