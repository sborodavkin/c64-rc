cc65 -Oirs -T render.c
ca65 render.s

cc65 -t c64 -T raycaster.c
ca65 raycaster.s

ld65 -t c64 -o rc raycaster.o render.o c64.lib

rm rc.d64
c1541 -format "rc,1" d64 rc.d64 -attach rc.d64 -write rc rc
