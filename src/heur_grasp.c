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

#include "probdata_dpd.h"
#include "parameters_dpd.h"
#include "heur_grasp.h"
#include "heur_problem.h"

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

void maxmin(Candidatos *C, int *max, int *min, int n) {
   if (n == 0) return; // vetor vazio, nada a fazer

   // caso base: apenas um candidato
   if (n == 1) {
       *min = C[0].peso_atribuido;
       *max = C[0].peso_atribuido;
       return;
   }

   // inicializa min e max com os dois primeiros candidatos
   if (C[0].peso_atribuido < C[1].peso_atribuido) {
       *min = C[0].peso_atribuido;
       *max = C[1].peso_atribuido;
   } else {
       *min = C[1].peso_atribuido;
       *max = C[0].peso_atribuido;
   }

   // percorre em pares a partir do terceiro elemento
   for (int i = 2; i < n - 1; i += 2) {
       int menor, maior;
       if (C[i].peso_atribuido < C[i + 1].peso_atribuido) {
           menor = C[i].peso_atribuido;
           maior = C[i + 1].peso_atribuido;
       } else {
           menor = C[i + 1].peso_atribuido;
           maior = C[i].peso_atribuido;
       }

       if (menor < *min) *min = menor;
       if (maior > *max) *max = maior;
   }

   // se o numero de candidatos for impar, verifica o ultimo
   if (n % 2 != 0) {
       if (C[n - 1].peso_atribuido < *min) *min = C[n - 1].peso_atribuido;
       if (C[n - 1].peso_atribuido > *max) *max = C[n - 1].peso_atribuido;
   }
}

void cria_candidatos(Candidatos *candidatos, int *codigo_turmas, int *pesos_atribuidos, int n){
   int posicao;
   for(int i = 0; i < n; i++){
      posicao = codigo_turmas[i] -1;
      candidatos[i].codigo_turma = codigo_turmas[i];
      candidatos[i].peso_atribuido = pesos_atribuidos[posicao];
   }
}

void cria_RCL(Candidatos *candidatos, Candidatos *RCL, int alpha, int max, int min, int n, int *n_RCL){
   for(int i = 0; i < n; i++){
      if(candidatos[i].peso_atribuido >= min * alpha*(max - min)){
         RCL[*n_RCL].codigo_turma = candidatos[i].codigo_turma;
         RCL[*n_RCL].peso_atribuido = candidatos[i].peso_atribuido;
         *n_RCL += 1;
      }
   }
}

Candidatos escolherAleatorio(Candidatos *vetor, int n) {
   if (n == 0) {
       Candidatos vazio = {-1, -1};
       return vazio;
   }

   int indice = rand() % n;
   return vetor[indice];
}

void atualiza_candidatos(Candidatos *candidatos, Candidatos removido, int n){
   for(int i = 0; i < n; i++){
      if(candidatos[i].codigo_turma == removido.codigo_turma){
         candidatos[i] = removido;
      }
   }
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
   int i, posicao_turma, max_i = 10, max, min, carga_s1, carga_s2, n_RCL;
   instanceT* I;
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
    
   solution = (SCIP_VAR**) malloc(sizeof(SCIP_VAR*)*n);
   covered = (int*) calloc(m,sizeof(int));  // o vetor de cobertos vai representar as turmas. inicialmente todas as pocicoes estao com 0 (nenhuma turma foi coberta)
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

   Candidatos posicao_escolhida;
   // int solucao_final = infinito ????
   for(i = 0; i < max_i; i++){

      // fase de construcao da solucao
      // percorrendo pelos professores
      for(int j = 0;j < n; j++){
         int n_candidatos = I->professores[j].numeroPreferencias;
         carga_s1 = 0,carga_s2 = 0;
         Candidatos *candidatos = malloc(sizeof(Candidatos) * n_candidatos);
         Candidatos *RCL = malloc(sizeof(Candidatos) * n_candidatos);
         // cria a lista de candidatos (todas as turmas)
         cria_candidatos(candidatos, I->professores[j].codigo_turmas, I->professores[j].preferencias, n_candidatos);

         printf("\nTESTE\n");
         while(n_candidatos > 0 && (carga_s1 + carga_s2) <= I->professores[j].CHmin){
            maxmin(candidatos, &max, &min, n_candidatos);
            n_RCL = 0;
            cria_RCL(candidatos, RCL, 0.7, max, min, n_candidatos, &n_RCL);
            posicao_escolhida = escolherAleatorio(RCL, n_RCL);

            // printf("\nLISTA DE CANDIDATOS DO PROFESSOR %d:\n", j);
            // for(int k = 0; k < n_candidatos; k++){
            //    printf("CODIGO TURMA: %d; PESO ATRIBUIDO: %d\n", candidatos[k].codigo_turma, candidatos[k].peso_atribuido);
            // }

            printf("\nESCOLHIDA PARA O PROFESSOR %d: CODIGO TURMA: %d; PESO ATRIBUIDO: %d\n", j, posicao_escolhida.codigo_turma, posicao_escolhida.peso_atribuido);

            int semestre = I->turmas[posicao_escolhida.codigo_turma].semestre;
            int posicao_certa = posicao_escolhida.codigo_turma;
            if(semestre == 1 && (carga_s1 + I->turmas[posicao_certa].CH) <= I->professores[i].CHmax1){
               carga_s1 += I->turmas[posicao_certa].CH;
               covered[posicao_certa] = 1;
               nCovered++;
               var = varlist[(j*m)+posicao_certa];
               solution[nInSolution++] = var;

               printf("PROFESSOR: %d / TURMA: %d\n", j, posicao_certa);

               printf("\nVARIAVEL SELECIONADA: %s\n", SCIPvarGetName(var));
               printf("\nVALOR DA VARIAVEL: %f\n", SCIPgetSolVal(scip, *sol, var));


            }else if(semestre == 2 && (carga_s2 + I->turmas[posicao_certa].CH) <= I->professores->CHmax2){
               carga_s2 += I->turmas[posicao_certa].CH;
               covered[posicao_certa] = 1;
               nCovered++;
               var = varlist[(j*m)+posicao_certa];
               solution[nInSolution++] = var;

               printf("PROFESSOR: %d / TURMA: %d\n", j, posicao_certa);

               printf("\nVARIAVEL SELECIONADA: %s\n", SCIPvarGetName(var));
               printf("\nVALOR DA VARIAVEL: %f\n", SCIPgetSolVal(scip, *sol, var));

            }
            // remover da lista de candidatos esse item q foi escolhido (e marcar ele como coberto)
            atualiza_candidatos(candidatos, posicao_escolhida,n_candidatos);
            n_candidatos--;

         }

         free(candidatos);
         free(RCL);

      }


      // fase de busca local

      // if solucao_atual > solucao_final entao solucao_final = solucao_atual



   }

   


   printf("NUMERO DE TURMAS: %d / NUMERO DE TURMAS COBERTAS: %d / NUMERO DE TURMAS SEM PROF: %d\n", I->m, nCovered, I->m-nCovered);
   for(i = 0; i < I->m; i++){
      printf("%d\n", covered[i]);
      if(covered[i] == 0){
         printf("TURMA SEM PROFESSOR: %s (CODIGO: %d)\n", I->turmas[i].disciplina.nome, i+1);
      }
   }


   // if(nCovered < I->m){
   //    printf("\nEXISTEM TURMAS QUE FICARAM SEM PROFESSORES ALOCADOS!\n");
   // }

  // printf("\n NUMERO DE TURMAS COBERTAS: %d\n", nCovered);


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
