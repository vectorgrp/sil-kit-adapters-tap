# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

#!/bin/bash
set -e

silkit_registry_path=$(find . -name sil-kit-registry -executable -print -quit)


if [ -n "$silkit_registry_path" ]; then
    echo "[Info] Found sil-kit-registry at: $silkit_registry_path, launching it..."
    "$silkit_registry_path" --listen-uri silkit://0.0.0.0:8501 -s
else
  # Use find to search for downloaded SILKit zip file and store the result in zip_file_path
  zip_file_path=$(find . -type f -name "*SilKit*.zip" -print -quit)
  
  # Check if zip_file_path exists
  if [ -n "$zip_file_path" ]; then
    echo "[Info] Found SIL Kit zip file: $zip_file_path, unzipping it..."
    downloads_dir=$(dirname $zip_file_path)
    unzip -q "$zip_file_path" -d $downloads_dir -x "*SilKit-Source/*" "*SilKit-Demos/*" "*SilKit-Documentation/*"
    echo "[Info] Done unzipping SIL Kit to: $downloads_dir"
   else
    echo "[Error] No SIL Kit zip was found. Consider re-building the adapter to fetch required SIL Kit package."
    exit 1
  fi

  silkit_registry_path=$(find $downloads_dir -name sil-kit-registry -executable -print -quit)
  if [ -n "$silkit_registry_path" ]; then
    echo "[Info] Found sil-kit-registry at: $silkit_registry_path, launching it..."
    "$silkit_registry_path" --listen-uri silkit://0.0.0.0:8501
  else
    echo "[Error] No sil-kit-registry executable was found. Consider re-building the adapter to fetch required SIL Kit package."
    exit 1
  fi
fi