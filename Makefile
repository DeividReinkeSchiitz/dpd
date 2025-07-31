#SCIP_LIB=/usr/local/lib
CFLAGS=-D NO_CONFIG_HEADER -D SCIP_VERSION_MAJOR
$LDFLAGS=-L $(SCIP_LIB)

bin/dpd: bin/cmain.o bin/probdata_dpd.o bin/problem.o bin/heur_problem.o bin/heur_gulosa.o bin/heur_badFeasible.o bin/heur_lns.o bin/utils.o bin/parameters_dpd.o
	gcc $(CFLAGS) $(LDFLAGS) -o bin/dpd bin/cmain.o bin/probdata_dpd.o bin/problem.o bin/heur_problem.o bin/heur_gulosa.o bin/heur_badFeasible.o bin/heur_lns.o bin/utils.o bin/parameters_dpd.o -lm  -lscip

bin/cmain.o: src/cmain.c
	gcc $(CFLAGS) -c -o bin/cmain.o src/cmain.c

bin/probdata_dpd.o: src/probdata_dpd.c src/probdata_dpd.h
	gcc $(CFLAGS) -c -o bin/probdata_dpd.o src/probdata_dpd.c

bin/problem.o: src/problem.c src/problem.h
	gcc $(CFLAGS) -c -o bin/problem.o src/problem.c

bin/heur_gulosa.o: src/heur_gulosa.c src/heur_gulosa.h
	gcc $(CFLAGS) -c -o bin/heur_gulosa.o src/heur_gulosa.c

bin/heur_problem.o: src/heur_problem.c src/heur_problem.h
	gcc $(CFLAGS) -c -o bin/heur_problem.o src/heur_problem.c

bin/heur_badFeasible.o: src/heur_badFeasible.c src/heur_badFeasible.h
	gcc $(CFLAGS) -c -o bin/heur_badFeasible.o src/heur_badFeasible.c

bin/heur_lns.o: src/heur_lns.c src/heur_lns.h
	gcc $(CFLAGS) -c -o bin/heur_lns.o src/heur_lns.c

bin/utils.o: src/utils.c src/utils.h
	gcc $(CFLAGS) -c -o bin/utils.o src/utils.c

bin/parameters_dpd.o: src/parameters_dpd.c src/parameters_dpd.h
	gcc $(CFLAGS) -c -o bin/parameters_dpd.o src/parameters_dpd.c

.PHONY: clean

format:
	clang-format -i src/*.c src/*.h

clean:
	rm -rf bin/*.o bin/dpd output/*

run:
	make && xargs -a ./configs/teste.config ./bin/dpd ./input/entrada_real.csv