#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

script_root=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "[info] Creating environment"
$canoe4sw_se_install_dir/environment-make $script_root/venvironment.yaml -o "$script_root" -A "Linux64"
exit_status=$?
if [ $exit_status -ne 0 ]; then
    exit $exit_status
fi

echo "[info] Compiling tests"
$canoe4sw_se_install_dir/test-unit-make $script_root/../tests/Testing_TAP_ping_demo.vtestunit.yaml -e $script_root/Default.venvironment -o "$script_root"
