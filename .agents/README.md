# CrossPoint Reader agent instructions

The root `AGENTS.md` is the always-loaded policy and router. Detailed repository facts live in `rules/`; reusable procedures and review axes live in `skills/`.

Inspect the relevant code and current behavior first. Use the root routing table
to find guidance for changed decisions, then read only the relevant sections.
Skill descriptions identify procedures, not an obligation to load every keyword
match. Reuse instructions already in context. Load firmware handoff after
implementation and main-agent self-review. Read-only inquiries and reviewers do
not run implementation handoff.

Fork-specific policy lives in the root `AGENTS.md` ("This fork" through
"Firmware handoff"), `SCOPE.md`, and the files listed under
[Syncing With Upstream](rules/git-workflow.md#syncing-with-upstream). Add new
fork policy to those files, and update that list if you edit another one.

`CLAUDE.md` links to the root guide. Each skill lives in a directory containing
`SKILL.md`, with a matching frontmatter `name`. Descriptions identify when an
agent should load a skill; contributors can also name it explicitly. Skills
provide decision procedures and self-review checklists; those checklists also
serve as review rubrics. Keep them short, anchor them on durable API/type/file
names, and avoid line-number references that drift. Edit the skill at its new
home rather than restoring `.skills/` or duplicating the root policy.

## Rules

- `environment.md`: host detection, PlatformIO, and local configuration
- `hardware-resources.md`: target hardware and resource protocol
- `architecture-hal.md`: build flags, repository structure, and HAL boundaries
- `coding-standards.md`: C/C++ conventions and platform pitfalls
- `ui-activities.md`: orientation, input, UI, activities, tasks, and fonts
- `testing-debugging.md`: build, formatting, CI, serial, crash, and verification procedures
- `git-workflow.md`: repository context, branches, commits, PRs, and upstream sync
- `generated-files-cache.md`: generated sources, i18n, local artifacts, and cache formats

## Firmware review

`firmware-handoff` runs one independent read-only review using these
checklists (it may split them across up to four reviewers for large changes):

- `review-correctness`
- `review-architecture`
- `review-embedded`
- `review-i18n-docs`

The [handoff skill](skills/firmware-handoff/SKILL.md) owns the workflow and
completion criteria. Each reviewer task carries the inspection-only scope;
the main agent owns test execution, builds, and diagnostic code.
