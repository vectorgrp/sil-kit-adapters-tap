# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

param (
    [string]$SILKitDir
)

# Check if exactly one argument is passed
if (-not $SILKitDir) {
    # If no argument is passed, check if SIL Kit dir has its own environment variable (for the ci-pipeline)
    $SILKitDir = $env:SILKit_InstallDir
    if (-not $SILKitDir) {
        Write-Host "Error: Either provide the path to the SIL Kit directory as an argument or set the `$env:SILKit_InstallDir` environment variable"
        Write-Host "Usage: .\run_all.ps1 <path_to_sil_kit_dir>"
        exit 1
    }
}

$logDir = Join-Path $PSScriptRoot "logs"
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# Create the log directory
if (-not (Test-Path -Path $logDir))
{
    mkdir -p $logDir | Out-Null
}

# Scripts to run the executables and commands in background
$execRegistry = {
   param ($SILKitDir, $ScriptDir, $timestamp, $logDir)
   & $SILKitDir\sil-kit-registry.exe --listen-uri 'silkit://0.0.0.0:8501' -s | Out-File -FilePath "$logDir\sil-kit-registry_$timestamp.out"
}

$execAdapter = {
    param ($ScriptDir, $timestamp, $logDir)
    & $ScriptDir\..\..\..\bin\sil-kit-adapter-tap.exe --log Debug --configuration $scriptDir/../SilKitConfig_Adapter.silkit.yaml | Out-File -FilePath "$logDir\sil-kit-adapter-tap_$timestamp.out"
}

$execDemo = {
    param ($ScriptDir, $timestamp, $logDir)
    & $ScriptDir\..\..\..\bin\sil-kit-demo-ethernet-icmp-echo-device.exe --log Debug | Out-File -FilePath "$logDir\sil-kit-demo-ethernet-icmp-echo-device_$timestamp.out"
}

$execPing = {
    param ($ScriptDir, $timestamp, $logDir)
    & ping 192.168.7.35 -S 192.168.7.2 -n 100 | Out-File -FilePath "$logDir\ping-command_$timestamp.out"
}

Start-Job -ScriptBlock $execRegistry -ArgumentList $SILKitDir, $PSScriptRoot, $timestamp, $logDir -Name SILKitRegistry

Start-Sleep -Seconds 1

Start-Job -ScriptBlock $execAdapter -ArgumentList $PSScriptRoot, $timestamp, $logDir -Name TapAdapter

Start-Job -ScriptBlock $execDemo -ArgumentList $PSScriptRoot, $timestamp, $logDir -Name Demo

Start-Job -ScriptBlock $execPing -ArgumentList $PSScriptRoot, $timestamp, $logDir -Name PingCmd

# Execute CANoe4SW_SE tests
& $PSScriptRoot\run.ps1 | Out-File -FilePath "$logDir\run_canoe4sw_se_$timestamp.out"
$result = $LASTEXITCODE

# Stop all the jobs
Stop-Job -Name PingCmd, TapAdapter, Demo, SILKitRegistry

if($result -eq 0)
{
    Write-Output "Tests passed"
    exit 0
}
else
{
    Write-Output "Tests failed"
    exit 1
}
