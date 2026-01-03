# Phase 0 - Setup & Architecture Testing Guide

## Overview

Phase 0 establishes the project foundation by setting up the development environment, creating the project structure, and documenting the overall architecture. This phase includes:
- Verifying PlatformIO installation
- Creating organized folder structure for code
- Documenting system architecture strategy
- Initializing Git repository
- Creating GitHub repository for version control

## Prerequisites

- ✅ **Hardware:** Not required for Phase 0 (setup only)
- ✅ **Software:**
  - Python 3.x installed
  - PlatformIO CLI installed
  - Git installed
  - GitHub CLI installed (optional but recommended)
- ✅ **Tools:**
  - Terminal/Command Prompt
  - Text editor (VSCode recommended)
  - GitHub account (for repository creation)

## Testing Procedure

### Step 1: Verify PlatformIO Installation

Check if PlatformIO is installed and accessible:

```bash
python -m platformio --version
```

**Expected Output:**
```
PlatformIO Core, version 6.x.x
```

**If not installed:**
```bash
pip install platformio
```

**Troubleshooting:**
- If `pip` not found, install Python first from python.org
- On Linux/Mac, you may need `pip3` instead of `pip`
- Use `python3` instead of `python` on some systems

---

### Step 2: Verify Folder Structure

Check that all necessary folders were created:

```bash
ls -la src/
```

**Expected Output:**
```
src/
├── hardware/      (empty folder ready for Timer3, BusController)
├── main.cpp       (skeleton Arduino code)
├── strategies/    (empty folder ready for IC strategies)
└── utils/         (empty folder ready for UART, CommandParser)
```

Also check include folders:

```bash
ls -la include/
```

**Expected Output:**
```
include/
├── hardware/      (empty, ready for headers)
├── README         (PlatformIO default)
├── strategies/    (empty, ready for headers)
└── utils/         (empty, ready for headers)
```

---

### Step 3: Verify Project Compiles

Compile the skeleton project to ensure PlatformIO and toolchain work correctly:

```bash
python -m platformio run
```

**Expected Output:**
```
Processing megaatmega2560 (platform: atmelavr; board: megaatmega2560; framework: arduino)
...
RAM:   [          ]   0.1% (used 9 bytes from 8192 bytes)
Flash: [          ]   0.3% (used 662 bytes from 253952 bytes)
========================= [SUCCESS] Took X.XX seconds =========================
```

**Success Criteria:**
- ✅ Build completes with `[SUCCESS]`
- ✅ No compilation errors
- ✅ RAM usage < 10% (should be ~0.1%)
- ✅ Flash usage < 1% (should be ~0.3%)

**Common Issues:**

**Issue:** `pio: command not found`
- **Solution:** Use `python -m platformio` instead of `pio`

**Issue:** `Platform 'atmelavr' not installed`
- **Solution:** PlatformIO will auto-install, just wait for completion

---

### Step 4: Verify Documentation Structure

Check that all documentation files were created:

```bash
ls -la CLAUDE.md Roadmap/ Strategy/ Documents/
```

**Expected Files:**
```
✅ CLAUDE.md                           (Development guidelines)
✅ Roadmap/Roadmap.md                  (6-phase development plan)
✅ Strategy/00-Overall-Architecture.md (Architecture design)
✅ Documents/Multi-IC_Firmware_Spec.md (Firmware specification)
✅ Documents/Multi-IC_Tester_Pinout.md (Hardware pinout)
✅ Documents/Pinouts/*.md              (Individual IC pinouts)
```

Verify CLAUDE.md contains development guidelines:

```bash
grep "Development Guidelines" CLAUDE.md
```

**Expected:** Should find the section header.

Verify Roadmap contains all 6 phases:

```bash
grep "## Phase" Roadmap/Roadmap.md
```

**Expected Output:** Should list Phase 0 through Phase 6.

---

### Step 5: Verify Git Repository Initialization

Check Git repository status:

```bash
git status
```

**Expected Output:**
```
On branch master
nothing to commit, working tree clean
```

Check initial commit:

```bash
git log --oneline
```

**Expected Output:**
```
ba7b128 Initial commit: Phase 0 complete - Project setup and architecture
```

Check that .gitignore is working:

```bash
git status
```

**Expected:** Build artifacts (.pio folder) should NOT appear in untracked files.

---

### Step 6: Verify GitHub Repository

Check remote repository is configured:

```bash
git remote -v
```

**Expected Output:**
```
origin  https://github.com/icaroNZ/Multi-IC-Tester.git (fetch)
origin  https://github.com/icaroNZ/Multi-IC-Tester.git (push)
```

Verify code was pushed to GitHub:

```bash
gh repo view
```

**Expected:** Should display repository information.

**Alternative (without gh CLI):** Visit repository in browser:
```
https://github.com/icaroNZ/Multi-IC-Tester
```

**Verify on GitHub:**
- ✅ All files are present (CLAUDE.md, Roadmap/, Strategy/, src/, etc.)
- ✅ .gitignore is working (.pio folder not uploaded)
- ✅ Initial commit message is visible
- ✅ Repository description is set

---

## Expected Results Summary

After completing all steps, you should have:

### ✅ Development Environment
- [x] Python installed and working
- [x] PlatformIO installed and accessible via `python -m platformio`
- [x] Git installed and configured
- [x] GitHub CLI installed (optional)

### ✅ Project Structure
- [x] Folder structure created (`src/strategies/`, `src/hardware/`, `src/utils/`)
- [x] Include folders created (`include/strategies/`, etc.)
- [x] Tests folder created for testing documentation

### ✅ Documentation
- [x] CLAUDE.md with comprehensive development guidelines
- [x] Roadmap.md with 6 phases and 40+ tasks
- [x] Strategy/00-Overall-Architecture.md with system design
- [x] All firmware specs and pinout documents present

### ✅ Version Control
- [x] Git repository initialized locally
- [x] .gitignore configured for PlatformIO/Arduino
- [x] Initial commit created with descriptive message
- [x] GitHub repository created
- [x] Code pushed to GitHub successfully

### ✅ Compilation
- [x] Skeleton code compiles without errors
- [x] RAM usage: ~0.1% (9 bytes / 8192 bytes)
- [x] Flash usage: ~0.3% (662 bytes / 253952 bytes)

---

## Troubleshooting

### Issue: PlatformIO not found

**Error:**
```
bash: pio: command not found
```

**Solution:**
Always use `python -m platformio` instead of `pio`:
```bash
python -m platformio run
```

---

### Issue: Git repository creation failed

**Error:**
```
fatal: not a git repository
```

**Solution:**
Initialize git repository:
```bash
git init
git add .
git commit -m "Initial commit"
```

---

### Issue: GitHub authentication failed

**Error:**
```
gh: To use GitHub CLI, please authenticate first
```

**Solution:**
Authenticate with GitHub:
```bash
gh auth login
```
Follow the prompts to authenticate via browser or token.

---

### Issue: Compilation fails with missing platform

**Error:**
```
Platform 'atmelavr' has not been installed yet
```

**Solution:**
This is normal on first build. PlatformIO will automatically download the platform. Wait for the download to complete and compilation will proceed.

---

### Issue: Permission denied when creating folders

**Error:**
```
mkdir: cannot create directory: Permission denied
```

**Solution:**
- Windows: Run terminal as Administrator
- Linux/Mac: Check folder permissions or use `sudo` if necessary
- Ensure you're in the correct project directory

---

## Success Criteria

Phase 0 is complete when ALL of the following are true:

- [ ] ✅ PlatformIO installed and can compile skeleton code
- [ ] ✅ All folder structure created (src/strategies/, src/hardware/, src/utils/)
- [ ] ✅ CLAUDE.md contains development guidelines with 8 mandatory practices
- [ ] ✅ Roadmap.md contains all 6 phases with detailed tasks
- [ ] ✅ Strategy/00-Overall-Architecture.md documents system architecture
- [ ] ✅ Git repository initialized with proper .gitignore
- [ ] ✅ Initial commit created with comprehensive commit message
- [ ] ✅ GitHub repository created and accessible online
- [ ] ✅ Code pushed to GitHub successfully
- [ ] ✅ Compilation succeeds with no errors
- [ ] ✅ Ready to proceed to **Phase 1: Foundation & Infrastructure**

---

## Next Steps

✅ **Phase 0 Complete!**

You are now ready to proceed to **Phase 1: Foundation & Infrastructure**, which includes:
- Implementing UART Handler for serial communication
- Creating Command Parser for command processing
- Defining ICTestStrategy base class
- Creating pin definitions header
- Implementing Mode Manager
- Integrating everything in main.cpp

**To start Phase 1:**
1. Read `Roadmap/Roadmap.md` - Phase 1 section
2. Create `Strategy/01-Phase1-Foundation.md` strategy document
3. Begin with Phase 1, Item 1.1: Implement UART Handler

---

## Verification Checklist

Before moving to Phase 1, verify each item:

```bash
# Verify PlatformIO
python -m platformio --version

# Verify folder structure
ls -la src/strategies src/hardware src/utils

# Verify compilation
python -m platformio run

# Verify git repository
git log --oneline

# Verify GitHub repository
gh repo view
# OR visit: https://github.com/icaroNZ/Multi-IC-Tester

# Verify documentation
ls -la CLAUDE.md Roadmap/Roadmap.md Strategy/00-Overall-Architecture.md
```

**All commands should succeed with expected output.**

---

**Phase 0 Testing Complete! ✅**

Repository: https://github.com/icaroNZ/Multi-IC-Tester
