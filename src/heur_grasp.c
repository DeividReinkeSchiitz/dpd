/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2016 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   heur_grasp.c
 * @brief  grasp primal heuristic
 * @author Edna Hoshino (based on template provided by Tobias Achterberg)
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>
#include <time.h>
#include <string.h>

#include "probdata_dpd.h"
#include "parameters_dpd.h"
#include "heur_grasp.h"
#include "heur_problem.h"
#include "problem.h"

//#define DEBUG_GRASP 1
/* configuracao da heuristica */
#define HEUR_NAME             "grasp"
#define HEUR_DESC             "grasp primal heuristic template"
#define HEUR_DISPCHAR         'g'
#define HEUR_PRIORITY         2 /**< heuristics of high priorities are called first */
#define HEUR_FREQ             1 /**< heuristic call frequency. 1 = in all levels of the B&B tree */
#define HEUR_FREQOFS          0 /**< starts of level 0 (root node) */
#define HEUR_MAXDEPTH         -1 /**< maximal level to be called. -1 = no limit */
#define HEUR_TIMING           SCIP_HEURTIMING_AFTERNODE /**< when the heuristic should be called? SCIP_HEURTIMING_DURINGLPLOOP or SCIP_HEURTIMING_AFTERNODE */
#define HEUR_USESSUBSCIP      FALSE  /**< does the heuristic use a secondary SCIP instance? */

#ifdef DEBUG
   #define PRINTF(...) printf(__VA_ARGS__)
#else
   #define PRINTF(...) 
#endif

/*
 * Data structures
 */

/* TODO: fill in the necessary primal heuristic data */

/** primal heuristic data */
/*struct SCIP_HeurData
{
};
*/

/*
 * Local methods
 */

/* put your local methods here, and declare them static */

/*
 * Callback methods of primal heuristic
 */

/* TODO: Implement all necessary primal heuristic methods. The methods with an #if 0 ... #else #define ... are optional */

/** copy method for primal heuristic plugins (called when SCIP copies plugins) */
static
SCIP_DECL_HEURCOPY(heurCopyGrasp)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static
SCIP_DECL_HEURFREE(heurFreeGrasp)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}


/** initialization method of primal heuristic (called after problem was transformed) */
static
SCIP_DECL_HEURINIT(heurInitGrasp)
{  /*lint --e{715}*/


   return SCIP_OKAY;
}


/** deinitialization method of primal heuristic (called before transformed problem is freed) */
static
SCIP_DECL_HEUREXIT(heurExitGrasp)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}


/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static
SCIP_DECL_HEURINITSOL(heurInitsolGrasp)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}


/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static
SCIP_DECL_HEUREXITSOL(heurExitsolGrasp)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}

// void maxmin(Candidatos *C, int *max, int *min, int n) {
//    if (n == 0) return; // vetor vazio, nada a fazer

//    // caso base: apenas um candidato
//    if (n == 1) {
//        *min = C[0].peso_atribuido;
//        *max = C[0].peso_atribuido;
//        return;
//    }

//    // inicializa min e max com os dois primeiros candidatos
//    if (C[0].peso_atribuido < C[1].peso_atribuido) {
//        *min = C[0].peso_atribuido;
//        *max = C[1].peso_atribuido;
//    } else {
//        *min = C[1].peso_atribuido;
//        *max = C[0].peso_atribuido;
//    }

//    // percorre em pares a partir do terceiro elemento
//    for (int i = 2; i < n - 1; i += 2) {
//        int menor, maior;
//        if (C[i].peso_atribuido < C[i + 1].peso_atribuido) {
//            menor = C[i].peso_atribuido;
//            maior = C[i + 1].peso_atribuido;
//        } else {
//            menor = C[i + 1].peso_atribuido;
//            maior = C[i].peso_atribuido;
//        }

//        if (menor < *min) *min = menor;
//        if (maior > *max) *max = maior;
//    }

//    // se o numero de candidatos for impar, verifica o ultimo
//    if (n % 2 != 0) {
//        if (C[n - 1].peso_atribuido < *min) *min = C[n - 1].peso_atribuido;
//        if (C[n - 1].peso_atribuido > *max) *max = C[n - 1].peso_atribuido;
//    }
// }


// // para estar na lista de candidatos do prof, a turma nao pode estar coberta
// int cria_candidatos(Candidatos *candidatos, int *codigo_turmas, int *pesos_atribuidos, int n, int *covered){
//    //printf("\nCRIANDO A LISTA DE CANDIDATOS:\n");
//    int posicao, j=0;
//    for(int i = 0; i < n; i++){
//       posicao = codigo_turmas[i] -1;
//       if(covered[posicao] == 0){
//          //printf("\nTURMA NAO COBERTA: %d\n", covered[posicao]);
//          candidatos[j].codigo_turma = codigo_turmas[i];
//          candidatos[j].peso_atribuido = pesos_atribuidos[posicao];
//          j++;
//       }else{
//          //printf("\nTURMA COBERTA: %d\n", covered[posicao]);
//       }
     
//    }

//    return j;
  
// }

// void cria_RCL(Candidatos *candidatos, Candidatos *RCL, int alpha, int max, int min, int n, int *n_RCL){
//    for(int i = 0; i < n; i++){
//       if(candidatos[i].peso_atribuido >= min * alpha*(max - min)){
//          RCL[*n_RCL].codigo_turma = candidatos[i].codigo_turma;
//          RCL[*n_RCL].peso_atribuido = candidatos[i].peso_atribuido;
//          *n_RCL += 1;
//       }
//    }
// }

// Candidatos escolherAleatorio(Candidatos *vetor, int n) {
//    if (n == 0) {
//        Candidatos vazio = {-1, -1};
//        return vazio;
//    }

//    int indice = rand() % n;
//    return vetor[indice];
// }

// void atualiza_candidatos(Candidatos *candidatos, Candidatos removido, int n){
//    if(n == 0){
//       return;
//    }
//    for(int i = 0; i < n; i++){
//       if(candidatos[i].codigo_turma == removido.codigo_turma){
//          candidatos[i] = candidatos[n-1];
//       }
//    }
// }


// funcao para verificar se a area de uma turma coincide com a area do prof
int verifica_area(const char a[15], const char b[15]) {
    for (int i = 0; i < 15; i++) {
        if (a[i] == '1' && b[i] == '1') {
            return 1;
        }
    }
    return 0;
}

int verifica_areas(int areaProfessor, int areaTurma, int numAreas){
   for (int i = 0; i < numAreas; i++)
   {
     if (areaProfessor % 10 == areaTurma % 10)
     {
       if (areaProfessor % 10 == 1)
       {
         return 1;
       }
     }
     areaProfessor = areaProfessor / 10;
     areaTurma     = areaTurma / 10;
   }
   return 2;
}

float numero_aleatorio(int a, int b) {
    // garante que a é menor do que b
    if (a > b) {
        int temp = a;
        a = b;
        b = temp;
    }

    // inicializa a semente do gerador de numeros aleatorios
    static int inicializado = 0;
    if (!inicializado) {
        srand(time(NULL));
        inicializado = 1;
    }
    // gera um numero aleatorio entre 0 e 1
    float r = (float)rand() / RAND_MAX;
    
    // escala para o intervalo estabelecido
    return a + r * (b - a);
}

int calculaScore(int peso_preferencia, int alfa){
   return peso_preferencia + alfa * 1;
}


/**
 * @brief Core of the grasp heuristic: it builds one solution for the problem by grasp procedure.
 *
 * @param scip problem
 * @param sol pointer to the solution structure where the solution wil be saved
 * @param heur pointer to the grasp heuristic handle (to contabilize statistics)
 * @return int 1 if solutions is found, 0 otherwise.
 */
int grasp(SCIP* scip, SCIP_SOL** sol, SCIP_HEUR* heur)
{
   int found, infeasible, nInSolution;
   unsigned int stored;
   int nvars;
   int *covered, n, m, nCovered;
   SCIP_VAR *var, **solution, **varlist;
   //  SCIP* scip_cp;
   SCIP_Real valor, bestUb;
   SCIP_PROBDATA* probdata;
   int i, posicao_turma, max_i = 1, max, min, carga_s1, carga_s2, n_RCL, semestre;
   instanceT* I;
   Professor *professores;
   // Auxiliar* profs_auxiliar;

   found = 0;
   infeasible = 0;
   
#ifdef DEBUG_GRASP
   printf("\n============== New grasp heur at node: %lld\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
#endif

   /* recover the problem data*/
   probdata=SCIPgetProbData(scip);
   assert(probdata != NULL);

   nvars = SCIPprobdataGetNVars(probdata);
   varlist = SCIPprobdataGetVars(probdata);
   I = SCIPprobdataGetInstance(probdata);
   n = I->n;  // quant de professores
   m = I->m;  // quant de turmas
   
   professores = (Professor*) malloc(sizeof(Professor) * n);
   solution = (SCIP_VAR**) malloc(sizeof(SCIP_VAR*)* (n*m));
   covered = (int*) calloc(m,sizeof(int));  // o vetor de cobertos vai representar as turmas. inicialmente todas as posicoes estao com 0 (nenhuma turma foi coberta)
   nInSolution = 0;
   nCovered = 0;

   // first, select all variables already fixed in 1.0
   for(i=0;i<nvars;i++){
      var = varlist[i];
      if(SCIPvarGetLbLocal(var) > 1.0 - EPSILON){ // var >= 1.0
        solution[nInSolution++]=var;        
        covered[i%m] = 1;
        nCovered++;
#ifdef DEBUG_GRASP
        printf("\nSelected fixed var= %s. TotalItems=%d infeasible=%d", SCIPvarGetName(var), nInSolution, infeasible);
#endif
      }
      else{ // discard items fixed in 0.0
        if(SCIPvarGetUbLocal(var) < EPSILON){ // var fixed in 0.0
          //covered[i] = 1;
          //nCovered++;
        }
      }
   }
   
   // copiando as info dos prof para uma var auxiliar, porque no futuro eu irei alterar esse dados
   professores = I->professores;
   int flag = 0, k, score, codigo_turma, peso_atribuido, alfa, aux;
   
   for(int i = 0; i < n; i++){
      printf("PROFESSOR: %s\n", I->professores[i].nome);
      //printf("n DO PROF: %d\n", I->professores[i].Score->n);
      
      for(int j = 0; j < m; j++){
        // printf("TURMA: %d\n", I->turmas[j].codigo);
         // verifico se a turma j esta nas escolhida pelo prof i
         for(k = 0; k < I->professores[i].numeroPreferencias; k++){

            if(I->professores[i].codigo_turmas[k] == I->turmas[j].codigo){
               codigo_turma = I->turmas[j].codigo;
               peso_atribuido = I->professores[i].preferencias[codigo_turma-1];
               printf("CODIGO TURMA: %d e PESO ATRIBUIDO: %d\n", codigo_turma, peso_atribuido);
               flag = 1;
               break;
            }
         }

         if(flag == 1){
            // calcula o score passando o peso da preferencia
            aux = I->professores[i].n;
            alfa = numero_aleatorio(1, 3);
            score = calculaScore(peso_atribuido, alfa);
            I->professores[i].Score[aux].score = score;
            I->professores[i].n+= 1;


         }else{
            // nao foi escolhida previamente. preciso verificar se é da área do prof

         }


         // o importante aqui é: se a turma é da area do prof ela tem que entrar no vetor de SCORE

      }
   }

   if(!infeasible){
      /* create SCIP solution structure sol */
      SCIP_CALL( SCIPcreateSol(scip, sol, heur) );
      // save found solution in sol
      for(i=0;i<nInSolution;i++){
        var = solution[i];
        SCIP_CALL( SCIPsetSolVal(scip, *sol, var, 1.0) );
      }
      //valor = custo;//createSolution(scip, *sol, solution, nInSolution, &infeasible, covered);
      bestUb = SCIPgetPrimalbound(scip);
#ifdef DEBUG_GRASP
      printf("\nFound solution...\n");
      //      SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
      printf("\ninfeasible=%d value = %lf > bestUb = %lf? %d\n\n", infeasible, valor, bestUb, valor > bestUb + EPSILON);
#endif
      if(!infeasible && valor > bestUb + EPSILON){
#ifdef DEBUG_GRASP
         printf("\nBest solution found...\n");
         SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
#endif
         
         /* check if the solution is feasible */
         SCIP_CALL( SCIPtrySolMine(scip, *sol, TRUE, TRUE, FALSE, TRUE, &stored) );
         if( stored )
         {
#ifdef DEBUG_PRIMAL
            printf("\nSolution is feasible and was saved! Total of items = %d", nInSolution);
            SCIPdebugMessage("found feasible grasp solution:\n");
            SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
#endif
            //       *result = SCIP_FOUNDSOL;
         }
         else{
            found = 0;
#ifdef DEBUG_GRASP
            printf("\nCould not found\n. BestUb=%lf", bestUb);
#endif
         }
      }
   }
   //#ifdef DEBUG_GRASP
   //   getchar();
   //#endif
   free(solution);
   free(covered);
   return found;

}

/** execution method of primal heuristic */
static
SCIP_DECL_HEUREXEC(heurExecGrasp)
{  /*lint --e{715}*/
   SCIP_SOL*             sol;                /**< solution to round */
   int nlpcands;

   assert(result != NULL);
   //   assert(SCIPhasCurrentNodeLP(scip));

   *result = SCIP_DIDNOTRUN;

   /* continue only if the LP is finished */
   if ( SCIPgetLPSolstat(scip) != SCIP_LPSOLSTAT_OPTIMAL )
      return SCIP_OKAY;

   /* continue only if the LP value is less than the cutoff bound */
   if( SCIPisGE(scip, SCIPgetLPObjval(scip), SCIPgetCutoffbound(scip)) )
      return SCIP_OKAY;


   /* check if there exists integer variables with fractionary values in the LP */
   SCIP_CALL( SCIPgetLPBranchCands(scip, NULL, NULL, NULL, &nlpcands, NULL, NULL) );
   //Fractional implicit integer variables are stored at the positions *nlpcands to *nlpcands + *nfrac - 1
  
   /* stop if the LP solution is already integer   */
   if ( nlpcands == 0 )
     return SCIP_OKAY;

   /* solve grasp */
   if(grasp(scip, &sol, heur)){
     *result = SCIP_FOUNDSOL;
   }
   else{
     *result = SCIP_DIDNOTFIND;
#ifdef DEBUG_PRIMAL
     printf("\nGrasp could not find feasible solution!");      
#endif
   }
   return SCIP_OKAY;
}


/*
 * primal heuristic specific interface methods
 */

/** creates the grasp_crtp primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPincludeHeurGrasp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_HEURDATA* heurdata;
   SCIP_HEUR* heur;

   /* create grasp primal heuristic data */
   heurdata = NULL;

   heur = NULL;

   /* include primal heuristic */
#if 0
   /* use SCIPincludeHeur() if you want to set all callbacks explicitly and realize (by getting compiler errors) when
    * new callbacks are added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeHeur(scip, HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, param.heur_freq, param.heur_freqofs,
         param.heur_maxdepth, HEUR_TIMING, HEUR_USESSUBSCIP,
         heurCopyGrasp, heurFreeGrasp, heurInitGrasp, heurExitGrasp, heurInitsolGrasp, heurExitsolGrasp, heurExecGrasp,
         heurdata) );
#else
   /* use SCIPincludeHeurBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeHeurBasic(scip, &heur,
         HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, param.heur_round_freq, param.heur_round_freqofs,
         param.heur_round_maxdepth, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecGrasp, heurdata) );

   assert(heur != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetHeurCopy(scip, heur, heurCopyGrasp) );
   SCIP_CALL( SCIPsetHeurFree(scip, heur, heurFreeGrasp) );
   SCIP_CALL( SCIPsetHeurInit(scip, heur, heurInitGrasp) );
   SCIP_CALL( SCIPsetHeurExit(scip, heur, heurExitGrasp) );
   SCIP_CALL( SCIPsetHeurInitsol(scip, heur, heurInitsolGrasp) );
   SCIP_CALL( SCIPsetHeurExitsol(scip, heur, heurExitsolGrasp) );
#endif

   /* add grasp primal heuristic parameters */
   /* TODO: (optional) add primal heuristic specific parameters with SCIPaddTypeParam() here */

   return SCIP_OKAY;
}