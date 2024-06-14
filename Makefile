all:
	gcc -o dcgm_test dcgm_test.c -ldcgm

clean:
	rm -rf dcgm_test __pycache__