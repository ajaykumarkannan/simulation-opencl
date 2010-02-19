# bin/sh

BIN="oclAgents"
BINPATH="bin/darwin/release"

#FFMPEG_OUT="out.gif"
FFMPEG_OUT="out.avi"
#FFMPEG_PARAMS="-r 10 -b 1800 -r 25 -vcodec huffyuv -i out_%d.ppm"
FFMPEG_PARAMS="-i out_%d.bmp -r 10 -vcodec huffyuv"

make clean
make
TEMP_PATH=$(PWD)
cd $BINPATH
./$BIN
cd $TEMP_PATH
mv ./$BINPATH/*.ppm ./
rm -f $FFMPEG_OUT
ffmpeg -i out_%d.ppm out_%d.png
ffmpeg $FFMPEG_PARAMS $FFMPEG_OUT
rm -f *.ppm