# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

#configure error handling
$ErrorActionPreference = "Stop"
Set-StrictMode -Version 3

#get folder of CANoe4SW Server Edition
$canoe4sw_se_install_dir = $env:CANoe4SWSE_InstallDir64

#create environment
& $canoe4sw_se_install_dir/environment-make.exe "$PSScriptRoot/venvironment.yaml"  -o "$PSScriptRoot"

#compile test unit
& $canoe4sw_se_install_dir/test-unit-make.exe  "$PSScriptRoot/../tests/Testing_TAP_ping_demo.vtestunit.yaml"-e "$PSScriptRoot/Default.venvironment" -o "$PSScriptRoot"

#run tests
& $canoe4sw_se_install_dir/canoe4sw-se.exe "$PSScriptRoot/Default.venvironment" -d "$PSScriptRoot/working-dir" --verbosity-level "2" --test-unit "$PSScriptRoot/Testing_TAP_ping_demo.vtestunit"  --show-progress "tree-element"

