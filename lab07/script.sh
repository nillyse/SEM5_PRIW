#!/bin/bash
#ka44367
gcc -g lab07.c -o lab07 -fopenmp -fpermissive -O3 -lm
n=( 1 2 3 4 5 6 7)
image_sizes=( 420 1200  2520 )

for image_size in "${image_sizes[@]}"
do    
    for thread in "${n[@]}"

    do
        ./lab07 ${thread} ${image_size}
    done
done



