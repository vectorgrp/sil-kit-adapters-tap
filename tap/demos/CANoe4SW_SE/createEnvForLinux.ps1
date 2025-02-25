# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

param (
    [string]$remote_ip
)

#configure error handling
$ErrorActionPreference = "Stop"
Set-StrictMode -Version 3

#get folder of CANoe4SW Server Edition
$canoe4sw_se_install_dir = $env:CANoe4SWSE_InstallDir64

#create environment
& $canoe4sw_se_install_dir/environment-make.exe "$PSScriptRoot/venvironment.yaml"  -o "$PSScriptRoot" -A "Linux64"

#compile test unit
& $canoe4sw_se_install_dir/test-unit-make.exe "$PSScriptRoot/../tests/Testing_TAP_ping_demo.vtestunit.yaml" -e "$PSScriptRoot/Default.venvironment" -o "$PSScriptRoot"

$ipv4Pattern = '^(?:(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])(\.(?!$)|$)){4}$' # regex that matches an IPv4 addresses

$ipAddressNotEmpty = -not [string]::IsNullOrEmpty($remote_ip)

if($ipAddressNotEmpty)
{
	if($remote_ip -match $ipv4Pattern)
	{
		# remote machine and paths configuration
		$remote_username="vector"
		$remote_SKA_base_path="/home/$remote_username/vfs/sil-kit-adapters-tap"
		$artifacts_subdirectory = "tap/demos/CANoe4SW_SE"
		$remote_full_path = Join-Path -Path $remote_SKA_base_path -ChildPath $artifacts_subdirectory

		#copy artifacts to VM
		Write-Host "Copying artifacts to [${remote_username}@${remote_ip}:${remote_full_path}]..."
		scp -r $PSScriptRoot/Default.venvironment $PSScriptRoot/*.vtestunit "${remote_username}@${remote_ip}:${remote_full_path}"
		Write-Host "Done."
	}
	else {
		Write-Host "[Warning] Passed IP address does not have a right format, nothing will be copied to remote."
	}
}
else {
	Write-Host "[Info] No IP address has been passed, nothing will be copied to remote."
}
