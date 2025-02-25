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

# Scripts to run the executables and commands in background
$execRegistry = {
   param ($SILKitDir, $ScriptDir)
   & $SILKitDir\sil-kit-registry.exe --listen-uri 'silkit://0.0.0.0:8501' -s | Out-File -FilePath $ScriptDir\sil-kit-registry.out
}

$execAdapter = {
    param ($ScriptDir)
    & $ScriptDir\..\..\..\bin\sil-kit-adapter-tap.exe --log Debug --configuration $scriptDir/../SilKitConfig_Adapter.silkit.yaml | Out-File -FilePath $ScriptDir\sil-kit-adapter-tap.out
}

$execDemo = {
    param ($ScriptDir)
    & $ScriptDir\..\..\..\bin\sil-kit-demo-ethernet-icmp-echo-device.exe --log Debug | Out-File -FilePath $ScriptDir\sil-kit-demo-ethernet-icmp-echo-device.out
}

$execPing = {
    param ($ScriptDir)
    & ping 192.168.7.35 -S 192.168.7.2 -n 100 | Out-File -FilePath $ScriptDir\ping-command.out
}

Start-Job -ScriptBlock $execRegistry -ArgumentList $SILKitDir, $PSScriptRoot -Name SILKitRegistry

Start-Sleep -Seconds 1

Start-Job -ScriptBlock $execAdapter -ArgumentList $PSScriptRoot -Name TapAdapter

Start-Job -ScriptBlock $execDemo -ArgumentList $PSScriptRoot -Name Demo

Start-Job -ScriptBlock $execPing -ArgumentList $PSScriptRoot -Name PingCmd

# Get the last line telling the overall test verdict (passed/failed)
$scriptResult = & $PSScriptRoot\run.ps1 | Select-Object -Last 1

$isPassed = select-string -pattern "passed" -InputObject $scriptResult

# Stop all the jobs
Stop-Job -Name PingCmd, TapAdapter, Demo, SILKitRegistry

if($isPassed)
{
    Write-Output "Tests passed"
    exit 0
}
else
{
    Write-Output "Tests failed"
    exit 1
}
