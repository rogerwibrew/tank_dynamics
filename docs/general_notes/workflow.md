# Hybrid AI Workflow Guide

This document explains how to use the multi-role AI workflow for your project.

## Workflow Overview

```
┌─────────────┐    ┌──────────────────┐    ┌────────────────┐
│  specs.md   │───▶│    Architect     │───▶│    plan.md     │
│  (You)      │    │    (Opus)        │    │                │
└─────────────┘    └──────────────────┘    └───────┬────────┘
                                                   │
                                                   ▼
┌─────────────┐    ┌──────────────────┐    ┌────────────────┐
│  Code       │◀───│ Senior Engineer  │◀───│    plan.md     │
│             │    │    (Sonnet)      │    │                │
└──────┬──────┘    └──────────────────┘    └────────────────┘
       │                   │
       │                   ▼
       │           ┌────────────────┐
       │           │    next.md     │
       │           └───────┬────────┘
       │                   │
       │                   ▼
       │           ┌──────────────────┐
       │           │    Engineer      │───────┐
       │           │ (Local LLM/You)  │       │
       │           └──────────────────┘       │
       │                   │                  │
       │                   ▼                  │
       │           ┌──────────────────┐       │
       │           │ Docstring Writer │       │
       │           │   (Local LLM)    │       │
       │           └──────────────────┘       │
       │                                      │
       ▼                                      ▼
┌─────────────┐    ┌──────────────────┐    ┌────────────────┐
│  Code       │◀───│  Code Reviewer   │    │ Documentation  │
│  Feedback   │    │    (Sonnet)      │    │    Writer      │
└─────────────┘    └──────────────────┘    │    (Haiku)     │
                                           └────────────────┘
```

## Step-by-Step Process

### Phase 0: Project Setup

1. Copy this template to your project directory
2. Initialize git: `git init`
3. Create initial commit: `git add . && git commit -m "Initial project setup"`

### Phase 1: Specification

**You do this yourself.**

1. Edit `docs/specs.md` with your project requirements
2. Be as detailed as possible about features, constraints, and goals
3. List any technology preferences or requirements
4. Note open questions that need architect input
5. Commit: `git add docs/specs.md && git commit -m "Add project specification"`

### Phase 2: Architecture

**Use Claude Opus.**

1. Provide the Architect prompt: `prompts/architect.md`
2. Ask it to read `docs/specs.md`
3. Let it ask clarifying questions (answer them)
4. Receive `docs/plan.md`
5. Review the plan - request changes if needed
6. Commit: `git add docs/plan.md && git commit -m "Add architectural plan"`

### Phase 3: Task Breakdown

**Use Claude Sonnet.**

1. Provide the Senior Engineer prompt: `prompts/senior-engineer.md`
2. Ask it to read `docs/plan.md`
3. Receive `docs/next.md` with 2-3 tasks
4. Verify tasks are clear and code-free
5. Commit: `git add docs/next.md && git commit -m "Add initial tasks"`

### Phase 4: Implementation Loop

**Use Local LLM + You (or Haiku for complex tasks).**

Repeat for each task:

1. Read the task from `docs/next.md`
2. Provide the Engineer prompt: `prompts/engineer.md`
3. Implement the task
4. Add docstrings (use `prompts/docstring-writer.md` if needed)
5. Verify against acceptance criteria
6. Commit: `git add <files> && git commit -m "Task N: Description"`
7. Mark task complete in `next.md`

After completing all tasks in `next.md`:
- Return to Senior Engineer for next batch of tasks
- Continue until phase is complete

### Phase 5: Review (Every 5-10 Tasks or Phase End)

**Use Claude Sonnet.**

1. Provide the Code Reviewer prompt: `prompts/code-reviewer.md`
2. Ask it to review changes since last review
3. Receive `docs/feedback.md`
4. Address critical and major issues before continuing
5. Commit: `git add docs/feedback.md && git commit -m "Code review: Phase N"`

### Phase 6: Documentation (Every 3-4 Tasks)

**Use Claude Haiku.**

1. Provide the Documentation Writer prompt: `prompts/documentation-writer.md`
2. Ask it to update documentation
3. Review and commit documentation updates

## Model Selection Quick Reference

| Situation | Model to Use |
|-----------|--------------|
| Creating the plan | Opus |
| Breaking down tasks | Sonnet |
| Backend code (Python, C++, Go, etc.) | Local LLM |
| Frontend code (React, Vue, etc.) | Haiku |
| Stuck on implementation | Haiku |
| Code review | Sonnet |
| Documentation | Haiku |
| Docstrings | Local LLM |
| Shell/terminal commands | You or Claude |

## Escalation Protocol

When to switch from Local LLM to Haiku/Sonnet:

1. **Framework unfamiliarity** - React, Vue, complex libraries
2. **Repeated failures** - Same error twice
3. **Spec mismatch** - Output doesn't match requirements after 2 attempts
4. **System operations** - File creation, terminal commands
5. **Multi-file refactoring** - Complex changes across files

To escalate, the engineer outputs:
```
ESCALATE: [reason]
```

Then switch to Haiku or Sonnet for that task.

## Git Workflow

### Commit Frequency
- Commit after each completed task
- Commit after each phase
- Commit after documentation updates
- Commit after addressing review feedback

### Commit Message Format
```
Task N: Brief description of what was done

- Detail 1
- Detail 2
```

### Using Git for Context
Instead of re-reading entire files, use:
```bash
git diff HEAD~3          # Changes in last 3 commits
git log --oneline -10    # Recent commit history
git show HEAD            # Last commit details
```

## Tips for Success

### Prompt Preparation
- Always read the role prompt before starting
- Include relevant context (plan.md, next.md) in conversation
- Clear conversation/context between tasks for local LLM

### Task Management
- Only work on one task at a time
- Verify acceptance criteria before marking complete
- Don't skip tasks or do them out of order

### Quality Control
- Review feedback seriously - it catches real issues
- Don't let minor issues accumulate
- Test as you go, not just at the end

### Context Management
- Keep conversations focused on single tasks
- Use git to reduce need for re-reading files
- Clear local LLM context between tasks

## Troubleshooting

### Local LLM produces bad output
1. Check if task is too complex (escalate to Haiku)
2. Verify task description is clear (may need Senior Engineer revision)
3. Try breaking task into smaller pieces

### Tasks are unclear
1. Return to Senior Engineer for clarification
2. Ask specific questions about ambiguous parts
3. Check if plan.md has relevant context

### Workflow feels slow
1. Batch documentation updates
2. Only review at phase boundaries
3. Use parallel work where tasks are independent

### Context getting too large
1. Clear local LLM context more frequently
2. Use git diff instead of reading files
3. Summarize relevant parts of plan.md for tasks
