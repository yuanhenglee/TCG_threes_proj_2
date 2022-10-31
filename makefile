all:
	g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o threes threes.cpp
stats:
	./threes --total=1000 --slide="load=weights.bin alpha=0" --save=stats.txt
clean:
	rm threes