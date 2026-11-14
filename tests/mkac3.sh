#!/bin/sh
# Makes ac3.library's test files (tests/fixtures/ac3/) from synthetic
# sound (tones, a sweep and noise; nothing recorded): stereo 192 kbit/s at
# 48 kHz, 5.1 448 kbit/s at 48 kHz, mono 96 kbit/s at 44.1 kHz, and 3/0
# at 32 kHz, two seconds each. Needs ffmpeg with its AC-3 encoder. The
# files are in git; this is how they were made. (c) 2026 Brian King, MIT.
set -e
out=${1:-tests/fixtures/ac3}
mkdir -p "$out"
src='aevalsrc=0.4*sin(2*PI*440*t)+0.2*sin(2*PI*(200+900*t)*t)+0.1*(random(0)-0.5)|0.4*sin(2*PI*660*t)+0.2*sin(2*PI*3000*t)*sin(2*PI*2*t):s=48000:d=2'
ffmpeg -v error -y -f lavfi -i "$src" -c:a ac3 -b:a 192k "$out/stereo-192k.ac3"
ffmpeg -v error -y -f lavfi -i "$src" -af "pan=5.1|FL=c0|FR=c1|FC=0.5*c0+0.5*c1|LFE=0.3*c0|BL=0.6*c1|BR=0.6*c0" \
    -c:a ac3 -b:a 448k "$out/5.1-448k.ac3"
ffmpeg -v error -y -f lavfi -i "$src" -af "pan=mono|c0=0.5*c0+0.5*c1,aresample=44100" -c:a ac3 -b:a 96k \
    "$out/mono-44k.ac3"
ffmpeg -v error -y -f lavfi -i "$src" -af "pan=3.0|FL=c0|FR=c1|FC=0.5*c0+0.5*c1,aresample=32000" -c:a ac3 \
    -b:a 128k "$out/3.0-32k.ac3"
