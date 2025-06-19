#ifndef __PROBLEM__
#define __PROBLEM__
#include <stdio.h>
#include "scip/scip.h"

typedef struct
{
  char nome[100];
  char sigla[20];
  int areas;
  char myareas[15];
}Disciplina;

typedef struct{
  int codigo_turma;
  int score;
}SCORE;


typedef struct
{
  Disciplina disciplina;
  int codigo;
  int numero;
  int CH;
  enum Periodo {matutino, vespertino, noturno} periodo;
  int semestre;
  char cursos [25];
}Turma;

typedef struct
{
  int CHmin;
  int CHmax1;
  int CHmax2;
  int ativo; //bool
  char nome[100];
  int areas;
  char myareas[15];
  int carga_restante1, carga_restante2;
  int ch_totalatribuida;

  //preferencias é um vetor de inteiros, onde cada valor representa o peso dado a turma de indice correspondente. 
  //deve ser alocado dinamicamente de acordo com o numero de turmas
  int numeroPreferencias;
  int* codigo_turmas;
  int* preferencias;
  float pesoMedioPreferencias;
  int n;  // quantidade de turmas que são da area do prof
  SCORE Score[75];
}Professor;

typedef struct
{
  int n; //numero de professores
  int m; //numero de turmas
  int numAreas; //numero de areas diferentes no arquivo
  int *C;
  Professor *professores;
  Turma *turmas;
  int area_penalty;
}instanceT;


/*
// structure for each item
typedef struct{
  int label; 
  int value;
  int weight;
}itemType;

typedef struct{
   int n;
   int m;
   int *C;
   itemType *item; //< data for each item in 0..n-1
} instanceT;
*/

void freeInstance(instanceT* I);
void createInstance(instanceT** I, int n, int m, int numareas);
void printInstance(instanceT* I);
// load instance from a file
int loadInstance(char* filename, instanceT** I, int area_penalty);
// load instance problem into SCIP
int loadProblem(SCIP* scip, char* probname, instanceT* in);
#endif