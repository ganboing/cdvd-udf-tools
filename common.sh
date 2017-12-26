#!/bin/bash

WRITER=./cdvd-writer.exe

emit_dd_line() {
  echo "$2 <("
  echo 'base64 -d <<EOF | xzcat'
  dd conv=notrunc if="$1" bs=$SS skip="$2" count="$3" | xz | base64
  echo 'EOF'
  echo ') \'
}

