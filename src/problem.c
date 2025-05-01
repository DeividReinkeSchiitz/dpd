/**@file   problem.c
 * @brief  This file contains routines specific for the problem and the functions loadInstance(), freeInstance, 
 * printInstance, and loadProblem must be implemented 
 *
 **/ 
#include<stdio.h>
#include<math.h>
#include "scip/scip.h"
#include "probdata_dpd.h"

void freeInstance(instanceT* I) 
{
  if(I){
    for(int i=0; i<I->n; i++){
      free(I->professores[i].preferencias);
      free(I->professores[i].codigo_turmas);

    }
    free(I->professores);
    free(I->turmas);
    free(I->C);
    free(I);
    I = NULL;
  }
}
void createInstance(instanceT** I, int n, int m, int numAreas)
{
  *I = (instanceT*) malloc(sizeof(instanceT));
  (*I)->professores = (Professor*) malloc(sizeof(Professor)*n);
  (*I)->turmas = (Turma*) malloc(sizeof(Turma)*m);
  (*I)->C = (int*) malloc(sizeof(int)*(m+2*n));

  //aloca as preferencias dos professores
  //int pref;
  for(int i=0; i<n; i++){
    (*I)->professores[i].preferencias = (int*) calloc(m, sizeof(int));
    //pref = (*I)->professores[i].numeroPreferencias;  // pegando a quant de turmas que o prof i tem interesse
    //printf("\n\nTESTE: %d\n\n", pref);
   (*I)->professores[i].codigo_turmas = (int*) calloc(m, sizeof(int));
   // aparentemente aqui eu nao li ainda as info de cada prof ent n posso alocar um vetor usando a quant de preferencia que ele possui
  }

  (*I)->n = n;
  (*I)->m = m;
  (*I)->numAreas = numAreas;
}

void printInstance(instanceT* I){
	printf("%d;%d;%d\n", I->m, I->n, I->numAreas);
	printf("codigo;semestre;numero;nome;curso;CH;area\n");
	for(int i=0; i<I->m; i++){
		printf("%d;%d;T%d;%s;%s;%d;%d\n", i+1, I->turmas[i].semestre, I->turmas[i].numero, I->turmas[i].disciplina.nome, I->turmas[i].cursos, I->turmas[i].CH, I->turmas[i].disciplina.areas);
	}	
	printf("\nnome;CHmin;CHmax1;CHmax2;preferencias;areas\n");
	for(int i=0; i<I->n; i++){
		printf("%s;%d;%d;%d;%d;%d\n", I->professores[i].nome, I->professores[i].CHmin, I->professores[i].CHmax1, I->professores[i].CHmax2, I->professores[i].numeroPreferencias, I->professores[i].areas);
		for(int j=0; j< I->m; j++){
      if(I->professores[i].preferencias[j]!=0){
			  printf("%d;%d\n", j+1, I->professores[i].preferencias[j]);
      }
		}
	}
}

int loadInstance(char* filename, instanceT** I, int area_penalty)
{
  FILE* f;
  int n, m, numareas;
  f = fopen(filename, "r");
  if(!f){
    printf("\nProblem to open file %s\n", filename);
    return 0;
  }
  char linha [400];
  fgets(linha, sizeof(linha), f);
  sscanf(linha, "%d;%d;%d", &m, &n, &numareas);
  createInstance(I, n, m, numareas);
  (*I)->area_penalty = area_penalty;
  
  //lendo cabecalho
  fgets(linha, sizeof(linha), f);

  for(int i=0; i<m; i++){
  	fgets(linha, sizeof(linha), f);
  	sscanf(linha, "%*d;%d;T%d;%99[^;];%99[^;];%d;%d", &((*I)->turmas[i].semestre), &((*I)->turmas[i].numero), (*I)->turmas[i].disciplina.nome, (*I)->turmas[i].cursos, &(*I)->turmas[i].CH, &(*I)->turmas[i].disciplina.areas);
  }
  
  fgets(linha, sizeof(linha), f);//linha em branco
  fgets(linha, sizeof(linha), f);//cabecalho professores
  
  //lendo professores
  for(int i=0; i<n; i++){
  	fgets(linha, sizeof(linha), f);
  	int p;
  	sscanf(linha, "%99[^;];%d;%d;%d;%d;%d", (*I)->professores[i].nome, &((*I)->professores[i].CHmin), &((*I)->professores[i].CHmax1), &((*I)->professores[i].CHmax2), &p, &(*I)->professores[i].areas);
	(*I)->professores[i].numeroPreferencias=p;
    float sum = 0;
  	for(int j=0; j<p; j++){
  		fgets(linha, sizeof(linha), f);
  		int indexTurma, peso;
  		sscanf(linha, "%d;%d", &indexTurma, &peso);
  		(*I)->professores[i].preferencias[indexTurma-1]=peso;
      (*I)->professores[i].codigo_turmas[j]=indexTurma;
        sum += peso;
	}
    (*I)->professores[i].pesoMedioPreferencias = sum / p;
  }
  return 1;

}

// load instance problem into SCIP
int loadProblem(SCIP* scip, char* probname, instanceT* I)
{
  SCIP_RETCODE ret_code;


  ret_code = SCIPprobdataCreate(scip, probname, I);
  if(ret_code!=SCIP_OKAY)
    return 0;
  return 1;
}
