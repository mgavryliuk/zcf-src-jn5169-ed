#!/usr/bin/env bash
set -eo pipefail

install_vscode_extensions() {
  extensions=(
    "ms-vscode.cmake-tools"
    "ms-python.python"
    "ms-vscode.cpptools"
    "ms-vscode.cpptools-extension-pack"
    "ms-vscode.cpptools-themes"
  )
  if command -v cursor > /dev/null 2>&1; then
    cursor $(printf -- '--install-extension %s ' "${extensions[@]}")
  elif command -v code > /dev/null 2>&1; then
    code $(printf -- '--install-extension %s ' "${extensions[@]}")
  fi
}

main() {
  install_vscode_extensions
}

main
