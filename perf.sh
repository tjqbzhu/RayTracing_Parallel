#!/usr/bin/env bash
set -e

DYNAMIC="dynamic"
GUIDED="guided"
STATIC="static"
RESOL_X=(800 1440 1920)
RESOL_Y=(600 960 1080)

TARGET_LINE="#pragma omp parallel for collapse(2) schedule"
OUTPUT="../perf_data/omp"

for c in 0 4 8 16 32 64 128 256 512 1024
do
    if [ "$1" != "$DYNAMIC" ] && [ "$1" != "$GUIDED" ] && [ "$1" != "$STATIC" ]; then
	echo "Invalid command argument, must be one of $DYNAMIC, $STATIC or $GUIDED"
	exit 1
    fi

    if [ $c == 0 ]; then
	sed -i "/$TARGET_LINE/c\\${TARGET_LINE}(${1})" ../main_openmp.cpp
	echo "replacing $TARGET_LINE with ${TARGET_LINE}(${1})"
    else
	sed -i "/$TARGET_LINE/c\\${TARGET_LINE}(${1}, ${c})" ../main_openmp.cpp
	echo "replacing $TARGET_LINE with ${TARGET_LINE}(${1}, ${c})"
    fi

    cd ..
    make
    cd build

    for t in 2 4 8 16 32
    do
        echo "chunk is $c, number of threads is $t"

        export OMP_NUM_THREADS=$t
	for i in 0 1 2
	do
		output_file="${OUTPUT}_${1}_${t}_${c}_${RESOL_X[$i]}"
		echo "" > "$output_file"
		echo "cleared $output_file"
		
		for k in 1 2 3
		do
		    perf stat -d ./RayTracing_openmp ${RESOL_X[$i]} ${RESOL_Y[$i]} > out.txt 2>>$output_file;
                done
		
		echo "written to $output_file"

	done
    done
done
