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

int grasp_randomized_selection(SCORE *candidates, int num_candidates, float alpha){
   if(num_candidates == 1) return 0;

   // encontrando o score max e min
   int min_score = candidates[0].score;
   int max_score = candidates[0].score;
   for(int i = 0; i < num_candidates; i++){
      if(candidates[i].score < min_score) min_score = candidates[i].score;
      if(candidates[i].score > max_score) max_score = candidates[i].score;
   }

   // criando a RCL
   int limite = min_score + alpha * (max_score-min_score);
   int rcl[num_candidates];
   int rcl_size=0;

   for(int i = 0; i < num_candidates; i++){
      if(candidates[i].score >= limite){
         rcl[rcl_size] = i;
         rcl_size++;
      }
   }
   
   // selecionando aleatoriamente da RCL
   if(rcl_size==0) return rand() % num_candidates;

   return rcl[rand() % rcl_size];

}

void construct_soluction(
   SCIP* scip,
   SCIP_VAR** varlist,
   SCIP_VAR** solution,
   Professor* professores,
   Turma* turmas,
   int* nInSolution, //num de vars na solucao
   int* covered,  // vetor de turmas cobertas
   int* nCovered, // quant de turmas cobertas
   int m,         // quant de turmas
   int n,         // quant de prof
   float alpha
) {
   // redefinindo a carga horaria de cada semestre dos professores
   for(int i = 0; i < n; i++){
      professores[i].current_CH1 = 0;
      professores[i].current_CH2 = 0;
   }

   Turma *turmas_sem_profs = (Turma*) malloc(sizeof(Turma) * 10); // considerando que ao final ficarao, no max, 10 turmas sem profs
   int n_sem_prof = 0;

   while(*nCovered < m){

      // percorrendo as turmas
      int t;
      for(t = 0; t < m; t++){
         
         // pegando as isnfo da turma atual t
         Turma turma = turmas[t];
         SCORE *candidate_scores = (SCORE*) malloc(sizeof(SCORE) * n);  
         SCIP_VAR **candidate_vars = (SCIP_VAR**) malloc(n * sizeof(SCIP_VAR*));
         int num_candidates = 0;

        // printf("turma: %d\n", turma.codigo);
         // percorrendo os profs
         printf("TURMA: %d\n", turma.codigo);
         for(int p = 0; p < n; p++){
           //printf("nome: %s\n", professores[p].nome);

            // indo na lista de pref do prof p, e verificando se a turma t esta la
            for(int s = 0; s < professores[p].n && professores[p].Score[s].codigo_turma <= turma.codigo; s++){

              // printf("turma: %d ; turma escolhida pelo prof: %s\n",professores[p].Score[s].codigo_turma ,professores[p].nome);
               // verificando se o prof p tem a materia t na sua lista
               if(turma.codigo == professores[p].Score[s].codigo_turma){

                  // verificando se o prof p pode ser candidato para a turma t(verificar a carga horaria dele).
                  if(turma.semestre == 1){

                     // checando a carga horaria do prof no semestre 1
                     if(professores[p].current_CH1 + turma.CH <= professores[p].CHmax1){
                        candidate_scores[num_candidates].codigo_turma = p;  // p = 0 --> amaury
                        candidate_scores[num_candidates].score = professores[p].Score[s].score;
                        candidate_vars[num_candidates] = varlist[p*m + t];
                        num_candidates++;

                       //printf("\nVARIAVEL SELECIONADA: %s\n", SCIPvarGetName(varlist[p*m + turma.codigo-1]));

                     }
                  }else{
                     // checando a carga horaria do prof no semestre 2
                     if(professores[p].current_CH2 + turma.CH <= professores[p].CHmax2){
                        candidate_scores[num_candidates].codigo_turma = p;  // p = 0 --> amaury
                        candidate_scores[num_candidates].score = professores[p].Score[s].score;
                        candidate_vars[num_candidates] = varlist[p*m + t];
                        num_candidates++;

                      //  printf("\nVARIAVEL SELECIONADA: %s\n", SCIPvarGetName(varlist[p*m + turma.codigo-1]));

                     }
                  }
                  break;  // cod turma beteu com o cod do prof. logo, saia do laço desse prof
               }
            }
         }

         // se a lista de candidatos da turma t nao estiver vazia
         printf("numero de prof candidatos: %d\n", num_candidates);
         if(num_candidates > 0){
            // seleciona um prof da rcl
            int selected = grasp_randomized_selection(candidate_scores, num_candidates, alpha);
            int p = candidate_scores[selected].codigo_turma;
            //printf("prof selecionado: %d\n", p);

            // marcando a turma t como coberta
            covered[t] = 1;
            (*nCovered)++;
            solution[*nInSolution] = candidate_vars[selected];
            (*nInSolution)++;

            //printf("\nVARIAVEL SELECIONADA: %s\n", SCIPvarGetName(candidate_vars[selected]));

            printf("numero de turmas cobertas: %d\n", *nCovered);

            if(turma.semestre == 1){
               professores[p].current_CH1 += turma.CH;

            }else{
               professores[p].current_CH2 += turma.CH;
            }
         }else{
            // turma t ficou nem nenhum prof candidato. salvando as insfo dela
            turmas_sem_profs[n_sem_prof] = turma;
            n_sem_prof++;
         }

         free(candidate_scores);
         free(candidate_vars);

      }
      // ======  FIM DO LAÇO QUE PERCORRE TODAS AS TURMAS =====

      // verificanndo se existem turmas que não foram cobertas
      if(*nCovered < m){

         // percorrendo as turmas que ficaram sem prof
         for(int i = 0; i < n_sem_prof; i++){
            if(turmas_sem_profs[i].semestre == 1){
               // encontrando o primeiro prof que possui carga horaria livre no 1° semestre
               for(int p = 0; p < n; p++){
                  printf("nome: %s\n", professores[p].nome);
                  if(professores[p].current_CH1 + turmas_sem_profs[i].CH <= professores[p].CHmax1){
                     professores[p].current_CH1 += turmas_sem_profs[i].CH;
                     covered[turmas_sem_profs[i].codigo-1];
                     (*nCovered)++;
                     solution[*nInSolution] = varlist[p*m + turmas_sem_profs[i].codigo-1];

                     printf("\nVARIAVEL SELECIONADA: %s\n", SCIPvarGetName(varlist[p*m + turmas_sem_profs[i].codigo-1]));
                     break;
                  }
               }

            }else{
               // encontrando o primeiro prof que possui carga horaria livre no 2° semestre
               for(int p = 0; p < n; p++){
                  if(professores[p].current_CH2 + turmas_sem_profs[i].CH <= professores[p].CHmax2){
                     professores[p].current_CH2 += turmas_sem_profs[i].CH;
                     covered[turmas_sem_profs[i].codigo-1];
                     (*nCovered)++;
                     solution[*nInSolution] = varlist[p*m + turmas_sem_profs[i].codigo-1];

                     printf("\nVARIAVEL SELECIONADA: %s\n", SCIPvarGetName(varlist[p*m + turmas_sem_profs[i].codigo-1]));
                     break;
                  }
               }
            } 
         }
      }
      
      // for(int i = 0; i < n; i++){
      //    printf("nome: %s, ch1 atual: %d, ch2 atual: %d\n", professores[i].nome, professores[i].current_CH1, professores[i].current_CH2);
      // }

      printf("\n\nFIM\n\nTURMAS COBERTAS: %d ; TURMAS: %d\n", *nCovered, m);
      // se eu passei por todas as turmas, independente de terem ficado turmas sem profs, saia
      // if(t == m){
      //    break;
      // }

      free(turmas_sem_profs);

   }

}

// funcao para verificar se a area de uma turma coincide com a area do prof
int check_area(const char a[15], const char b[15]) {
    for (int i = 0; i < 15; i++) {
        if (a[i] == '1' && b[i] == '1') {
            return 1;
        }
    }
    return 0;
}

int random_number(int a, int b) {
    // garante que a é menor do que b
    if (a > b) {
        int temp = a;
        a = b;
        b = temp;
    }

   //  // inicializa a semente do gerador de numeros aleatórios apenas uma vez
   //  static int inicializado = 0;
   //  if (!inicializado) {
   //      srand(time(NULL));
   //      inicializado = 1;
   //  }

    // gera um numero inteiro aleatório entre a e b (inclusive)
    return a + rand() % (b - a + 1);
}


int check_preference(int *preferencias, int codigo_turma, int m){

   // percorrendo todas as turmas
   for(int i = 0; i < m; i++){ 
      if(preferencias[i] != 0 && i+1 == codigo_turma){
         //printf("teste1\n");
         return preferencias[i];
      }
   }
   return 0;
}

int calculaScore(int peso_preferencia, int alfa){
   return peso_preferencia + alfa * 1;
}

void reset_vector(int *vetor, int n){
   for(int i = 0; i < n; i++){
      vetor[i] = 0;
   }
}

int compareTurmasByN(const void *a, const void *b)
{
    const Turma *turmaA = (const Turma *)a;
    const Turma *turmaB = (const Turma *)b;
    
    if (turmaA->n < turmaB->n) return -1;
    if (turmaA->n > turmaB->n) return 1;
    return 0;
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
   int i;
   instanceT* I;
   Professor *professores;
   Turma *turmas;

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
   turmas = (Turma*) malloc(sizeof(Turma) * m);
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

   // copiando as insfo para vars auxiliares
   professores = I->professores;
   turmas = I->turmas;
   int j, pref, score, alfa, nscore;
   
   // percorre os professores
   for(int i = 0; i < n; i++){
      nscore=0;  // quant de disciplinas que o prof i pode ministrar (escolheu + area)
      
      // percorre as turmas
      for(j = 0; j < m; j++){

         // verificado se a turma j foi escolhida pelo prof i
         pref = check_preference(professores[i].preferencias, turmas[j].codigo, m);
         if(pref != 0){
            alfa = random_number(1, 5);
            score = calculaScore(pref, alfa);
            professores[i].Score[nscore].score = score;
            professores[i].Score[nscore].codigo_turma = j+1;
            nscore++;
            turmas[j].n++;

            // nao foi escolhida. verificando se a turma j é da area do prof i
         }else if(check_area(professores[i].myareas, turmas[j].disciplina.myareas) == 1){
            alfa = random_number(1, 8);
            //score = calculaScore(0, alfa);  // score quando n tem pref eh so o alfa. posso tentar mudar isso depois
            professores[i].Score[nscore].score = alfa;
            professores[i].Score[nscore].codigo_turma = j+1;
            nscore++; 
            turmas[j].n++;
        }

      }
      //printf("NSCORE: %d\n", nscore);
      professores[i].n = nscore;
      
   }
   // ordenando as turmas com a menor quant de profs
  // qsort(turmas, m, sizeof(Turma), compareTurmasByN);

   // for(int i = 0; i < m; i++){
   //    printf("turma: %d ; profs que podem ministrar: %d\n", turmas[i].codigo, turmas[i].n);
   // }


   // ================================================================================================================================
   
   SCIP_VAR** best_solution = (SCIP_VAR**)malloc(m *sizeof(SCIP_VAR*));  // m pq a quant de var na solucao final é no maximo m (quant de turmas)
   SCIP_VAR** current_solution = (SCIP_VAR**)malloc(m *sizeof(SCIP_VAR*));

   int maxitr = 10, best_nInSolution = 0, current_nInSolution = 0;
   double y = SCIPinfinity(scip);  // y = x^*
   

   for(int k = 0; k < maxitr; k++){
      // fase de construção da solucao
      construct_soluction(scip, varlist, current_solution, professores, turmas, &current_nInSolution, covered, &nCovered, m, n, 0.3);

   




      // antes de chamar a funcao construct_soluction, eu preciso percorrer o vetor covered e marcar todas as turmas como nao cobertas. ou seha, atribuir o valor 1
      nCovered = 0;
      reset_vector(covered, m);
      printf("\n\nFIM DA ITERACO %d\n\n", k);
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
   free(professores);
   free(turmas);
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