# GitLab Flow with Release Branches

## Branch model

- Default branch: `main`
- Release maintenance branches follow `v<vendor-major>.<vendor-minor>.<vendor-patch>.x`
  - Example: `v0.8.3.x`

## Protection rules

- Protect `main` from direct pushes.
- Require merge requests with passing pipeline for merge.
- Require at least one reviewer approval.
- Protect release branches (`v*.x`) with the same merge requirements.

## Release process

1. Merge validated changes into `main`.
2. Cherry-pick or merge release-ready changes into `v0.8.3.x`.
3. Tag wrapper releases using Numeric Quad versioning (e.g., `0.8.3.0`, `0.8.3.1`).
