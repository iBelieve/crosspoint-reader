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
- PRs (and issues, if enabled) go to `iBelieve/crosspoint-reader` only. Pass the repository
  explicitly (for example `gh pr create -R iBelieve/crosspoint-reader`, or the
  REST API `repos/iBelieve/crosspoint-reader/pulls` where GraphQL is blocked),
  since tools may resolve a fork's base to the upstream parent. Anything opened
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

- Open the PR against `develop` as a draft while work is in progress. Mark it
  ready only after the
  [verification checks](../../AGENTS.md#verification-is-not-optional) pass and,
  for firmware changes, [firmware-handoff](../skills/firmware-handoff/SKILL.md)
  steps 1-5 are done.
- Fill in the [PR template](../../.github/PULL_REQUEST_TEMPLATE.md): what
  changed and why, checks run with results, checks that could not run and why,
  memory/flash impact, and the device test plan.
- Watch CI on the PR and fix failures. A PR is ready only when CI is green.
- Before merging any PR, run a code review at `xhigh` effort on its current
  head, unless one was already run on that head (see
  [Verification](../../AGENTS.md#verification-is-not-optional)). Fix real
  findings and rerun affected checks first.
- Do not merge unless the owner asks you to. The owner normally merges after
  testing on hardware.

### Syncing With Upstream

Upstream merges into `develop` are their own PR, with no other changes. Paths
below are from the repository root.

- **Fork-owned; keep the fork's version:** `SCOPE.md`,
  `.github/PULL_REQUEST_TEMPLATE.md`, `.github/ISSUE_TEMPLATE/`,
  `.agents/rules/git-workflow.md`, `.agents/skills/firmware-handoff/`, and
  `.agents/skills/scope-discipline/`. Read upstream's change for technical
  facts worth porting, but do not take its policy.
- **Fork sections in shared files:** in `AGENTS.md`, keep the fork's "This
  fork", "How agents work here", "Verification is not optional", and "Firmware
  handoff" sections, the routing-table row for "New features, apps, ...", and
  the "Start here" wording ("tie it to the requirement", verifying for the
  owner). Upstream's "Human ownership" and "Mandatory firmware handoff"
  sections stay deleted. The same applies to the fork edits in
  `.agents/README.md`, `.agents/rules/testing-debugging.md`,
  `.agents/rules/ui-activities.md`, `README.md` (scope links), `ROADMAP.md`
  (note and upstream links), and `GOVERNANCE.md` (upstream scope link). Take
  upstream's other changes in these files.
- **Everything else** in `.agents/` is upstream technical guidance. Take
  upstream's version, but adapt or drop any upstream policy wording in it
  (human approval for commits, no agent PRs, interactive apps out of scope).
- **Merge method:** a sync PR must land as a merge commit, not a squash, so
  git records the upstream parent. If the repository allows only squash
  merges, the owner merges the sync locally and pushes it.

If you edit another file for fork policy, add it to this list. Then run the
full verification list.
