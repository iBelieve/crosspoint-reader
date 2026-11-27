---
name: firmware-handoff
description: Final checks, one independent review, and the PR test plan for completed firmware/build changes. Use after implementation and self-review, before opening or readying a PR; excludes read-only audits and reviewer tasks.
---

# Firmware handoff

Run after implementation and self-review, before opening the PR or marking it
ready. The main agent owns the result; reviewer output is evidence, not
authority.

## 1. Prepare the candidate

Record the requirement, comparison base (`develop`), changed files, and diff.
Account for every hunk; remove or separate anything the requirement does not
explain. Self-review against the four review checklists below.

**Done when:** every hunk is explained and the self-review is done.

## 2. Run every check

Run the [verification list](../../../AGENTS.md#verification-is-not-optional):
format, host tests, `pio check`, firmware builds for affected boards, and any
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
requirement, repository path, base, diff, check results, and these four
checklists, applying only those relevant to the change:

- [review-correctness](../review-correctness/SKILL.md)
- [review-architecture](../review-architecture/SKILL.md)
- [review-embedded](../review-embedded/SKILL.md)
- [review-i18n-docs](../review-i18n-docs/SKILL.md)

Include this scope: "Inspect code, diffs, history, and recorded checks only. No
file writes, tests, builds, or new diagnostics; request needed checks from the
main agent." For a large or risky change (new app, memory ownership, HAL or
storage format changes), you may split the checklists across up to four
reviewers.

If the environment cannot run subagents, do a second, separate self-review
pass against the same checklists and say so in the PR.

**Done when:** the review result is recorded against the candidate.

## 4. Resolve findings

Verify each finding against reachable code. Fix the real ones together, explain
rejected ones, and rerun the checks affected by the fixes. Ask the reviewer to
re-check only when a fix changes behavior, ownership, resource use, an
interface, or a stored format. Stop after two review rounds; report anything
still unresolved in the PR instead of starting a third.

**Done when:** no verified finding is unaddressed and checks pass on the final
diff.

## 5. Open the PR

Commit, push, and open or update the PR following the
[git rules](../../rules/git-workflow.md). In the PR description, give:

- old and new behavior, and who owns any new state or memory;
- the main design tradeoff, if there is one;
- checks run and results, including any that could not run;
- a device test plan: steps, expected results, likely failure signs, and the
  orientations, resources, and caches affected.

Never claim a hardware result. Watch CI and fix failures.

**Done when:** the PR is open with green CI, the description covers the points
above, and the owner has been told it is ready.
