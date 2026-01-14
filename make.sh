echo "compiling"
# cd box2d
# gcc -c *.c
# cd ..

gcc ./src/wiidestroyboxes.c ./src/draw.c ./box2d/*.o -I./box2d/ -lm -lX11 -o wiidestroyboxes -Wall
