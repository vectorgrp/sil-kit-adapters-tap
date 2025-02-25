# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

#!/bin/bash
set -e

echo "[Info] Cleaning..."
rm -rf bin/ lib/ build/ Downloads/

echo "[Info] Building adapter..."
cmake -S. -Bbuild 
cmake --build build --parallel
echo "[Info] Done building adapter."
