# CMake Workarounds and Special Handling

**File:** `CMakeLists.txt`  
**Last Updated:** 2026-02-02

This document explains the non-standard CMake patterns used in this project and why they're necessary.

---

## Overview

The root `CMakeLists.txt` contains two important workarounds:

1. **Manual Eigen Fetch** - Prevents test pollution
2. **Precise enable_testing() Timing** - Ensures correct test discovery

Both are necessary due to how Eigen's CMakeLists.txt interacts with CTest.

---

## Workaround 1: Manual Eigen Fetch

### Standard Approach (What We DON'T Do)

```cmake
FetchContent_Declare(Eigen3 ...)
FetchContent_MakeAvailable(Eigen3)  # ❌ This causes problems
```

### Our Approach

```cmake
FetchContent_Declare(Eigen3 ...)
FetchContent_GetProperties(Eigen3)
if(NOT eigen3_POPULATED)
    FetchContent_Populate(Eigen3)  # ✅ Download only, no processing
    add_library(Eigen3::Eigen INTERFACE IMPORTED GLOBAL)
    target_include_directories(Eigen3::Eigen INTERFACE ${eigen3_SOURCE_DIR})
endif()
```

### Why?

**Problem:** `FetchContent_MakeAvailable()` calls `add_subdirectory()` on Eigen's source, which processes its `CMakeLists.txt`. Eigen's build system registers 900+ tests via `add_test()`, polluting our CTest suite.

**Solution:** Use `FetchContent_Populate()` to download Eigen without processing its CMake files, then manually create an `INTERFACE IMPORTED` target with just the include path.

**Works Because:** Eigen is header-only - we only need the headers in our include path, not Eigen's build targets or configuration.

**Trade-off:** We lose access to Eigen's CMake configuration features (e.g., `EIGEN_BUILD_DOC`), but we don't need them.

---

## Workaround 2: Precise enable_testing() Timing

### Standard Approach (What We DON'T Do)

```cmake
project(TankDynamics)
enable_testing()  # ❌ Called too early
# ... dependencies ...
add_subdirectory(tests)
```

### Our Approach

```cmake
project(TankDynamics)
# ... fetch dependencies (Eigen, GoogleTest) ...
add_subdirectory(src)
enable_testing()  # ✅ Called at exactly the right moment
add_subdirectory(tests)
```

### Why?

**Problem:** If `enable_testing()` is called before fetching Eigen, and if Eigen were fetched normally (via `FetchContent_MakeAvailable`), Eigen's tests would be registered in our suite.

**Solution:** Call `enable_testing()` after all dependencies are fetched but before our `tests/` directory.

**Works Because:**
1. Eigen is manually fetched (doesn't process its CMakeLists.txt)
2. `enable_testing()` is called before `add_subdirectory(tests/)`
3. Our tests can use `add_test()` and `gtest_discover_tests()`

**Result:** CTest discovers exactly our 17 tests, nothing more.

---

## How to Verify It's Working

```bash
# Clean build
rm -rf build
cmake -B build -S .
cmake --build build

# Check test count - should be exactly 17
ctest --test-dir build -N

# Run tests - all should pass
ctest --test-dir build
```

Expected output:
```
Test #1: TankModelTest.SteadyStateZeroDerivative
Test #2: TankModelTest.PositiveDerivativeWhenInletExceedsOutlet
...
Test #17: PIDControllerTest.CombinedPIDAction

100% tests passed, 0 tests failed out of 17
```

---

## What Happens If You Remove These Workarounds?

### Removing Manual Eigen Fetch

If you change to standard `FetchContent_MakeAvailable(Eigen3)`:

```bash
$ ctest --test-dir build -N
Test project /home/roger/dev/tank_dynamics/build
  Test   #1: rand
  Test   #2: meta
  ...
  Test #932: eigensolver_generic_15
  
Total Tests: 932
```

CTest will try to run 932 tests (915 from Eigen + 17 ours), but 915 will fail with "executable not found" because Eigen tests aren't built.

### Moving enable_testing() Earlier

If you call `enable_testing()` before fetching dependencies:

- Same 932-test problem (if using standard Eigen fetch)
- Tests/ subdirectory won't be able to register tests if called too late

---

## Alternative Solutions Considered

### Option 1: Use BUILD_TESTING

```cmake
set(BUILD_TESTING OFF)
FetchContent_MakeAvailable(Eigen3)
set(BUILD_TESTING ON)
```

**Doesn't Work:** Eigen ignores `BUILD_TESTING` and registers tests anyway.

### Option 2: Find Eigen as System Package

```cmake
find_package(Eigen3 REQUIRED)
```

**Doesn't Work:** Requires Eigen to be pre-installed on the system, breaking our goal of automatic dependency management.

### Option 3: Git Submodule

Clone Eigen as a submodule and manually add include path.

**Works But:** More manual management, less portable, doesn't solve test pollution if using `add_subdirectory()`.

---

## Future Considerations

### Adding More Header-Only Libraries

If adding another header-only library via FetchContent (e.g., nlohmann/json, spdlog):

1. Check if its CMakeLists.txt registers tests
2. If yes, use the same manual fetch pattern as Eigen
3. If no, can use standard `FetchContent_MakeAvailable()`

### If Eigen Fixes This

If a future Eigen version respects `BUILD_TESTING` properly, we can simplify:

```cmake
set(BUILD_TESTING OFF)
FetchContent_MakeAvailable(Eigen3)
set(BUILD_TESTING ON)
enable_testing()
```

Monitor: https://gitlab.com/libeigen/eigen/-/issues

---

## References

- **Full Solution Details:** `docs/CTEST_FIX.md`
- **Code Review:** `docs/feedback.md` (Issue C2)
- **CMake FetchContent Docs:** https://cmake.org/cmake/help/latest/module/FetchContent.html
- **INTERFACE IMPORTED:** https://cmake.org/cmake/help/latest/command/add_library.html#imported-libraries

---

## Summary

These workarounds are necessary but well-documented. The inline comments in `CMakeLists.txt` explain the reasoning, and this document provides the full context. Future maintainers should read both before modifying the dependency fetch strategy.

**Key Takeaway:** Header-only libraries fetched via FetchContent may need manual handling to prevent test pollution. Always verify test count after adding dependencies.
