weights_size="65536,65536,65536,65536,65536,65536,65536,65536" # 8x4-tuple
./threes --total=100000 --block=1000 --limit=1000 --slide="init=$weights_size save=weights.bin" # need to inherit from weight_agent