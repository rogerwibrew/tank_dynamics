#!/usr/bin/env python3
"""
Simple test script to verify the tank_sim module can be imported and used.

This script tests the basic functionality of the Python bindings created
by the pybind11 build system.
"""

import sys

import tank_sim

print(f"tank_sim version: {tank_sim.get_version()}")
print("Module imported successfully!")
