#!/bin/bash

set -xe

get_size() {
  stat -c %s "$1"
}

ISO9660="$1"
UDF="$2"

S1="$(get_size "$ISO9660")"
S2="$(get_size "$UDF")"

[ "$S2" -gt "$S1" ] && exit 1

D=$((S1 - S2))

SS=2048

[ $((S1 % SS)) -eq 0 ] || exit 2
[ $((S2 % SS)) -eq 0 ] || exit 2

N1=$((S1 / SS))
N2=$((S2 / SS))
DS=$((D / SS))

C1="$(echo "10*$SS" | bc)"
C2="$(echo "$DS*$SS" | bc)"

set +x

emit_dd_line() {
  echo "$2 <("
  echo 'base64 -d <<EOF | xzcat'
  dd conv=notrunc if="$1" bs=$SS skip="$2" count="$3" | xz | base64 -w 0
  echo 'EOF'
  echo ') \'
}

echo \
'#!/bin/bash
set -xe

[ -z "$CDVD_WRITE_FD_LOCKED" ] && exit 1

base64 -d <<EOF | xzcat | dd conv=notrunc bs='$SS' seek=16 count='$C1' iflag=count_bytes >&$CDVD_WRITE_FD_LOCKED
'"$(dd conv=notrunc if="$1" bs=$SS skip=16 count=$C1 iflag=count_bytes | xz | base64 -w 0)"'
EOF

base64 -d <<EOF | xzcat | dd conv=notrunc bs='$SS' seek='$N2' count='$C2' iflag=count_bytes >&$CDVD_WRITE_FD_LOCKED
'"$(dd conv=notrunc if="$1" bs=$SS skip=$N2 count=$C2 iflag=count_bytes | xz | base64 -w 0)"'
EOF
'
