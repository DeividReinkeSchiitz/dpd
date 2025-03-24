#SCIP_LIB=/usr/local/lib
CFLAGS=-D NO_CONFIG_HEADER -D SCIP_VERSION_MAJOR
$LDFLAGS=-L $(SCIP_LIB)


bin/dpd: bin/cmain.o bin/probdata_dpd.o bin/problem.o bin/heur_myrounding.o bin/heur_problem.o bin/heur_gulosa.o
	gcc $(CFLAGS) $(LDFLAGS) -o bin/dpd bin/cmain.o bin/probdata_dpd.o bin/problem.o bin/heur_myrounding.o bin/heur_problem.o bin/heur_gulosa.o -lscip 

bin/cmain.o: src/cmain.c
	gcc $(CFLAGS) -c -o bin/cmain.o src/cmain.c

bin/probdata_dpd.o: src/probdata_dpd.c src/probdata_dpd.h
	gcc $(CFLAGS) -c -o bin/probdata_dpd.o src/probdata_dpd.c

bin/problem.o: src/problem.c src/problem.h
	gcc $(CFLAGS) -c -o bin/problem.o src/problem.c

bin/heur_myrounding.o: src/heur_myrounding.c src/heur_myrounding.h
	gcc $(CFLAGS) -c -o bin/heur_myrounding.o src/heur_myrounding.c

bin/heur_gulosa.o: src/heur_gulosa.c src/heur_gulosa.h
	gcc $(CFLAGS) -c -o bin/heur_gulosa.o src/heur_gulosa.c

bin/heur_problem.o: src/heur_problem.c src/heur_problem.h
	gcc $(CFLAGS) -c -o bin/heur_problem.o src/heur_problem.c

.PHONY: clean

format:
	clang-format -i src/*.c src/*.h

clean:
	rm -f bin/*.o bin/dpd

