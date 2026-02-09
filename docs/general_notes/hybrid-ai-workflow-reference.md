# Hybrid AI Coding: A Comprehensive Reference Guide

## Using Claude Models and Local LLMs Together for Software Development

This document provides a detailed reference for implementing a hybrid AI workflow that combines cloud-based Claude models (Opus, Sonnet, Haiku) with local LLMs to build software projects efficiently and cost-effectively.

---

## Table of Contents

1. [Overview](#overview)
2. [Core Philosophy](#core-philosophy)
3. [The Role System](#the-role-system)
4. [Document Flow](#document-flow)
5. [Role Definitions and Prompts](#role-definitions-and-prompts)
6. [Model Selection Guide](#model-selection-guide)
7. [Escalation Protocol](#escalation-protocol)
8. [Git Integration](#git-integration)
9. [Quality Assurance Cycle](#quality-assurance-cycle)
10. [Context Management](#context-management)
11. [Practical Workflow Example](#practical-workflow-example)
12. [Template Files](#template-files)
13. [Troubleshooting](#troubleshooting)
14. [Best Practices](#best-practices)

---

## Overview

The hybrid AI workflow divides software development into specialized roles, each assigned to the most appropriate AI model based on task complexity, cost, and capability requirements.

### Why Hybrid?

| Approach | Pros | Cons |
|----------|------|------|
| Cloud-only (Opus/Sonnet) | Highest capability | Expensive, rate limits, latency |
| Local-only | Free, private, fast | Limited capability, context windows |
| **Hybrid** | Best of both worlds | Requires coordination |

The hybrid approach uses:
- **Claude Opus**: Strategic planning requiring deep reasoning
- **Claude Sonnet**: Task decomposition and code review
- **Claude Haiku**: Documentation and simpler frontend work
- **Local LLMs**: High-volume implementation tasks (backend code, tests, docstrings)

### Cost Optimization

By routing ~70% of implementation work to local LLMs and reserving cloud models for planning and review, you can reduce API costs significantly while maintaining quality through structured oversight.

---

## Core Philosophy

### 1. Separation of Concerns

Each role has **strict boundaries**. The Architect never writes code. The Engineer never plans. The Senior Engineer never implements. This separation:
- Prevents scope creep
- Ensures appropriate models handle appropriate tasks
- Creates clear handoff points
- Makes debugging easier (you know which stage failed)

### 2. Document-Driven Development

All communication between roles happens through markdown documents:
- `specs.md` → Human requirements
- `plan.md` → Architectural decisions
- `next.md` → Actionable tasks
- `feedback.md` → Quality issues

This creates an auditable trail and allows asynchronous work.

### 3. Git as the Source of Truth

Every task completion = a commit. This provides:
- Natural checkpoints
- Easy rollback from bad AI output
- Context for reviews (use `git diff` instead of re-reading files)
- Progress visibility

### 4. Escalation Over Guessing

When a local LLM struggles, it should escalate rather than produce bad output. A clear escalation protocol prevents wasted iterations.

---

## The Role System

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              WORKFLOW DIAGRAM                               │
└─────────────────────────────────────────────────────────────────────────────┘

                    ┌─────────────┐
                    │  specs.md   │  ← You write this
                    │  (Human)    │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  ARCHITECT  │  ← Claude Opus
                    │             │
                    │ • Reviews specs
                    │ • Asks questions
                    │ • Designs system
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  plan.md    │  ← Architectural plan
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │   SENIOR    │  ← Claude Sonnet
                    │  ENGINEER   │
                    │             │
                    │ • Reads plan
                    │ • Creates tasks
                    │ • NO CODE
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  next.md    │  ← 2-3 detailed tasks
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
           ▼               ▼               ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │  ENGINEER   │ │  DOCSTRING  │ │    DOCS     │
    │             │ │   WRITER    │ │   WRITER    │
    │ Local LLM   │ │ Local LLM   │ │ Claude Haiku│
    │ + Human     │ │             │ │             │
    └──────┬──────┘ └─────────────┘ └─────────────┘
           │
           │ (every 5-10 tasks)
           ▼
    ┌─────────────┐
    │    CODE     │  ← Claude Sonnet
    │  REVIEWER   │
    │             │
    │ • Reviews code
    │ • NO fixes
    │ • feedback.md
    └─────────────┘
```

### Role Summary Table

| Role | Model | Input | Output | Key Constraint |
|------|-------|-------|--------|----------------|
| Architect | Opus | specs.md | plan.md | No code, ask questions |
| Senior Engineer | Sonnet | plan.md | next.md | No code, prose only |
| Engineer | Local LLM / Human | next.md | Code + commits | Follow spec exactly |
| Docstring Writer | Local LLM | Code | Documented code | No logic changes |
| Documentation Writer | Haiku | Code + plan | README, guides | Keep current |
| Code Reviewer | Sonnet | Code (git diff) | feedback.md | No fixes, only critique |

---

## Document Flow

### specs.md — Project Specification

**Created by:** Human
**Purpose:** Define what you want to build

**Structure:**
```markdown
# Project Specification

## Overview
[What is this project?]

## Goals
[What should it accomplish?]

## Non-Goals
[What is explicitly out of scope?]

## Users
[Who will use this?]

## Features
### Feature 1: [Name]
- Description
- User Story
- Requirements
- Acceptance Criteria

## Technical Requirements
- Performance
- Security
- Compatibility
- Dependencies

## Constraints
[Limitations, deadlines, budget]

## Open Questions
[Things you need the Architect to clarify]

## References
[Links to relevant documentation]
```

### plan.md — Architectural Plan

**Created by:** Architect (Claude Opus)
**Purpose:** Strategic decisions and system design

**Structure:**
```markdown
# Project Plan: [Name]

## Executive Summary
[2-3 sentence overview]

## Architecture Overview
[Diagram + explanation of major components]

## Technology Decisions
| Component | Technology | Rationale |
|-----------|------------|-----------|

## Component Breakdown
### Component 1: [Name]
- Purpose:
- Responsibilities:
- Interfaces: (described in prose, not code)

## Implementation Phases
### Phase 1: [Name]
- Goals:
- Deliverables:
- Dependencies:

## Risk Assessment
| Risk | Likelihood | Impact | Mitigation |

## Testing Strategy
[How to verify correctness]

## Success Criteria
- [ ] Criterion 1
- [ ] Criterion 2
```

### next.md — Task Queue

**Created by:** Senior Engineer (Claude Sonnet)
**Purpose:** Detailed, actionable tasks for implementation

**Structure:**
```markdown
# Next Tasks

## Current Phase: [Phase N - Name]
**Status:** [Progress description]
**Recent Commits:** [List of completed work]

---

## Task N: [Title]

**Phase:** [Which phase]
**Prerequisites:** [Prior tasks required]

### Files to Create/Modify
- [Exact file paths]

### Requirements
[Prose description of what to build]
[Behavior for normal cases]
[Behavior for edge cases]
[Error handling]

### Interface Descriptions
[Method names, parameters, returns — in prose, NOT code]

### Verification
[How to test completion]

### Acceptance Criteria
- [ ] Criterion 1
- [ ] Criterion 2
```

**Critical Rule:** The Senior Engineer NEVER writes code. All specifications are prose. This forces clarity and prevents the Engineer from just copying code.

### feedback.md — Code Review

**Created by:** Code Reviewer (Claude Sonnet)
**Purpose:** Quality assurance and improvement suggestions

**Structure:**
```markdown
# Code Review: [Date/Phase]

## Summary
[Overall assessment]

## Critical Issues
[Must fix before proceeding]

## Major Issues
[Should fix soon]

## Minor Issues
[Fix when convenient]

## Notes
[Observations without required action]

## Positive Observations
[What was done well]

## Recommended Actions
[Prioritized list]
```

---

## Role Definitions and Prompts

Each role has a dedicated prompt file in `prompts/`. These are provided to the AI at the start of a session.

### Architect Prompt (prompts/architect.md)

**Key responsibilities:**
1. Read `docs/specs.md` completely
2. Ask clarifying questions — never assume
3. Create comprehensive `docs/plan.md`
4. Justify all technology decisions
5. Consider implementers (local LLMs need extra clarity)

**Forbidden actions:**
- Writing implementation code
- Making assumptions about unclear requirements
- Skipping the questioning phase

**Deliverable format:**
- Executive Summary
- Architecture Overview (with diagram)
- Technology Decisions table
- Component Breakdown
- Implementation Phases
- Risk Assessment
- Success Criteria

### Senior Engineer Prompt (prompts/senior-engineer.md)

**Key responsibilities:**
1. Read `docs/plan.md` for architecture
2. Check `docs/next.md` for current progress
3. Generate 2-3 detailed, actionable tasks
4. Ensure tasks are clear enough for a local LLM

**Forbidden actions (critical):**
- Writing code (no functions, classes, implementations)
- Including code blocks
- Using programming syntax
- Showing "example code"

**Task specification requirements:**
- Exact file paths
- Behavioral requirements (inputs → outputs)
- Edge cases and error handling
- Interface descriptions in prose
- Verification steps
- Acceptance criteria checklist

**Task granularity:** Each task should be completable in 30-60 minutes, independently testable, and clearly bounded.

### Engineer Prompt (prompts/engineer.md)

**Key responsibilities:**
1. Read current task in `docs/next.md`
2. Implement exactly what is specified
3. Ask for clarification when unclear
4. Write clean, tested code
5. Commit after completion

**Process:**
1. Read task completely before coding
2. Understand context, files, expected behavior
3. Ask questions if unclear (specific, not vague)
4. Implement according to spec
5. Verify against acceptance criteria
6. Commit with clear message

**Scope discipline:**
- Implement ONLY what the task specifies
- No "nice to have" features
- No refactoring unrelated code
- No extra error handling beyond spec

**Escalation triggers:**
- Unfamiliar framework
- Same error twice
- Spec mismatch after 2 attempts
- Shell commands or system operations
- Complex multi-file refactoring

**Escalation format:**
```
ESCALATE: [reason]
```

**Stuck format:**
```
STUCK: [problem description]
ATTEMPTED: [what was tried]
NEED: [what would help]
```

### Code Reviewer Prompt (prompts/code-reviewer.md)

**Key responsibilities:**
1. Review code changes since last review
2. Evaluate quality, design, best practices
3. Identify bugs and architectural issues
4. Provide constructive feedback

**Forbidden actions:**
- Writing code
- Implementing fixes
- Refactoring

**Review checklist:**
- **Correctness:** Does it meet spec? Edge cases? Bugs?
- **Design:** Follows architecture? Proper separation?
- **Readability:** Clear names? Self-documenting?
- **Maintainability:** Easy to modify? Hidden dependencies?
- **Security:** Input validation? No hardcoded secrets?
- **Performance:** Obvious inefficiencies? N+1 queries?

**Severity levels:**

| Level | Meaning | Action |
|-------|---------|--------|
| Critical | Bug or security issue | Must fix before proceeding |
| Major | Design problem | Should fix soon |
| Minor | Style or improvement | Fix when convenient |
| Note | Observation | No action required |

**Review frequency:**
- Every 5-10 engineer tasks
- End of each phase
- Before major milestones

### Docstring Writer Prompt (prompts/docstring-writer.md)

**Key responsibilities:**
- Add docstrings to functions, methods, classes
- Follow language-specific conventions
- Keep docstrings concise but complete

**Documentation standards by language:**

**Python (Google Style):**
```python
def function(param: Type) -> ReturnType:
    """Brief description.

    Args:
        param: Description of parameter.

    Returns:
        Description of return value.

    Raises:
        ExceptionType: When this happens.
    """
```

**C++ (Doxygen):**
```cpp
/**
 * @brief Brief description.
 *
 * @param param Description of parameter.
 * @return Description of return value.
 * @throws ExceptionType When this happens.
 */
```

**What to document:**
- Public functions and methods
- Classes and their purpose
- Complex algorithms
- Non-obvious parameters
- Return values
- Exceptions

**What to skip:**
- Obvious getters/setters
- Private helpers with clear names
- Single-line lambdas
- Test functions

### Documentation Writer Prompt (prompts/documentation-writer.md)

**Key responsibilities:**
- Create and maintain README
- Write user guides and tutorials
- Create API documentation
- Keep docs in sync with implementation

**Documentation types:**
- `README.md` — Project overview, quick start
- `docs/USER_GUIDE.md` — End-user documentation
- `docs/DEVELOPER_GUIDE.md` — Contributor setup
- `docs/API.md` — API reference

**Update frequency:** Every 3-4 engineer tasks

---

## Model Selection Guide

### When to Use Each Model

| Situation | Model | Reasoning |
|-----------|-------|-----------|
| Creating architectural plan | **Opus** | Requires deep reasoning, trade-off analysis |
| Breaking down tasks | **Sonnet** | Needs understanding of implementation complexity |
| Backend code (Python, C++, Go) | **Local LLM** | Structured, well-defined tasks |
| Frontend code (React, Vue) | **Haiku** | Framework familiarity important |
| Stuck on implementation | **Haiku → Sonnet** | Escalate if Haiku insufficient |
| Code review | **Sonnet** | Needs architectural context |
| Documentation | **Haiku** | Straightforward writing task |
| Docstrings | **Local LLM** | Mechanical, high-volume |
| Shell/terminal commands | **Human / Claude** | Safety, system access |
| Complex debugging | **Sonnet** | Requires broad context |

### Local LLM Suitability

Local LLMs work well for:
- Implementing well-specified functions
- Writing unit tests from specs
- Adding docstrings
- Simple refactoring
- Repetitive code patterns

Local LLMs struggle with:
- Unfamiliar frameworks
- Complex multi-file changes
- Ambiguous requirements
- System operations
- Frontend/UI work

### Cost-Benefit Analysis

Assuming typical token costs:

| Model | Cost per 1M tokens (input) | Best for |
|-------|---------------------------|----------|
| Opus | ~$15 | Planning (1-2 calls per project) |
| Sonnet | ~$3 | Task breakdown, reviews (5-10 calls) |
| Haiku | ~$0.25 | Docs, simple implementation |
| Local | $0 | High-volume implementation |

A typical project might use:
- 1 Opus call (planning)
- 10 Sonnet calls (tasks + reviews)
- 20 Haiku calls (docs + frontend)
- 100+ local LLM calls (implementation)

---

## Escalation Protocol

### When to Escalate

The Engineer (local LLM or human) should escalate when:

1. **Framework unfamiliarity** — Task involves React, Vue, or complex libraries the model doesn't know well
2. **Repeated failures** — Same error occurs twice
3. **Spec mismatch** — Output doesn't match requirements after 2 attempts
4. **System operations** — File creation, terminal commands, environment changes
5. **Multi-file refactoring** — Complex changes across many files
6. **Contradictory spec** — Task requirements are impossible or conflicting

### Escalation Format

```
ESCALATE: [reason]
```

Example:
```
ESCALATE: Task requires React context API which I'm unfamiliar with.
The component needs to consume a theme context but I'm unsure of the
correct hook usage pattern for this codebase.
```

### Escalation Path

```
Local LLM
    │
    ▼ (framework issues, simple errors)
Claude Haiku
    │
    ▼ (architectural issues, complex bugs)
Claude Sonnet
    │
    ▼ (fundamental design questions)
Claude Opus (or back to Architect phase)
```

### Stuck Protocol

If unable to complete but not a clear escalation case:

```
STUCK: [description of the problem]
ATTEMPTED: [what you tried]
NEED: [what information or clarification would help]
```

The human or higher-tier model can then provide targeted assistance.

---

## Git Integration

### Why Git is Essential

Git provides:
1. **Context efficiency** — `git diff` shows changes without re-reading files
2. **Natural checkpoints** — Each task = one commit
3. **Easy rollback** — Undo bad AI output instantly
4. **Progress tracking** — Commit history shows accomplishments
5. **Review support** — Reviewers see exactly what changed

### Commit Strategy

| Event | Commit? | Message Format |
|-------|---------|----------------|
| Task completed | Yes | `Task N: Brief description` |
| Phase completed | Yes | `Complete Phase N: Description` |
| Plan created | Yes | `Add architectural plan` |
| Tasks defined | Yes | `Define Phase N tasks` |
| Docs updated | Yes | `Docs: Update API reference` |
| Review feedback addressed | Yes | `Review: Address feedback` |
| Work in progress | No | Use `git stash` instead |

### Commit Message Format

```
Task N: Brief description (under 50 chars)

Optional longer description:
- What was done
- Why it was done
- Important details

Task reference: docs/next.md Task N
```

### Using Git for AI Context

Instead of asking AI to re-read entire files:

```bash
# What changed in last commit
git show HEAD

# Changes in last 3 commits
git diff HEAD~3

# Changes to specific file
git diff HEAD~5 -- src/specific_file.py

# Recent history summary
git log --oneline -10
```

### Recovery Commands

```bash
# Discard uncommitted changes to file
git checkout -- src/bad_file.py

# Undo last commit (keep changes)
git reset --soft HEAD~1

# Undo last commit (discard changes)
git reset --hard HEAD~1

# Create savepoint before risky operation
git branch savepoint-before-refactor
```

---

## Quality Assurance Cycle

### The Review Loop

```
┌──────────────┐
│   Engineer   │
│  implements  │
│   5-10       │
│   tasks      │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│    Code      │
│   Reviewer   │
│   (Sonnet)   │
└──────┬───────┘
       │
       ▼
┌──────────────┐     ┌──────────────┐
│  feedback.md │────▶│   Engineer   │
│              │     │  addresses   │
│ • Critical   │     │   issues     │
│ • Major      │     └──────────────┘
│ • Minor      │
└──────────────┘
```

### Review Triggers

- Every 5-10 completed tasks
- End of each implementation phase
- Before major milestones (releases, demos)
- When requested by team

### Priority of Fixes

1. **Critical** — Must fix before any new work
2. **Major** — Fix before end of current phase
3. **Minor** — Fix when convenient, batch if possible

### Positive Reinforcement

Reviews should also highlight:
- Well-structured code
- Good naming conventions
- Proper error handling
- Clean architecture alignment

This helps calibrate future work.

---

## Context Management

### The Context Problem

Local LLMs have limited context windows (often 4K-8K tokens). Even cloud models benefit from focused context.

### Strategies

**1. Clear context between tasks**
- Start fresh conversation for each task
- Don't carry over implementation details from previous tasks
- The task spec in `next.md` should be self-contained

**2. Use git instead of file reads**
```bash
# Show what changed, not entire file
git diff HEAD~1 -- src/component.py
```

**3. Reference documents by section**
- "See Phase 2 in plan.md" rather than pasting entire plan
- "Per Task 3 acceptance criteria" rather than repeating them

**4. Summarize when handing off**
- At phase boundaries, create summaries
- "Phase 1 complete: TankModel and PIDController implemented with 15 passing tests"

### Task Self-Containment

Each task in `next.md` should include ALL information needed:
- Exact file paths
- Complete behavioral requirements
- Edge cases
- Verification steps

The Engineer should not need to read `plan.md` or `specs.md` for most tasks.

---

## Practical Workflow Example

### Phase 0: Setup

```bash
# Clone template or initialize project
git clone https://github.com/user/hybrid-ai-template my-project
cd my-project
git add . && git commit -m "Initial project setup"
```

### Phase 1: Specification (Human)

1. Edit `docs/specs.md` with your requirements
2. Be thorough — the Architect can only work with what you provide
3. List open questions explicitly
4. Commit: `git add docs/specs.md && git commit -m "Add project specification"`

### Phase 2: Architecture (Claude Opus)

1. Provide `prompts/architect.md` to Opus
2. Ask it to read `docs/specs.md`
3. Answer its clarifying questions (this is critical!)
4. Record Q&A in `docs/architect-questions.md`
5. Receive `docs/plan.md`
6. Review — request changes if needed
7. Commit: `git add docs/ && git commit -m "Add architectural plan"`

### Phase 3: Task Breakdown (Claude Sonnet)

1. Provide `prompts/senior-engineer.md` to Sonnet
2. Ask it to read `docs/plan.md`
3. Receive `docs/next.md` with 2-3 tasks
4. Verify tasks are clear and code-free
5. Commit: `git add docs/next.md && git commit -m "Define initial tasks"`

### Phase 4: Implementation Loop

For each task:

1. **Read** — Local LLM reads task from `next.md`
2. **Ask** — Clarify if anything is ambiguous
3. **Implement** — Write code per specification
4. **Document** — Add docstrings (Docstring Writer)
5. **Verify** — Check acceptance criteria
6. **Commit** — `git commit -m "Task N: Description"`
7. **Update** — Mark task complete in `next.md`

After completing all tasks in `next.md`:
- Return to Senior Engineer (Sonnet) for next batch
- Continue until phase complete

### Phase 5: Review (Every 5-10 Tasks)

1. Provide `prompts/code-reviewer.md` to Sonnet
2. Share `git log` and relevant diffs
3. Receive `docs/feedback.md`
4. Address Critical and Major issues
5. Commit: `git add . && git commit -m "Review: Address feedback"`

### Phase 6: Documentation (Every 3-4 Tasks)

1. Provide `prompts/documentation-writer.md` to Haiku
2. Ask it to update README and guides
3. Review and commit updates

### Repeat

Continue the Implementation → Review → Documentation cycle until project complete.

---

## Template Files

### CLAUDE.md (Project Root)

This file configures AI behavior for the project:

```markdown
# CLAUDE.md - Hybrid AI Project Configuration

## Project Overview
[Brief description]

## Hybrid AI Workflow
[Reference to role system]

### Role Definitions
[Summary of each role with responsibilities and prompts]

## Critical Rules

### Before Starting ANY Work
1. IDENTIFY YOUR ROLE
2. READ THE ROLE PROMPT
3. FOLLOW ROLE BOUNDARIES

### Role Boundary Enforcement
- Architects: No implementation details
- Senior Engineers: NO CODE, prose only
- Code Reviewers: No code generation
- Engineers: Follow spec exactly

## Git Workflow
[Commit conventions]

## Escalation Policy
[When to switch models]

## Technology Stack
[Project-specific technologies]

## Success Criteria
[Definition of done]
```

### Directory Structure

```
project/
├── CLAUDE.md                    # AI workflow configuration
├── prompts/                     # Role prompts
│   ├── architect.md
│   ├── senior-engineer.md
│   ├── engineer.md
│   ├── code-reviewer.md
│   ├── docstring-writer.md
│   └── documentation-writer.md
├── docs/                        # Project documentation
│   ├── specs.md                 # Your requirements
│   ├── plan.md                  # Architect output
│   ├── next.md                  # Current tasks
│   ├── feedback.md              # Review results
│   ├── architect-questions.md   # Q&A from planning
│   └── workflow.md              # Process reference
├── src/                         # Source code
├── tests/                       # Test files
└── README.md                    # Project readme
```

---

## Troubleshooting

### Local LLM Produces Bad Output

**Symptoms:** Code doesn't compile, logic errors, misses requirements

**Solutions:**
1. Check if task is too complex → escalate to Haiku
2. Verify task description is clear → may need Senior Engineer revision
3. Break task into smaller pieces
4. Provide more context in the task spec

### Tasks Are Unclear

**Symptoms:** Engineer asks many questions, produces wrong output

**Solutions:**
1. Return to Senior Engineer for clarification
2. Ask specific questions about ambiguous parts
3. Check if `plan.md` has relevant context
4. The Senior Engineer may need to read more of the codebase

### Workflow Feels Slow

**Symptoms:** Too much overhead, context switching

**Solutions:**
1. Batch documentation updates (every 3-4 tasks, not every task)
2. Only review at phase boundaries, not after every task
3. Use parallel work where tasks are independent
4. Pre-write several tasks in `next.md`

### Context Getting Too Large

**Symptoms:** Local LLM quality degrades, responses slow

**Solutions:**
1. Clear local LLM context more frequently (every 1-2 tasks)
2. Use `git diff` instead of reading full files
3. Summarize relevant parts of `plan.md` in task descriptions
4. Keep tasks truly self-contained

### Code Review Finds Many Issues

**Symptoms:** Long feedback.md, many critical issues

**Solutions:**
1. Review the task specifications — were they clear enough?
2. Consider escalating implementation to higher-tier model
3. Add more edge cases to task specs
4. Review earlier (every 3-5 tasks instead of 5-10)

### Engineer and Spec Disagree

**Symptoms:** Implementation doesn't match requirements

**Solutions:**
1. Re-read the specification carefully
2. Check if spec was updated after implementation started
3. Escalate with specific question about the discrepancy
4. May need Architect input if it's a design issue

---

## Best Practices

### For Specifications (specs.md)

- Be explicit about what you want
- List non-goals to prevent scope creep
- Include acceptance criteria for each feature
- Note open questions rather than leaving gaps
- Reference any existing systems or constraints

### For Architecture (plan.md)

- Justify every technology choice
- Draw diagrams (ASCII is fine)
- Break into phases with clear boundaries
- Identify risks and mitigations
- Define interfaces in prose, not code

### For Task Breakdown (next.md)

- 2-3 tasks maximum at a time
- Each task completable in 30-60 minutes
- Include ALL needed context in the task
- Specify exact file paths
- Never include code — prose only
- Clear acceptance criteria checklist

### For Implementation

- Read the full task before starting
- Ask questions early, not after 2 hours of work
- Verify against acceptance criteria before committing
- Keep commits atomic (one task = one commit)
- Escalate rather than guess

### For Code Review

- Review diffs, not entire files
- Be specific about locations
- Explain why, not just what
- Acknowledge good work
- Prioritize by severity

### For the Human

- Answer Architect questions thoroughly
- Review AI output before accepting
- Don't skip the questioning phase
- Commit frequently
- Trust the process, but verify the output

---

## References

- **GitHub Template:** The hybrid-ai-template repository provides starter files
- **Tank Dynamics Project:** A real example using this workflow (tank level control simulator)
- **Claude Documentation:** https://docs.anthropic.com/
- **Local LLM Setup:** Ollama, LM Studio, or similar for running local models

---

## Changelog

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-30 | 1.0 | Initial comprehensive reference |

---

*This document is part of the Hybrid AI Workflow system. For questions or improvements, see the project repository.*
