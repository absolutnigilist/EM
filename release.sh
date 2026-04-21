#!/usr/bin/env bash
set -euo pipefail

DEFAULT_PRESET="linux-release"
ACTION="build"
PRESET="$DEFAULT_PRESET"

# Использование:
#   ./build.sh                -> собрать DEFAULT_PRESET
#   ./build.sh linux-debug    -> собрать указанный preset
#   ./build.sh re             -> пересобрать DEFAULT_PRESET с очисткой
#   ./build.sh re linux-debug -> пересобрать указанный preset с очисткой
if [[ $# -ge 1 ]]; then
  if [[ "$1" == "re" ]]; then
    ACTION="re"
    PRESET="${2:-$DEFAULT_PRESET}"
  else
    PRESET="$1"
  fi
fi

BUILD_DIR="out/build/${PRESET}"

echo "\"$(basename "$0") re [preset]\" to rebuild current preset"
echo "\"$(basename "$0") [preset]\" to simple build"

# Полная очистка build-dir выбранного пресета
if [[ "$ACTION" == "re" ]]; then
  rm -rf "$BUILD_DIR"
fi

# Распаковка glog при первом запуске
if [[ ! -d ./glog ]]; then
  echo "glog/ not found, extracting glog.tar.gz..."
  tar -xzf glog.tar.gz
fi

# Конфигурация из CMakePresets.json
cmake --preset "$PRESET"

# Сборка
cmake --build "$BUILD_DIR" -j

# Копируем inlet.in в build-dir, если его там ещё нет
if [[ -f ./inlet.in && ! -f "$BUILD_DIR/inlet.in" ]]; then
  cp ./inlet.in "$BUILD_DIR/inlet.in"
fi

# Копируем локальные glog shared libraries в build-dir, если они есть
if [[ -d ./glog/lib ]]; then
  shopt -s nullglob
  for lib in ./glog/lib/libglog.so*; do
    base="$(basename "$lib")"
    if [[ ! -f "$BUILD_DIR/$base" ]]; then
      cp "$lib" "$BUILD_DIR/$base"
    fi
  done
  shopt -u nullglob
fi