#!/bin/bash
set -e

for f in /home/yuxian/Documents/CS533/RayTracing_Parallel/perf_data_pre/*
do
	echo "processing $f";
	grep "seconds" $f | awk -F ' ' '{printf("%s %s\n", $3, $1)}' > tmp;
	grep "instructions\|L1-dcache-load-misses\|LLC-load-misses" $f | awk -F ' ' '{printf("%s %s\n", $2, $4)}' >> tmp;
	mv tmp $f;
done
