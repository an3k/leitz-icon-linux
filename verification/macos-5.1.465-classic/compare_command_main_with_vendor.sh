#!/bin/bash
# Exact OEM vs candidate main stderr/stdout/exit; no real printer requests.
set -uo pipefail
work="$1"; original="$2"; ppd="$3"
candidate="reconstructions/macos-5.1.465-classic/command2leitz/command2leitz_original_main_candidate"
test -x "$original" && test -x "$candidate" && test -f "$ppd" || exit 1
printf '%s\n' 'ReportState YES' > "$work/entry-unknown.conf"
rm -f "$work/entry-notfound.ppd" "$work/entry-notfound.conf"
errors=0
compare() {
  local label="$1" ppdfile="$2" input_mode="$3"
  local args=(201 testuser Classic 1 'OptimizationLevel=2')
  if [[ "$input_mode" == "file" ]]; then
    args+=("$work/entry-unknown.conf")
  elif [[ "$input_mode" == "missing" ]]; then
    args+=("$work/entry-notfound.conf")
  fi
  PPD="$ppdfile" "$original" "${args[@]}" < "$work/entry-unknown.conf" > "$work/o-$label.out" 2> "$work/o-$label.err"
  local a=$?
  PPD="$ppdfile" "$candidate" "${args[@]}" < "$work/entry-unknown.conf" > "$work/r-$label.out" 2> "$work/r-$label.err"
  local b=$?
  if [[ "$a" == "$b" ]] && cmp -s "$work/o-$label.out" "$work/r-$label.out" && cmp -s "$work/o-$label.err" "$work/r-$label.err"; then
    echo "ORIGINAL_COMMAND_MAIN=$label EXACT_EXIT_STDOUT_STDERR_PARITY=YES"
  else
    echo "ORIGINAL_COMMAND_MAIN=$label EXACT_EXIT_STDOUT_STDERR_PARITY=NO (OEM=$a RECOVERED=$b)"
    diff -u "$work/o-$label.err" "$work/r-$label.err" || true
    errors=1
  fi
}
compare file_unknown "$ppd" file
compare stdin_unknown "$ppd" stdin
compare missing_file "$ppd" missing
compare missing_ppd "$work/entry-notfound.ppd" file
exit "$errors"
