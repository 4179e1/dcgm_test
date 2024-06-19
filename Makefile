all: dcgm_dmon dcgm_test

dcgm_dmon: dcgm_dmon.c
	gcc -o dcgm_dmon dcgm_dmon.c -ldcgm

dcgm_test: dcgm_test.c
	gcc -o dcgm_test dcgm_test.c -ldcgm

clean:
	rm -rf dcgm_test __pycache__