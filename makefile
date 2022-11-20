main: main.cpp
	g++ -o main main.cpp

dct: main_dct_pc.cpp dct_pc.py
	g++ -o main_dct_pc dct main_dct_pc.cpp

dct_c: main_dct_c.cpp
	g++ -o main_dct_c main_dct_c.cpp

# # Only test working on Ubuntu 22.04 with python 3.8, please change '***' suitable path
# dct_p: main_dct.cpp my_dct.py
# 	export CPLUS_INCLUDE_PATH=*** # where Python.h locate
# 	g++ main_dct.cpp $(python3.8-config --ldflags --embed) $(python3.8-config --cflags --embed) -o main_dct
# 	export LD_LIBRARY_PATH=***/miniconda3/envs/***/lib/ # where libpython3.8.so locate

clean:
	rm main main_dct main_dct_c main_dct_pc
	rm -r main.dSYM main_dct.dSYM main_dct_c.dSYM main_dct_pc.dSYM
	rm -r process_folder.nosync
	mkdir process_folder.nosync