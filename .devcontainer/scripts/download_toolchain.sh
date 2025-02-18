#!/usr/env/bin bash

set -ex

TOOLCHAIN_DOWNLOADED_FILE="toolchain/.downloaded"
if [ -f "${TOOLCHAIN_DOWNLOADED_FILE}" ]; then
    exit 0
fi

TOOLCHAIN_DOWNLOAD_URL="https://github.com/openlumi/BA2-toolchain/releases/download/20201219/ba-toolchain-20201219.tar.bz2"
TOOLCHAIN_DOWNLOAD_PATH="/tmp/ba-toolchain-20201219.tar.bz2"
TOOLCHAIN_UNPACK_TARGET_DIR="${WORKSPACE}/toolchain"

echo "Downloading BA2 toolchain..."
wget -O "${TOOLCHAIN_DOWNLOAD_PATH}" "${TOOLCHAIN_DOWNLOAD_URL}"

echo "Extracting toolchain..."
tar -xjf "${TOOLCHAIN_DOWNLOAD_PATH}" -C "${TOOLCHAIN_UNPACK_TARGET_DIR}" --strip-components=1

touch "${TOOLCHAIN_DOWNLOADED_FILE}"

echo "Cleaning up archive..."
rm "${TOOLCHAIN_DOWNLOAD_PATH}"
