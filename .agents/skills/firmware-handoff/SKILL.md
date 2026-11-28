---
name: firmware-handoff
description: Final checks, one independent review, and the PR test plan for completed firmware/build changes. Use after implementation and self-review, before marking a PR ready; excludes read-only audits and reviewer tasks.
---

# Firmware handoff

Run after implementation and self-review, before marking the PR ready. A
draft PR may already be open. The main agent owns the result; reviewer output is evidence, not
authority.

## 1. Prepare the candidate

Record the requirement, comparison base (`develop`), changed files, and diff,
including uncommitted work and any SDK submodule changes. Account for every
hunk. Leave unrelated work (especially the owner's uncommitted edits) out of
the commit; never revert it. Self-review against the four review checklists below.

**Done when:** every hunk is explained and the self-review is done.

## 2. Run every check

Run the [verification list](../../../AGENTS.md#verification-is-not-optional):
format, host tests, `pio check -e x4c`, firmware builds for affected boards, and any
other job in `.github/workflows/ci.yml`. Follow the
[testing rule](../../rules/testing-debugging.md) for build logs.

- New or changed logic has host tests, and they pass.
- The change adds no compiler warnings, cppcheck findings, or lint findings.
- Record each command and its outcome. A check that could not run is reported
  with its reason; it is not a pass.

**Done when:** all checks pass, or each unavailable check is recorded with its
reason and CI is relied on for it.

## 3. One independent review

Launch one read-only reviewer subagent with a fresh context. Give it the
requirement, repository path, base, the pinned diff and revision, check
results, and these four checklists, applying only those relevant to the
change. Give it the needed discussion but not your own analysis or verdict:

- [review-correctness](../review-correctness/SKILL.md)
- [review-architecture](../review-architecture/SKILL.md)
- [review-embedded](../review-embedded/SKILL.md)
- [review-i18n-docs](../review-i18n-docs/SKILL.md)

Include this scope: "Inspect code, diffs, history, and recorded checks only. No
file writes, tests, builds, or new diagnostics; request needed checks from the
main agent." For a large or risky change (new app, memory ownership, HAL or
storage format changes), you may split the checklists across up to four
reviewers.

Keep the candidate unchanged until the review returns.

If the environment cannot run subagents, do a second, separate self-review
pass against the same checklists and say so in the PR.

**Done when:** the review result is recorded against the candidate.

## 4. Resolve findings

Verify each finding against reachable code. Fix the real ones together, explain
rejected ones, and rerun the checks affected by the fixes. Ask the reviewer to
re-check only when a fix changes behavior, ownership, resource use, an
interface, or a stored format. Stop after two review rounds. The PR stays in
draft, and you report to the owner instead of starting a third round, when a
verified finding is still open or a fix after round two changed behavior,
ownership, resource use, an interface, or a stored format without review.

**Done when:** no verified finding is unaddressed and checks pass on the final
diff.

## 5. Code review at xhigh

Commit all fixes and push, then run the `xhigh` code review from the
[verification rules](../../../AGENTS.md#verification-is-not-optional) on the
pushed PR head (open a draft PR first if none exists). Fix real findings,
rerun the checks affected by the fixes, commit, push, and run it again on the
new head. After two `xhigh` rounds that
still find real issues, leave the PR in draft and report to the owner.

**Done when:** an `xhigh` review has run on the pushed final head with no
unaddressed real finding. Make no further commits before step 6.

## 6. Mark the PR ready

Mark the PR ready following the
[git rules](../../rules/git-workflow.md). In the PR description, give:

- old and new behavior, and who owns any new state or memory;
- the main design tradeoff, if there is one;
- checks run and results, including any that could not run;
- a device test plan: steps, expected results, likely failure signs, and the
  orientations, resources, and caches affected;
- any change to partitions, bootloader, OTA/recovery code, the HAL, or the SDK
  pin, with its update and rollback risk.

Wait for CI on the reviewed head before marking it ready. If CI fails, fix it
and go back to step 5 for the new head. Never claim a hardware result.

**Done when:** the PR is marked ready on a reviewed head with green CI, the description covers the points
above, and the owner has been told it is ready.
