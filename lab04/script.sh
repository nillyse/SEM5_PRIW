#!/bin/bash
#ka44367
gcc -g lab04.c -o main -pthread -fpermissive -O3 -lm
threads=( 1 4 16 25 49)
image_sizes=( 420 840  2520 )

for thread in "${threads[@]}"
do    
    for image_size in "${image_sizes[@]}"
    do
        ./main ${thread} ${image_size}
    done
done



