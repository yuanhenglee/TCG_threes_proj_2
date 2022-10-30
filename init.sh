weights_size="65536,65536,65536,65536,65536,65536,65536,65536" # 8x4-tuple
alpha=0.025
./threes --total=0 --slide="init=$weights_size save=weights.bin" # generate a clean network