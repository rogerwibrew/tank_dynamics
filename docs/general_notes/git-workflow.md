# Git Workflow for Hybrid AI Projects

This document describes how to use Git effectively with the hybrid AI workflow.

## Why Git Matters

Git integration provides:

1. **Context efficiency** - `git diff` shows changes without re-reading entire files
2. **Natural checkpoints** - Each task completion is a commit
3. **Easy rollback** - Undo bad AI output with `git checkout`
4. **Progress tracking** - Commit history shows what was accomplished
5. **Collaboration** - Clear record of human vs AI contributions

## Setup

### Initialize Repository

```bash
# In your project directory
git init

# Create initial commit with template files
git add .
git commit -m "Initial project setup from hybrid-ai-template"
```

### Configure Git (if not already done)

```bash
git config user.name "Your Name"
git config user.email "your.email@example.com"
```

## Commit Strategy

### When to Commit

| Event | Commit? | Example Message |
|-------|---------|-----------------|
| Task completed | Yes | `Task 3: Implement user authentication` |
| Phase completed | Yes | `Complete Phase 2: API layer` |
| Plan created | Yes | `Add architectural plan` |
| Tasks defined | Yes | `Define Phase 2 tasks` |
| Documentation updated | Yes | `Update API documentation` |
| Review feedback addressed | Yes | `Address code review feedback` |
| Work in progress | No | (use stash instead) |
| Debugging/experimenting | No | (reset if needed) |

### Commit Message Format

```
[Type]: Brief description (under 50 chars)

Optional longer description explaining:
- What was done
- Why it was done
- Any important details

Task reference: docs/next.md Task N
```

**Types:**
- `Task N` - Completing a defined task
- `Phase N` - Completing a phase
- `Plan` - Architecture/planning
- `Docs` - Documentation updates
- `Review` - Addressing review feedback
- `Fix` - Bug fixes

### Example Commits

```bash
# Task completion
git commit -m "Task 3: Add password hashing to auth service

Implemented bcrypt hashing with work factor 12.
Added validation for empty passwords.

Task reference: docs/next.md Task 3"

# Phase completion
git commit -m "Complete Phase 2: Core API layer

All CRUD endpoints implemented and tested.
Authentication middleware in place.
Ready for Phase 3: Frontend integration"

# Documentation
git commit -m "Docs: Update API reference with auth endpoints"

# Review feedback
git commit -m "Review: Address security feedback from code review

- Removed internal error details from responses
- Added rate limiting to auth endpoints
- Updated logging to exclude sensitive data"
```

## Using Git for AI Context

### Show Recent Changes

Instead of asking AI to re-read files:

```bash
# Changes in last commit
git show HEAD

# Changes in last 3 commits
git diff HEAD~3

# Changes to specific file
git diff HEAD~5 -- src/api/auth.py

# Summary of what changed
git log --oneline -10
```

### Providing Context to AI

When starting a new task, give the AI:

```bash
# What was recently done
git log --oneline -5

# Current state of relevant files
git diff HEAD~2 -- src/relevant_file.py
```

This is more efficient than reading entire files.

## Recovery Operations

### Undo Uncommitted Changes

```bash
# Discard changes to specific file
git checkout -- src/bad_file.py

# Discard all uncommitted changes
git checkout -- .
```

### Undo Last Commit (keep changes)

```bash
git reset --soft HEAD~1
```

### Undo Last Commit (discard changes)

```bash
git reset --hard HEAD~1
```

### Recover Deleted File

```bash
git checkout HEAD~1 -- path/to/deleted/file.py
```

### Create Checkpoint Before Risky Operation

```bash
# Create a branch as a savepoint
git branch savepoint-before-refactor

# Do the risky work...

# If it failed, go back
git checkout savepoint-before-refactor
git branch -D main  # careful!
git branch -m main
```

## Branch Strategy (Optional)

For larger projects, consider:

```
main
├── phase-1-calculator-engine
├── phase-2-api-layer
├── phase-3-frontend
└── docs-updates
```

```bash
# Create phase branch
git checkout -b phase-2-api-layer

# Work on phase...

# Merge when complete
git checkout main
git merge phase-2-api-layer
```

## Stashing Work in Progress

If you need to switch tasks before completing:

```bash
# Save work in progress
git stash push -m "WIP: Task 3 partial implementation"

# Do something else...

# Return to work
git stash pop
```

## Viewing History

```bash
# Compact history
git log --oneline

# History with file changes
git log --stat

# History for specific file
git log --oneline -- src/api/auth.py

# Search commits by message
git log --grep="Task 3"

# Show who changed what line
git blame src/api/auth.py
```

## Tips

### Atomic Commits
Each commit should represent one logical change. If a task has multiple parts, you can commit them separately:

```bash
git add src/models/user.py
git commit -m "Task 3 (1/2): Add User model"

git add src/api/auth.py
git commit -m "Task 3 (2/2): Add auth endpoint"
```

### Stage Specific Files
Don't use `git add .` blindly:

```bash
# Add specific files
git add src/api/auth.py tests/test_auth.py

# Review what's staged
git status

# Commit
git commit -m "Task 3: Implement authentication"
```

### Check Before Commit

```bash
# See what will be committed
git diff --staged

# See unstaged changes
git diff

# Overall status
git status
```

### Use .gitignore
Keep the repository clean by ignoring:
- Build artifacts
- Dependencies (node_modules, __pycache__)
- IDE settings
- Environment files with secrets

The template includes a comprehensive `.gitignore`.

## Integration with Workflow

### Starting a Task

```bash
# Check current state
git status
git log --oneline -3

# Read the task in docs/next.md
# Implement...
```

### Completing a Task

```bash
# Review changes
git diff

# Stage relevant files
git add src/...

# Commit with clear message
git commit -m "Task N: Description"

# Update task status in next.md
# (then commit that too)
git add docs/next.md
git commit -m "Mark Task N complete"
```

### Before Code Review

```bash
# Ensure everything is committed
git status

# Generate diff for review
git diff main...HEAD  # if using branches
# or
git log --oneline -10  # show recent commits for reviewer
```
