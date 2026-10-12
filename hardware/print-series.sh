#!/usr/bin/env bash
# Seven real Leitz Icon hardware checks in one batch. Run inside the CUPS container.
# Use only with a compatible installed 88mm continuous cartridge.
set -Eeuo pipefail
queue=${LEITZ_QUEUE:-Leitz_Icon}
work=$(mktemp -d /tmp/leitz-hw-series.XXXXXXXX)
trap 'rm -rf -- "$work"' EXIT

page() {
  local title=$1 mm=$2 number=$3 h text_y subtitle_y
  case "$mm" in
    35) h=99.2126; text_y=79; subtitle_y=66 ;;
    45) h=127.5591; text_y=107; subtitle_y=94 ;;
    55) h=155.9055; text_y=135; subtitle_y=122 ;;
    *) echo "Unsupported hardware test size: $mm" >&2; return 1 ;;
  esac
  cat <<PS
%%Page: $number $number
<< /PageSize [249.4488 $h] >> setpagedevice
0 setgray
0.6 setlinewidth
newpath 9 9 moveto 231 0 rlineto 0 $(awk "BEGIN {printf \"%.4f\", $h-18}") rlineto -231 0 rlineto closepath stroke
/Helvetica-Bold findfont 12 scalefont setfont
18 $text_y moveto ($title) show
/Helvetica findfont 8 scalefont setfont
18 $subtitle_y moveto (88mm  -  $mm mm  -  graphics and gray levels) show
% Gray ramp: all eight steps should remain visibly distinguishable.
0 1 7 {
  dup 7 div setgray
  25 mul 20 add 44 22 10 rectfill
} for
0 setgray
% Alternating narrow and wide black bars, including fine alignment lines.
0 1 36 {
  /i exch def
  i 3 mod 0 eq {1.5} {0.65} ifelse setlinewidth
  newpath i 5 mul 25 add 20 moveto i 5 mul 25 add 36 lineto stroke
} for
0.4 setlinewidth
newpath 17 39 moveto 216 0 rlineto stroke
showpage
PS
}
make_ps() {
  local filename=$1 mm=$2
  shift 2
  {
    cat <<'HEADER'
%!PS-Adobe-3.0
%%Creator: Leitz Icon Linux hardware test series
%%LanguageLevel: 2
HEADER
    printf '%%%%Pages: %d\n' "$#"
    printf '%%%%EndComments\n'
    local n=1 title
    for title in "$@"; do
      page "$title" "$mm" "$n"
      ((n+=1))
    done
    printf '%%%%EOF\n'
  } > "$filename"
}
submit() {
  local code=$1 protocol=$2 raster=$3 dpi=$4 mm=$5 cutter=$6 copies=$7 file=$8
  printf '\n%s | %s / %s / %s / %s mm / cut=%s / copies=%s\n' \
    "$code" "$protocol" "$raster" "$dpi" "$mm" "$cutter" "$copies"
  lp -d "$queue" -n "$copies" \
    -o "PageSize=Custom.88x${mm}mm" \
    -o "LeitzProtocol=$protocol" \
    -o "LeitzCurrentRaster=$raster" \
    -o "Resolution=$dpi" \
    -o "LeitzCutMode=Page" \
    -o "LabelCutter=$cutter" \
    "$file"
}
echo 'Leitz Icon hardware series: 7 CUPS jobs, 10 printed pages, normally 9 cut pieces.'
echo 'Do not resubmit automatically if a job fails; inspect printer and CUPS first.'

make_ps "$work/a.ps" 55 'A FULL WIDTH RAW'
make_ps "$work/b.ps" 55 'B TRIMMED RAW'
make_ps "$work/c.ps" 45 'C RESOLUTION 300'
make_ps "$work/d.ps" 35 'D COPY TEST'
make_ps "$work/e.ps" 35 'E1 PAGE CUT ON' 'E2 PAGE CUT ON'
make_ps "$work/f.ps" 35 'F1 PAGE CUT OFF' 'F2 PAGE CUT OFF'
make_ps "$work/g.ps" 55 'G NATIVE LINUX'

submit A Current FullWidthRaw 300x600dpi 55 ON 1 "$work/a.ps"
submit B Current TrimmedRaw 300x600dpi 55 ON 1 "$work/b.ps"
submit C Current Optimized 300x300dpi 45 ON 1 "$work/c.ps"
submit D Current Optimized 300x600dpi 35 ON 2 "$work/d.ps"
submit E Current Optimized 300x600dpi 35 ON 1 "$work/e.ps"
submit F Current Optimized 300x600dpi 35 OFF 1 "$work/f.ps"
submit G Native Optimized 300x600dpi 55 ON 1 "$work/g.ps"

echo
echo 'All seven jobs submitted to CUPS. Allow the print queue to finish.'
echo 'Report A/B visual match, C resolution, D two copies, E two cuts, F one cut after two pages, G Native print/cut.'
