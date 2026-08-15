# Part 1

---

## Divergent Branches

**Cause:** Someone ran `git push --force`, rewriting remote history. Local and remote branches diverged — Git can't auto-merge.

**Option A — Discard local, take remote (destructive)**
```bash
git fetch origin
git reset --hard origin/main   # wipes uncommitted changes + local-only commits
```

**Option B — Keep local commits, rebase on top of remote**
```bash
git fetch origin
git rebase origin/main         # replays your commits on updated remote history
# On conflict:
git add <file>
git rebase --continue
# To cancel:
git rebase --abort
```

---

## Fetch vs Pull

| | `fetch` + review | `pull` |
|---|---|---|
| Modifies working dir | ✗ | ✓ |
| Lets you review first | ✓ | ✗ |
| Safe for team projects | ✓ safer | risky |

**Recommended workflow:**
```bash
git fetch origin
git log main..remotes/origin/main --oneline   # what's new on remote
git diff main remotes/origin/main             # exact line changes
git merge remotes/origin/main                 # merge when ready
```

---

## Compare Local vs Remote

```bash
# Commits on remote not yet local
git log main..remotes/origin/main --oneline

# Commits in old version missing from new (after force push)
git log <new_hash>..<old_hash> --oneline

# Line-level diff
git diff main remotes/origin/main
git diff main remotes/origin/main -- path/to/file.js

# Useful flags
--name-only    # files changed only, no diffs
--stat         # summary: files + line counts
```

---

## Recover Lost Commits (after force push)

```bash
git reflog origin/main          # pointer history for branch
git reflog                      # pointer history for HEAD
git reflog --date=relative      # e.g. "2 hours ago"
git reflog --all                # all branches
```
Copy the hash of the lost commit, then cherry-pick or reset to it.

---

## Visualise History

```bash
git log --oneline --graph --all -n 20

# Useful filters
--author="Name"         # by author
--stat                  # files + line counts per commit
--since="yesterday"     # time range
```

---

## `git reset` Modes

| Flag | Commits | Index | Working dir |
|---|---|---|---|
| `--soft` | undone | kept staged | unchanged |
| `--mixed` *(default)* | undone | unstaged | unchanged |
| `--hard` | undone | cleared | cleared |
| `--keep` | undone | cleared | local edits preserved if possible |

---

## Remote Branch Reference Format

```
remotes/origin/main
remotes/origin/feat/my-feature
```

```bash
git branch -a    # local + remote
git branch -r    # remote only
```

---

## Key Flags Reference

| Command | Flag | Effect |
|---|---|---|
| `git fetch` | `--prune` | remove stale remote refs |
| `git fetch` | `--all` | fetch all remotes |
| `git rebase` | `-i` | interactive: squash/rename/drop |
| `git log` | `--oneline --graph --all` | visual branch tree |
| `git diff` | `--cached` | staged changes not yet committed |

# Part 2

---

## cherry-pick

Copy a specific commit from any branch onto the current branch — without merging the whole branch.

```bash
git cherry-pick <hash>              # apply one commit
git cherry-pick <hash1> <hash2>     # apply multiple (in order)
git cherry-pick <hash1>..<hash2>    # apply a range (exclusive start)
git cherry-pick <hash1>^..<hash2>   # apply a range (inclusive start)

# On conflict:
git add <file>
git cherry-pick --continue
git cherry-pick --abort             # cancel, restore original state
```

> **Note:** The `^` inclusive range syntax (`hash1^..hash2`) works correctly on Unix-based systems (macOS, Linux). On Windows PowerShell, escape it as `` `^ `` or use `--stdin` with a range instead.

> ⚠️ Creates a **new commit** with a different hash — it's a copy, not a move. Avoid cherry-picking the same commit into multiple long-lived branches; it causes duplicate history.

---

## stash

Temporarily shelve uncommitted changes to switch context without committing half-done work.

```bash
git stash                           # stash tracked changes
git stash -u                        # include untracked files
git stash push -m "wip: login fix"  # stash with a label

git stash list                      # show all stashes
git stash show stash@{0}            # summary of latest stash
git stash show -p stash@{0}         # full diff

git stash pop                       # apply latest + remove from list
git stash apply stash@{1}           # apply specific, keep in list
git stash drop stash@{0}            # delete specific stash
git stash clear                     # delete all stashes
```

> ⚠️ `pop` can cause conflicts. On conflict: resolve → `git add` → `git stash drop` manually. Stashes are local — not pushed to remote.

---

## commit --amend

Modify the **most recent** commit — fix the message, add a forgotten file, or both.

```bash
git commit --amend -m "correct message"     # replace message only
git add forgotten.js
git commit --amend --no-edit                # add file, keep message
```

> ⚠️ Rewrites history — never amend a commit already pushed to a shared branch. Safe on your own feature branch before opening a PR.

---

## rebase -i

Rewrite, reorder, squash, or drop commits interactively — clean up a branch before merging.

```bash
git rebase -i HEAD~3        # edit last 3 commits
git rebase -i <hash>        # edit from that commit forward

git rebase --abort          # cancel, restore original state
git rebase --continue       # resume after resolving conflict
```

Commands in the editor:

| Command | Effect |
|---|---|
| `pick` | keep as-is |
| `reword` | keep commit, edit message |
| `squash` | merge into previous commit, combine messages |
| `fixup` | merge into previous commit, discard message |
| `drop` | delete this commit entirely |
| `edit` | pause here to amend files |

> **Note:** `squash` and `fixup` are the most used in practice — accumulate small "WIP" commits during development, then squash into one clean commit before opening a PR.

> ⚠️ Don't rebase commits already pushed to a shared/remote branch.

---

## bisect

Binary search through commit history to find which commit introduced a bug.

```bash
git bisect start
git bisect bad                  # current commit has the bug
git bisect good <hash>          # last known good commit

# Git checks out a middle commit — test it, then:
git bisect good                 # no bug here
git bisect bad                  # bug here
# Repeat until Git prints: "abc1234 is the first bad commit"

git bisect reset                # return to original HEAD when done
```

---

## blame

Show who last changed each line of a file and in which commit.

```bash
git blame path/to/file.js
git blame -L 20,40 path/to/file.js      # lines 20–40 only
git blame --since="2 weeks ago" file.js
```

Output per line: `<hash> (<author> <date>) <code>`

---

## tag

Mark a specific commit permanently — typically for releases.

```bash
git tag v1.0.0                          # lightweight tag (pointer only)
git tag -a v1.0.0 -m "Release 1.0.0"   # annotated tag (includes author + date)
git tag -a v1.0.0 <hash>               # tag a past commit

git tag                                 # list all tags
git show v1.0.0                         # tag details + commit info

git push origin v1.0.0                  # push one tag (tags don't push automatically)
git push origin --tags                  # push all tags
git tag -d v1.0.0                       # delete local tag
git push origin --delete v1.0.0         # delete remote tag
```

---

## clean -fd

Delete all **untracked** files and directories from the working tree.

```bash
git clean -n        # dry run — shows what would be deleted
git clean -f        # delete untracked files
git clean -fd       # delete untracked files + directories
git clean -fdx      # also delete .gitignored files (build artifacts, node_modules)
```

> **Note:** Always run `-n` first. Treat the dry run as a mandatory step — deleted untracked files cannot be recovered from Git history.

---

## worktree

Check out multiple branches simultaneously into separate folders — without stashing or switching.

```bash
git worktree add ../hotfix-branch hotfix/login-bug    # new folder, existing branch
git worktree add ../feature-temp -b feat/experiment   # new folder + new branch

git worktree list                           # show all active worktrees
git worktree remove ../hotfix-branch        # remove when done
```