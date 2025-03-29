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

/**@file   heur_gulosa.c
 * @brief  gulosa primal heuristic
 * @author Edna Hoshino (based on template provided by Tobias Achterberg)
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "probdata_dpd.h"
#include "parameters_dpd.h"
#include "heur_gulosa.h"
#include "heur_problem.h"

//#define DEBUG_GULOSA 1
/* configuracao da heuristica */
#define HEUR_NAME             "gulosa"
#define HEUR_DESC             "gulosa primal heuristic template"
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
SCIP_DECL_HEURCOPY(heurCopyGulosa)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static
SCIP_DECL_HEURFREE(heurFreeGulosa)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}


/** initialization method of primal heuristic (called after problem was transformed) */
static
SCIP_DECL_HEURINIT(heurInitGulosa)
{  /*lint --e{715}*/


   return SCIP_OKAY;
}


/** deinitialization method of primal heuristic (called before transformed problem is freed) */
static
SCIP_DECL_HEUREXIT(heurExitGulosa)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}


/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static
SCIP_DECL_HEURINITSOL(heurInitsolGulosa)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}


/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static
SCIP_DECL_HEUREXITSOL(heurExitsolGulosa)
{  /*lint --e{715}*/

   return SCIP_OKAY;
}

void trocar(int *a, int *b) {
   int temp = *a;
   *a = *b;
   *b = temp;
}

int particionar(Auxiliar *dados, int baixo, int alto) {
   int pivo = dados->peso_atribuido[alto];  // pivo sera o ultimo elemento
   int i = baixo - 1;

   for (int j = baixo; j < alto; j++) {
       if (dados->peso_atribuido[j] >= pivo) {  // ordenando de forma decrescente
           i++;
           trocar(&dados->peso_atribuido[i], &dados->peso_atribuido[j]);
           trocar(&dados->codigo_turma[i], &dados->codigo_turma[j]);
       }
   }
   
   trocar(&dados->peso_atribuido[i + 1], &dados->peso_atribuido[alto]);
   trocar(&dados->codigo_turma[i + 1], &dados->codigo_turma[alto]);
   
   return i + 1;
}

void quickSort(Auxiliar *dados, int baixo, int alto) {
   if (baixo < alto) {
       int pi = particionar(dados, baixo, alto);

       quickSort(dados, baixo, pi - 1);
       quickSort(dados, pi + 1, alto);
   }
}

void imprimir(Auxiliar *dados, int n) {
   printf("codigo_turma; peso_atribuido;\n");
   for (int i = 0; i < n; i++) {
       printf("%d;%d;\n", dados->codigo_turma[i], dados->peso_atribuido[i]);
   }
}


/**
 * @brief Core of the gulosa heuristic: it builds one solution for the problem by gulosa procedure.
 *
 * @param scip problem
 * @param sol pointer to the solution structure where the solution wil be saved
 * @param heur pointer to the gulosa heuristic handle (to contabilize statistics)
 * @return int 1 if solutions is found, 0 otherwise.
 */
int gulosa(SCIP* scip, SCIP_SOL** sol, SCIP_HEUR* heur)
{
   int found, infeasible, nInSolution;
   unsigned int stored;
   int nvars;
   int *covered, n, m, nCovered;
   SCIP_VAR *var, **solution, **varlist;
   //  SCIP* scip_cp;
   SCIP_Real valor, bestUb;
   SCIP_PROBDATA* probdata;
   int i, posicao_turma;
   instanceT* I;
   Auxiliar* profs_auxiliar;

   found = 0;
   infeasible = 0;
   
#ifdef DEBUG_GULOSA
   printf("\n============== New gulosa heur at node: %lld\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
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

   profs_auxiliar = (Auxiliar*) malloc(sizeof(Auxiliar) * n);  // aloco para a quant de professores

   // pegando, para cada prof, o cod da turma e o peso atribuido
   int quant_pref;
   for(i = 0; i < n; i++){
      quant_pref = I->professores[i].numeroPreferencias;
      profs_auxiliar[i].codigo_turma = (int*) malloc(sizeof(int) * quant_pref);
      profs_auxiliar[i].peso_atribuido = (int*) malloc(sizeof(int) * quant_pref);
      //printf("\nQUANT DE PREF DO PROFESSOR %d: %d", i, quant_pref);
      for(int j = 0; j < quant_pref; j++){
         posicao_turma = I->professores[i].codigo_turmas[j]-1;
         profs_auxiliar[i].codigo_turma[j] = I->professores[i].codigo_turmas[j];
         profs_auxiliar[i].peso_atribuido[j] = I->professores[i].preferencias[posicao_turma];

         //printf("\n PROFESSOR %d: CODIGO DA TURMA: %d: PESO ASSOCIADO: %d\n",i, profs_auxiliar[i].codigo_turma[j], profs_auxiliar[i].peso_atribuido[j]);
      }

      quickSort(&profs_auxiliar[i], 0, quant_pref-1);

      // printf("\nDEPOIS DA ORDENACAO:\n");
      // printf("PROFESSOR: %s\n", I->professores[i].nome);
      // imprimir(&profs_auxiliar[i], quant_pref);
   }

   


   // first, select all variables already fixed in 1.0
   for(i=0;i<nvars;i++){
      var = varlist[i];
      if(SCIPvarGetLbLocal(var) > 1.0 - EPSILON){ // var >= 1.0
        solution[nInSolution++]=var;        
        covered[i%m] = 1;
        nCovered++;
#ifdef DEBUG_GULOSA
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

   /*IDEIA DE ALGORITMO: faz um laço que percorre todos os professores, e dentro desse laço vc atribui as turmas para o professor corrente de modo que:
   para o prof corrente: restricao 1 -> ele tenha uma carga horaria minima anual; restricao2 -> ele tenha uma carga horaria no 1° semestre que nao ultrapasse a sua
   carga horaria maxima 1; restricao 3 -> mesma da 2, mas para a carga horaria maxima fornecida para o 2° semestre
   
   ============================================================================================================================================================

   depois desse laço, certemante havera turmas que ficaram sem professores. para corrigir isso eu faço um outro laço que anda em todas as turmas, e toda vez que eu encontrar
   uma turma que esta sem prof, eu percorro nos professores ate encontrar um que possa ministrar ela. nesse caso, eu faço uma troca das disciplinas desse prof encontrado
   OBS: essa troca pode vir a quebrar uma das restriçoes do professor escolhido (restricoes essas que foram atendidas no primeiro laço)
   */

   // percorrendo os professores
   int carga_s1, carga_s2;
   int j, flag;
   for(i = 0; i < n; i++){
      carga_s1 = 0, carga_s2 = 0;
      
      
      // enquanto a carga horaria for menor que o minimo que ele deve cumprir anualmente
      j = 0, flag = 0;
      printf("\nCARGA HORARIA ANUAL DO PROFESSOR %s: %d\n", I->professores[i].nome, I->professores[i].CHmin);
      printf("CARGA HORARIA MAXIMA DO 1° SEMESTRE DO PROF %s: %d\n", I->professores[i].nome, I->professores[i].CHmax1);
      printf("CARGA HORARIA MAXIMA DO 2° SEMESTRE DO PROF %s: %d\n", I->professores[i].nome, I->professores[i].CHmax2);
      printf("\n NUMERO DE TURMAS COBERTAS: %d\n", nCovered);
      while((carga_s1+carga_s2) < I->professores[i].CHmin && flag < I->professores[i].numeroPreferencias){
         posicao_turma = profs_auxiliar[i].codigo_turma[j];  // selecionando a turma

         //printf("\nTURMA SELECIONADA PARA O PROFESSOR %s: %d (SEMESTRE: %d)\n", I->professores[i].nome, posicao_turma, I->turmas[posicao_turma].semestre);
         
         if(I->turmas[posicao_turma].semestre == 1 && carga_s1 < I->professores[i].CHmax1 && !covered[posicao_turma]){
            carga_s1 += I->turmas[posicao_turma].CH;  // descontando a carga horaria da turma selecionada na carga do 1° semestre
            // selecionar a variavel, colocar essa variavel na solucao e marcar a turma como coberta
            covered[posicao_turma] = 1;
            nCovered++;
            printf("\nTURMA SELECIONADA PARA O PROFESSOR %s: %d (SEMESTRE: %d)\n", I->professores[i].nome, posicao_turma, I->turmas[posicao_turma].semestre);
         }

         if(I->turmas[posicao_turma].semestre == 2 && carga_s2 < I->professores[i].CHmax2 && !covered[posicao_turma]){
            carga_s2 += I->turmas[posicao_turma].CH;  // descontando a carga horaria da turma selecionada na carga do 2° semestre

            // selecionar a variavel, colocar essa variavel na solucao e marcar a turma como coberta
            covered[posicao_turma] = 1;
            nCovered++;
            printf("\nTURMA SELECIONADA PARA O PROFESSOR %s: %d (SEMESTRE: %d)\n", I->professores[i].nome, posicao_turma, I->turmas[posicao_turma].semestre);
         }

         j++;
         flag++;

      }
   }

   printf("\n NUMERO DE TURMAS COBERTAS: %d\n", nCovered);


   
   // complete solution using items not fixed (not covered)
   for(i=0;i<n && nCovered < n ;i++){

      /*
     // only select actived var in scip and whose gulosa up is valid for the problem
     if(!covered[i] && I->item[i].weight <= residual){
       var = varlist[i];
       // include selected var in the solution
       solution[nInSolution++]=var;
        // update residual capacity
        residual -= I->item[i].weight;
       // update vertex covered by the current solution
       covered[i] = 1;
       nCovered++;
       custo += I->item[i].value;
       infeasible = residual<0?1:0;
       
#ifdef DEBUG_GULOSA
       printf("\n\nSelected var= %s. TotalItems=%d value=%d residual=%d infeasible=%d\n", SCIPvarGetName(var), nInSolution, custo, residual, infeasible);
#endif
     }
     comented
       */
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
#ifdef DEBUG_GULOSA
      printf("\nFound solution...\n");
      //      SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
      printf("\ninfeasible=%d value = %lf > bestUb = %lf? %d\n\n", infeasible, valor, bestUb, valor > bestUb + EPSILON);
#endif
      if(!infeasible && valor > bestUb + EPSILON){
#ifdef DEBUG_GULOSA
         printf("\nBest solution found...\n");
         SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
#endif
         
         /* check if the solution is feasible */
         SCIP_CALL( SCIPtrySolMine(scip, *sol, TRUE, TRUE, FALSE, TRUE, &stored) );
         if( stored )
         {
#ifdef DEBUG_PRIMAL
            printf("\nSolution is feasible and was saved! Total of items = %d", nInSolution);
            SCIPdebugMessage("found feasible gulosa solution:\n");
            SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
#endif
            //       *result = SCIP_FOUNDSOL;
         }
         else{
            found = 0;
#ifdef DEBUG_GULOSA
            printf("\nCould not found\n. BestUb=%lf", bestUb);
#endif
         }
      }
   }
   //#ifdef DEBUG_GULOSA
   //   getchar();
   //#endif
   free(solution);
   free(covered);
   return found;

}

/** execution method of primal heuristic */
static
SCIP_DECL_HEUREXEC(heurExecGulosa)
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

   /* solve gulosa */
   if(gulosa(scip, &sol, heur)){
     *result = SCIP_FOUNDSOL;
   }
   else{
     *result = SCIP_DIDNOTFIND;
#ifdef DEBUG_PRIMAL
     printf("\nGulosa could not find feasible solution!");      
#endif
   }
   return SCIP_OKAY;
}


/*
 * primal heuristic specific interface methods
 */

/** creates the gulosa_crtp primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPincludeHeurGulosa(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_HEURDATA* heurdata;
   SCIP_HEUR* heur;

   /* create gulosa primal heuristic data */
   heurdata = NULL;

   heur = NULL;

   /* include primal heuristic */
#if 0
   /* use SCIPincludeHeur() if you want to set all callbacks explicitly and realize (by getting compiler errors) when
    * new callbacks are added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeHeur(scip, HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, param.heur_freq, param.heur_freqofs,
         param.heur_maxdepth, HEUR_TIMING, HEUR_USESSUBSCIP,
         heurCopyGulosa, heurFreeGulosa, heurInitGulosa, heurExitGulosa, heurInitsolGulosa, heurExitsolGulosa, heurExecGulosa,
         heurdata) );
#else
   /* use SCIPincludeHeurBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeHeurBasic(scip, &heur,
         HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, param.heur_round_freq, param.heur_round_freqofs,
         param.heur_round_maxdepth, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecGulosa, heurdata) );

   assert(heur != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetHeurCopy(scip, heur, heurCopyGulosa) );
   SCIP_CALL( SCIPsetHeurFree(scip, heur, heurFreeGulosa) );
   SCIP_CALL( SCIPsetHeurInit(scip, heur, heurInitGulosa) );
   SCIP_CALL( SCIPsetHeurExit(scip, heur, heurExitGulosa) );
   SCIP_CALL( SCIPsetHeurInitsol(scip, heur, heurInitsolGulosa) );
   SCIP_CALL( SCIPsetHeurExitsol(scip, heur, heurExitsolGulosa) );
#endif

   /* add gulosa primal heuristic parameters */
   /* TODO: (optional) add primal heuristic specific parameters with SCIPaddTypeParam() here */

   return SCIP_OKAY;
}
