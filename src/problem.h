#ifndef __PROBLEM__
#define __PROBLEM__
#include <stdio.h>
#include "scip/scip.h"

typedef struct
{
  char nome[100];
  char sigla[20];
  int areas;
  char *myareas;
}Disciplina;

typedef struct{
  int codigo_turma;
  int score;
}Preferencia;

typedef struct
{
  Disciplina disciplina;
  int codigo;
  int numero;
  int CH;
  enum Periodo {matutino, vespertino, noturno} periodo;
  int semestre;
  char cursos [25];
  //int professores[61]; // profs que podem ministrar a turma em questao
  int n;  // grau do vertice da turma (quantos profs a turma pode ministrar)
  int covered;  // se 1 turma esta coberta, se 0 turma nao esta coberta
}Turma;

typedef struct
{
  int CHmin;
  int CHmax1;
  int CHmax2;
  int ativo; //bool
  char nome[50];
  int areas;
  char *myareas;
  int current_CH1, current_CH2, num_assigned;

  //preferencias é um vetor de inteiros, onde cada valor representa o peso dado a turma de indice correspondente. 
  //deve ser alocado dinamicamente de acordo com o numero de turmas
  int numeroPreferencias;
  int* codigo_turmas;
  int* preferencias;  // vetor esparço, que contem apenas as turmas que o prof escolheu previamente (contem o peso que ele atribui)
  float pesoMedioPreferencias;

  int codigo;  // campo para eu nao perder a referencia depois que eu ordenar os profs pelo grau do vertice
  int n;  // grau do vertice do prof (quantas disciplinas ele pode ministrar)
  int m;  // quantidade de turmas alocadas para o prof
  int turmasAlocadas[15];  // considerando que um prof pode ter no max 10 turmas atribuidas
  Preferencia pref[128];  // contem as turmas que o prof escolheu previamente + as turmas da sua area
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
// int check_area(const char *a, const char *b, int size);
// int check_preference(int *preferencias, int codigo_turma, int m);
#endif