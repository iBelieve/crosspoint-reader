## Git Workflow and Repository Awareness

### Repository Detection Protocol

Use the session's branch, remote, and status checks; inspect their roles rather
than assuming remote names. In this fork, `origin` is normally
`iBelieve/crosspoint-reader`. An `upstream` remote, when present, is
`crosspoint-reader/crosspoint-reader`; never push to it.

### Git Operation Rules

- Integration and PR comparisons target `develop`, not `master` or remote HEAD.
- Authorization follows [How agents work here](../../AGENTS.md#how-agents-work-here):
  agents may branch, commit, push their own branches, and open PRs. Merging,
  pushes to `develop`/`master`, tags, and history rewrites on branches you did
  not create are the owner's decisions.
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
CI checks PR titles against the same types.

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
- Do not merge unless the owner asks you to. The owner normally merges after
  testing on hardware.

### Syncing With Upstream

Upstream merges into `develop` are their own PR, with no other changes. Resolve
conflicts in fork-only files (`AGENTS.md`, `SCOPE.md`, `.agents/`, the PR
template) in favor of the fork's version, then run the full verification list.
