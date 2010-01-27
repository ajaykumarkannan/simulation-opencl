# bin/sh

BIN="oclAgents"
BINPATH="bin/darwin/release"

FFMPEG_OUT="out.gif"
FFMPEG_PARAMS="-r 10 -b 1800 -i out_%d.ppm"

make clean
make
TEMP_PATH=$(PWD)
cd $BINPATH
./$BIN
cd $TEMP_PATH
mv ./$BINPATH/*.ppm ./
rm -f $FFMPEG_OUT
ffmpeg $FFMPEG_PARAMS $FFMPEG_OUT
rm -f *.ppm