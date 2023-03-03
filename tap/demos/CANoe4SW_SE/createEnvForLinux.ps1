#configure error handling
$ErrorActionPreference = "Stop"
Set-StrictMode -Version 3

#get folder of CANoe4SW Server Edition
$canoe4sw_se_install_dir = $env:CANoe4SWSE_InstallDir64

#create environment
& $canoe4sw_se_install_dir/environment-make.exe "$PSScriptRoot/venvironment.yaml"  -o "$PSScriptRoot" -A "Linux64"

#compile test unit
& $canoe4sw_se_install_dir/test-unit-make.exe "$PSScriptRoot/../vTestStudio/TestUnit" -e "$PSScriptRoot/Default.venvironment" -o "$PSScriptRoot"

