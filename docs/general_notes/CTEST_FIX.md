# CTest Discovery Fix

**Date:** 2026-02-02  
**Issue:** Critical Issue C2 from code review - CTest not discovering tests

## Problem

When running `ctest --test-dir build`, the system reported:
```
Test project /home/roger/dev/tank_dynamics/build
No tests were found!!!
```

However, running the test executable directly worked fine:
```bash
$ ./build/tests/test_tank_sim_core
[==========] Running 17 tests from 2 test suites.
[  PASSED  ] 17 tests.
```

Additionally, CTest was attempting to run 932 Eigen library tests which don't exist in our build.

## Root Cause

The issue had two parts:

1. **`enable_testing()` called too late**: It was being called AFTER `FetchContent_MakeAvailable()`, which meant Eigen's CMakeLists.txt registered its tests before our project's testing infrastructure was set up.

2. **Eigen test pollution**: When Eigen was fetched via `FetchContent_MakeAvailable()`, its `CMakeLists.txt` was processed with `add_subdirectory()`, which registered all of Eigen's 900+ tests in our test suite even though the test executables weren't built.

## Solution

Modified `CMakeLists.txt` to:

1. **Manually fetch Eigen without processing its subdirectory**:
   ```cmake
   # Fetch Eigen manually without adding its subdirectory to prevent test pollution
   FetchContent_GetProperties(Eigen3)
   if(NOT eigen3_POPULATED)
       FetchContent_Populate(Eigen3)
       # Do NOT call add_subdirectory - just make the target available
       add_library(Eigen3::Eigen INTERFACE IMPORTED GLOBAL)
       target_include_directories(Eigen3::Eigen INTERFACE ${eigen3_SOURCE_DIR})
   endif()
   ```

2. **Call `enable_testing()` at the right time**:
   - After dependencies are fetched
   - Before our `tests/` subdirectory is added
   
   This prevents Eigen from registering tests while allowing our tests to be discovered.

## Result

```bash
$ ctest --test-dir build
Test project /home/roger/dev/tank_dynamics/build
      Start  1: TankModelTest.SteadyStateZeroDerivative
 1/17 Test  #1: TankModelTest.SteadyStateZeroDerivative ..................   Passed    0.00 sec
      Start  2: TankModelTest.PositiveDerivativeWhenInletExceedsOutlet
 2/17 Test  #2: TankModelTest.PositiveDerivativeWhenInletExceedsOutlet ...   Passed    0.00 sec
...
100% tests passed, 0 tests failed out of 17

Total Test time (real) =   0.03 sec
```

## Benefits

- CTest now properly discovers and runs only our 17 tests
- No Eigen test pollution
- Clean test output
- Proper CI/CD integration capability
- Developer workflow improved

## Files Modified

- `CMakeLists.txt`: Changed Eigen fetch strategy and `enable_testing()` placement
- `tests/CMakeLists.txt`: Removed duplicate `enable_testing()` call

## Testing

Verified with:
```bash
rm -rf build
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

All 17 tests pass consistently.

## Lessons Learned

- `FetchContent_MakeAvailable()` processes the entire CMakeLists.txt of dependencies
- Header-only libraries like Eigen don't need full `add_subdirectory()` processing
- Manual `FetchContent_Populate()` + `INTERFACE IMPORTED` targets provide finer control
- Test discovery timing is critical in CMake projects with FetchContent

## Future Considerations

If we add more header-only dependencies via FetchContent in the future, use the same manual fetch pattern to prevent test pollution.
