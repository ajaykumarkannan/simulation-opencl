# bin/sh

BIN="oclAgents"
BINPATH="bin/darwin/release"

#FFMPEG_OUT="out.gif"
FFMPEG_VIDEO_OUT="result/out.avi"
FFMPEG_PNG_OUT="result/png/out_%d.png"
#FFMPEG_PARAMS="-r 10 -b 1800 -r 25 -vcodec huffyuv -i out_%d.ppm"
FFMPEG_PARAMS="-i out_%d.bmp -r 25 -vcodec huffyuv"


make clean
make
TEMP_PATH=$(PWD)
cd $BINPATH
./$BIN
cd $TEMP_PATH
mv ./$BINPATH/*.ppm ./
rm -f $FFMPEG_VIDEO_OUT
rm -f result/png/*.png
ffmpeg -i out_%d.ppm $FFMPEG_PNG_OUT
ffmpeg -i out_%d.ppm out_%d.bmp
ffmpeg $FFMPEG_PARAMS $FFMPEG_VIDEO_OUT
rm -f *.ppm *.bmp