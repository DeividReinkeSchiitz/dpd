#ifndef __PROBLEM__
#define __PROBLEM__
#include "scip/scip.h"
#include <stdio.h>

typedef struct
{
  char nome[100];
  char sigla[20];
  int areas;
} Disciplina;

typedef struct
{
  Disciplina disciplina;
  int numero;
  int CH;
  enum Periodo
  {
    matutino,
    vespertino,
    noturno
  } periodo;
  int semestre;
  char cursos[25];
} Turma;

typedef struct
{
  int CHmin;
  int CHmax1;
  int CHmax2;
  int ativo;  //bool
  char nome[100];
  int areas;

  //preferencias é um vetor de inteiros, onde cada valor representa o peso dado a turma de indice correspondente.
  //deve ser alocado dinamicamente de acordo com o numero de turmas
  int numeroPreferencias;
  int *preferencias;
  float pesoMedioPreferencias;
} Professor;

typedef struct
{
  int n;         //numero de professores
  int m;         //numero de turmas
  int numAreas;  //numero de areas diferentes no arquivo
  int *C;
  Professor *professores;
  Turma *turmas;

  int area_penalty;
} instanceT;

void freeInstance(instanceT *I);
void createInstance(instanceT **I, int n, int m, int numareas);
void printInstance(instanceT *I);
// load instance from a file
int loadInstance(char *filename, instanceT **I, int area_penalty);
// load instance problem into SCIP
int loadProblem(SCIP *scip, char *probname, instanceT *in);
#endif
