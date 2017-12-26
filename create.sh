#!/bin/bash

set -xe

source "$(dirname "$0")/common.sh"

S=2246688
SS=2048

parse_udf_chunk() {
 [[ "$1" == "start="* ]] || return 0
  local S=${1#start=}
  local S=${S%,}
 [[ "$2" == "blocks="* ]] || return 0
  local B=${2#blocks=}
  local B=${B%,}
 [[ "$3" == "type="* ]] || return 0
  local T=${3#type=}
  local T=${T%,}
 [[ "$T" == "USPACE" ]] && return 0
 echo "S=$S B=$B T=$T" >&2
}

rm -f "$1"
fallocate -l $( echo "$S * $SS" | bc) "$1"
set +x
while read l; do
  parse_udf_chunk $l
done < <(mkudffs --media-type dvdrw -b $SS "$1" $S)
echo -n "#!/bin/bash

$WRITER \"\$1\" "
emit_dd_line "$1" 0 "$S"
