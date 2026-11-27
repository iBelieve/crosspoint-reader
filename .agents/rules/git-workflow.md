## Git Workflow and Repository Awareness

### Repository Detection Protocol

Use the session's branch, remote, and status checks; inspect their roles rather
than assuming remote names. In this fork, `origin` is normally
`iBelieve/crosspoint-reader`. An `upstream` remote, when present, is
`crosspoint-reader/crosspoint-reader`; never push to it.

### Git Operation Rules

- Integration and PR comparisons target `develop`, not `master` or remote HEAD.
- Authorization follows [How agents work here](../../AGENTS.md#how-agents-work-here),
  which lists the decisions the owner keeps. Agents may branch, commit, push
  their own branches, and open PRs.
- PRs and issues go to `iBelieve/crosspoint-reader` only. Pass the repository
  explicitly (for example `gh pr create -R iBelieve/crosspoint-reader`), since
  tools may resolve a fork's base to the upstream parent. Anything opened
  upstream is the owner's decision.
- Never force-push a branch with an open PR unless you created it and no one
  else has pushed to it. Prefer a new commit over amending pushed commits.
- Keep the attribution trailers your environment asks for (for example
  `Co-Authored-By`).
- When adapting another person's work, verify the original human author and
  credit them with `Co-Authored-By`; skip bot authors.

### Branch Naming Convention

Use `<type>/<short-description>`, where type is `feat`, `fix`, `refactor`,
`docs`, `test`, `chore`, or `perf`, matching PR title prefixes. If your
environment assigns a branch name, use that instead.

### Commit Message Format

Use `<type>: <short summary>` (summary at most 50 characters), with an optional
body containing relevant facts and verification. Types match branch prefixes.
CI checks that PR titles follow Conventional Commits; use the types above.

### When to Commit

Commit when a logical step is complete and its checks pass. Do not commit
experiments, failed builds, new warnings, or ignored outputs; inspect status
and `.gitignore` before staging. Exclude unrelated work. One logical change per
commit; a pure refactor preserves behavior and goes in its own commit.

### Pull Requests

- Open the PR against `develop` once the
  [verification checks](../../AGENTS.md#verification-is-not-optional) pass, or
  as a draft while work is in progress.
- Fill in the [PR template](../../.github/PULL_REQUEST_TEMPLATE.md): what
  changed and why, checks run with results, checks that could not run and why,
  memory/flash impact, and the device test plan.
- Watch CI on the PR and fix failures. A PR is ready only when CI is green.
- Before merging any PR, run a code review at `xhigh` effort (the
  `code-review` skill) on its current head, unless one was already run on that
  head. Fix real findings and rerun affected checks first.
- Do not merge unless the owner asks you to. The owner normally merges after
  testing on hardware.

### Syncing With Upstream

Upstream merges into `develop` are their own PR, with no other changes.

- Fork-owned files: `SCOPE.md` and `.github/PULL_REQUEST_TEMPLATE.md`. Keep the
  fork's version.
- Files with fork edits: `AGENTS.md` (the "This fork" sections and the scope
  table row), `.agents/README.md`, `rules/git-workflow.md`,
  `rules/testing-debugging.md`, `rules/ui-activities.md`,
  `skills/firmware-handoff/`, `skills/scope-discipline/`, the ROADMAP note,
  and the README scope link. Take upstream's
  changes and reapply only the fork-specific lines.
- Everything else in `.agents/` is upstream technical guidance. Take
  upstream's version.

Then run the full verification list.
