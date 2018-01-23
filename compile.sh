cl65 -l -T -t c64 rc.c

rm rc.d64
c1541 -format "rc,1" d64 rc.d64 -attach rc.d64 -write rc rc
