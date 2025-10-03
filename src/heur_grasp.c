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
#include <math.h>
#include <string.h>
#include <time.h>

#include "heur_grasp.h"
#include "heur_problem.h"
#include "parameters_dpd.h"
#include "probdata_dpd.h"
#include "problem.h"

//#define DEBUG_GRASP 1
/* configuracao da heuristica */
#define HEUR_NAME "grasp"
#define HEUR_DESC "grasp primal heuristic template"
#define HEUR_DISPCHAR 'g'
#define HEUR_PRIORITY 2                       /**< heuristics of high priorities are called first */
#define HEUR_FREQ 1                           /**< heuristic call frequency. 1 = in all levels of the B&B tree */
#define HEUR_FREQOFS 0                        /**< starts of level 0 (root node) */
#define HEUR_MAXDEPTH -1                      /**< maximal level to be called. -1 = no limit */
#define HEUR_TIMING SCIP_HEURTIMING_AFTERNODE /**< when the heuristic should be called? SCIP_HEURTIMING_DURINGLPLOOP or SCIP_HEURTIMING_AFTERNODE */
#define HEUR_USESSUBSCIP FALSE                /**< does the heuristic use a secondary SCIP instance? */

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
static SCIP_DECL_HEURCOPY(heurCopyGrasp)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static SCIP_DECL_HEURFREE(heurFreeGrasp)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}


/** initialization method of primal heuristic (called after problem was transformed) */
static SCIP_DECL_HEURINIT(heurInitGrasp)
{ /*lint --e{715}*/


  return SCIP_OKAY;
}


/** deinitialization method of primal heuristic (called before transformed problem is freed) */
static SCIP_DECL_HEUREXIT(heurExitGrasp)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}


/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static SCIP_DECL_HEURINITSOL(heurInitsolGrasp)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}


/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static SCIP_DECL_HEUREXITSOL(heurExitsolGrasp)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

// reset level of the each class
void reset_class(Turma *turmas, int m)
{
  for (int t = 0; t < m; t++)
  {
    turmas[t].n = 0;
  }
}

int random_number(int a, int b)
{
  if (a > b)
  {
    int temp = a;
    a        = b;
    b        = temp;
  }

  return a + rand() % (b - a + 1);
}

// calculate the score of the arests
int calculaScore(int peso_preferencia, int alfa)
{
  return peso_preferencia + alfa * 1;
}

// compare level of the two classes
int compareClasses(const void *a, const void *b)
{
  const Turma *turmaA = (const Turma *) a;
  const Turma *turmaB = (const Turma *) b;
  return (turmaA->n - turmaB->n);
}

// compare level of the two professors
int compareProfessoresByN(const void *a, const void *b)
{
  const Professor *profA = (const Professor *) a;
  const Professor *profB = (const Professor *) b;

  if (profA->n < profB->n) return -1;  // a comes first
  if (profA->n > profB->n) return 1;   // b comes first
  return 0;                            // equal
}

// check area of a professor with area of a class
int check_area(const char *a, const char *b, int size)
{
  for (int i = 0; i < size; i++)
  {
    if (a[i] == '1' && b[i] == '1')
    {
      return 1;
    }
  }
  return 0;
}

// run the vector of classes chosen by professor and check if the class with code "codigo_turma" is there
int check_preference(int *preferencias, int codigo_turma, int m)
{
  for (int i = 0; i < m; i++)
  {
    if (preferencias[i] != 0 && i + 1 == codigo_turma)
    {
      return preferencias[i];
    }
  }
  return 0;
}

// create the edges between professors and classes
void adaptive_edges(Professor *professores, Turma *turmas, int n, int m, int numareas)
{
  int peso, alfa, score, n_pref;

  /*idea:
  1° compute degrees (professors possible assignments and classes possible professors)
  2° for each feasible (prof, class) pair compute a score that favors low degree vertices
    for that: use a formula that combines the original preference weight with inverse degree terms
  */

  // zero degrees and temp counts
  for (int p = 0; p < n; p++)
  {
    professores[p].n = 0;
  }
  for (int t = 0; t < m; t++)
  {
    turmas[t].n = 0;
  }

  // 1°: count possible edges (degrees) without setting pref list
  for (int p = 0; p < n; p++)
  {
    for (int t = 0; t < m; t++)
    {
      if (turmas[t].covered == 1) continue;
      peso = check_preference(professores[p].preferencias, turmas[t].codigo, m);
      if (peso != 0)
      {
        professores[p].n++;
        turmas[t].n++;
      }
      else if (check_area(professores[p].myareas, turmas[t].disciplina.myareas, numareas) == 1)
      {
        professores[p].n++;
        turmas[t].n++;
      }
    }
  }

  // 2°: fill pref lists and compute scores using degrees
  const double K = 5.0; /* weight factor to amplify inverse-degree effect */
  for (int p = 0; p < n; p++)
  {
    n_pref = 0;  // index to fill professores[p].pref
    // if no possible classes, ensure n is zero
    if (professores[p].n == 0)
    {
      professores[p].n = 0;
      continue;
    }

    for (int t = 0; t < m; t++)
    {
      if (turmas[t].covered == 1) continue;
      peso = check_preference(professores[p].preferencias, turmas[t].codigo, m);
      if (peso != 0)
      {
        // combine base preference with inverse-degree bonuses
        double degp                              = (double) professores[p].n;  // level of the current professor
        double degt                              = (double) turmas[t].n;       // level of the current class
        double base                              = (double) peso;
        double bonus                             = K * (1.0 / (1.0 + degp) + 1.0 / (1.0 + degt));
        score                                    = (int) ceil(base * (1.0 + bonus));

        professores[p].pref[n_pref].codigo_turma = turmas[t].codigo;
        professores[p].pref[n_pref].score        = score;
        n_pref++;
      }
      else if (check_area(professores[p].myareas, turmas[t].disciplina.myareas, numareas) == 1)
      {
        double degp                              = (double) professores[p].n;  // level of the current professor
        double degt                              = (double) turmas[t].n;       // level of the current class
        double base                              = 1.0;                        // small base weight for area-based edges
        double bonus                             = K * (1.0 / (1.0 + degp) + 1.0 / (1.0 + degt));
        score                                    = (int) ceil(base * (1.0 + bonus));

        professores[p].pref[n_pref].codigo_turma = turmas[t].codigo;
        professores[p].pref[n_pref].score        = score;
        n_pref++;
      }
    }
    professores[p].n = n_pref;  // update actual filled count
  }
}

int grasp_randomized_selection(Preferencia *candidates, int num_candidates, float alpha)
{
  if (num_candidates == 1) return 0;

  // find max and min score
  int min_score = candidates[0].score;
  int max_score = candidates[0].score;
  for (int i = 0; i < num_candidates; i++)
  {
    if (candidates[i].score < min_score) min_score = candidates[i].score;
    if (candidates[i].score > max_score) max_score = candidates[i].score;
  }

  // create RCL
  int limite = min_score + alpha * (max_score - min_score);
  int rcl[num_candidates];
  int rcl_size = 0;

  for (int i = 0; i < num_candidates; i++)
  {
    if (candidates[i].score >= limite)
    {
      rcl[rcl_size] = i;
      rcl_size++;
    }
  }

  if (rcl_size == 0) return rand() % num_candidates;

  return rcl[rand() % rcl_size];
}

void construct_solution(
        SCIP *scip,
        SCIP_VAR **varlist,
        SCIP_VAR **solution,
        Professor *professores,
        Turma *turmas,
        int *nInSolution,  //number of the vars in the soluction
        int *covered,      // vector of the covered classes
        int *nCovered,     // number of the covered classes
        int m,             // number of the classes
        int n,             // number of the professors
        int numareas,      // number of the distinct areas
        float alpha)
{

  // redefining the workload of each semester for professors
  for (int i = 0; i < n; i++)
  {
    professores[i].current_CH1 = 0;
    professores[i].current_CH2 = 0;
  }

  Turma *turmas_sem_profs           = (Turma *) malloc(sizeof(Turma) * m);  // buffer sized to m
  int n_sem_prof                    = 0, num_candidates, codigo_prof;

  // track assigned variable per class code (index = codigo-1)

  SCIP_VAR **assigned_var_for_class = (SCIP_VAR **) calloc(m, sizeof(SCIP_VAR *));
  for (int i = 0; i < m; i++) assigned_var_for_class[i] = NULL;

  // build mapping codigo -> Turma* (turmas array may be reordered with qsort)
  Turma **turma_by_codigo = (Turma **) malloc(sizeof(Turma *) * m);
  for (int idx = 0; idx < m; idx++) turma_by_codigo[idx] = NULL;

  while (*nCovered < m)
  {
    int t;
    for (t = 0; t < m; t++)
    {
      Turma turma                   = turmas[t];
      Preferencia *candidate_scores = (Preferencia *) malloc(sizeof(Preferencia) * n);
      SCIP_VAR **candidate_vars     = (SCIP_VAR **) malloc(sizeof(SCIP_VAR *) * n);
      num_candidates                = 0;  // number of the candidates professors for the current class

      for (int p = 0; p < n; p++)
      {
        //if(professores[p].current_CH1 + professores[p].current_CH2 >= professores[p].CHmin) continue;
        codigo_prof = professores[p].codigo;

        for (int s = 0; s < professores[p].n && professores[p].pref[s].codigo_turma <= turma.codigo; s++)
        {

          if (turma.codigo == professores[p].pref[s].codigo_turma)
          {
            if (turma.semestre == 1)
            {

              if (professores[p].current_CH1 + turma.CH <= professores[p].CHmax1)
              {
                candidate_scores[num_candidates].codigo_turma = codigo_prof;
                candidate_scores[num_candidates].score        = professores[p].pref[s].score;
                candidate_vars[num_candidates]                = varlist[codigo_prof * m + turma.codigo - 1];
                num_candidates++;
                // printf("PROFESSOR: %s; TURMA: %s | ", professores[p].nome, turmas[t].disciplina.nome);
                // printf("CANDIDATE VARIABLE: %s\n", SCIPvarGetName(varlist[codigo_prof*m + turma.codigo-1]));
              }
            }
            else
            {
              if (professores[p].current_CH2 + turma.CH <= professores[p].CHmax2)
              {
                candidate_scores[num_candidates].codigo_turma = codigo_prof;
                candidate_scores[num_candidates].score        = professores[p].pref[s].score;
                candidate_vars[num_candidates]                = varlist[codigo_prof * m + turma.codigo - 1];
                num_candidates++;
              }
            }
            break;
          }
        }
      }

      if (num_candidates > 0)
      {
        // printf("\n=========================\n");
        int selected                             = grasp_randomized_selection(candidate_scores, num_candidates, alpha);
        int p                                    = candidate_scores[selected].codigo_turma;
        // printf("PROFESSOR SELECTED: %s | CODIGO: %d\n", professores[p].nome, professores[p].codigo);

        // register provisional assignment into mapping (we will rebuild solution list later)
        assigned_var_for_class[turma.codigo - 1] = candidate_vars[selected];
        turmas[t].covered                        = 1;
        turmas[t].current_prof                   = p;
        (*nCovered)++;

        // printf("\nVAR SELECTED: %s\n", SCIPvarGetName(candidate_vars[selected]));

        if (turma.semestre == 1)
        {
          professores[p].current_CH1 += turma.CH;
        }
        else
        {
          professores[p].current_CH2 += turma.CH;
        }

        reset_class(turmas, m);
        adaptive_edges(professores, turmas, n, m, numareas);
        qsort(turmas, m, sizeof(Turma), compareClasses);  // ordering the classes by the number of the vertex
      }
      else
      {
        turmas_sem_profs[n_sem_prof] = turma;
        n_sem_prof++;
      }


      free(candidate_scores);
      free(candidate_vars);
    }

    if (*nCovered < m)
    {
      for (int i = 0; i < n_sem_prof; i++)
      {

        if (turmas_sem_profs[i].semestre == 1)
        {
          for (int p = 0; p < n; p++)
          {
            codigo_prof = professores[p].codigo;

            if (professores[p].current_CH1 + turmas_sem_profs[i].CH <= professores[p].CHmax1)
            {
              professores[p].current_CH1 += turmas_sem_profs[i].CH;
              assigned_var_for_class[turmas_sem_profs[i].codigo - 1] = varlist[codigo_prof * m + turmas_sem_profs[i].codigo - 1];
              turmas[turmas_sem_profs[i].codigo - 1].covered         = 1;
              turmas[turmas_sem_profs[i].codigo - 1].current_prof    = codigo_prof;
              (*nCovered)++;
              break;
            }
          }
        }
        else
        {

          for (int p = 0; p < n; p++)
          {
            codigo_prof = professores[p].codigo;

            if (professores[p].current_CH2 + turmas_sem_profs[i].CH <= professores[p].CHmax2)
            {
              professores[p].current_CH2 += turmas_sem_profs[i].CH;
              assigned_var_for_class[turmas_sem_profs[i].codigo - 1] = varlist[codigo_prof * m + turmas_sem_profs[i].codigo - 1];
              turmas[turmas_sem_profs[i].codigo - 1].covered         = 1;
              turmas[turmas_sem_profs[i].codigo - 1].current_prof    = codigo_prof;
              (*nCovered)++;
              break;
            }
          }
        }
      }
    }
  }

  // printf("NUMBER OF THE COVERED CLASSES: %d\n", *nCovered);

  // create mapping codigo -> Turma* now that turmas array was possibly reordered
  for (int idx = 0; idx < m; idx++)
  {
    if (turmas[idx].codigo >= 1 && turmas[idx].codigo <= m)
      turma_by_codigo[turmas[idx].codigo - 1] = &turmas[idx];
  }

  // repair phase: try to move assigned classes to professors who are below CHmin without breaking others' CHmin/CHmax
  int repairs = 0;
  for (int i = 0; i < n; i++)
  {
    int changed = 0;
    for (int p = 0; p < n; p++)
    {
      int total_p = professores[p].current_CH1 + professores[p].current_CH2;
      if (total_p >= professores[p].CHmin) continue;
      // try to find a class j that can be moved to p
      for (int j = 0; j < m; j++)
      {
        SCIP_VAR *varj = assigned_var_for_class[j];
        if (varj == NULL) continue;

        Turma *ct = turma_by_codigo[j];
        if (ct == NULL) continue;

        int owner = ct->current_prof;
        if (owner == p) continue;

        // check semester capacity for p
        if (ct->semestre == 1)
        {
          if (professores[p].current_CH1 + ct->CH > professores[p].CHmax1) continue;
        }
        else
        {
          if (professores[p].current_CH2 + ct->CH > professores[p].CHmax2) continue;
        }

        // ensure owner won't fall below CHmin after removing this class
        int owner_total = professores[owner].current_CH1 + professores[owner].current_CH2;
        if (owner_total - ct->CH < professores[owner].CHmin) continue;

        // perform move
        if (ct->semestre == 1)
        {
          professores[p].current_CH1 += ct->CH;
          professores[owner].current_CH1 -= ct->CH;
        }
        else
        {
          professores[p].current_CH2 += ct->CH;
          professores[owner].current_CH2 -= ct->CH;
        }
        ct->current_prof          = p;

        // update assigned var mapping to point to the var of new professor
        assigned_var_for_class[j] = varlist[p * m + j];
        changed                   = 1;
        repairs++;
        break;  // try to satisfy next underloaded prof
      }
    }
    if (!changed) break;
  }

  int i = 0;
  for (int p = 0; p < n; p++)
  {
    if (professores[p].current_CH1 + professores[p].current_CH2 < professores[p].CHmin) i++;
  }
  // printf("NUMERO DE PROFS COM CH NAO ATENDIDA: %d (apos reparos=%d)\n", i, repairs);

  // rebuild solution array from assigned_var_for_class
  *nInSolution = 0;
  for (int j = 0; j < m; j++)
  {
    if (assigned_var_for_class[j] != NULL)
    {
      solution[(*nInSolution)++] = assigned_var_for_class[j];
    }
  }

  free(assigned_var_for_class);
  free(turma_by_codigo);
  free(turmas_sem_profs);
}


/**
 * @brief Core of the grasp heuristic: it builds one solution for the problem by grasp procedure.
 *
 * @param scip problem
 * @param sol pointer to the solution structure where the solution wil be saved
 * @param heur pointer to the grasp heuristic handle (to contabilize statistics)
 * @return int 1 if solutions is found, 0 otherwise.
 */
int grasp(SCIP *scip, SCIP_SOL **sol, SCIP_HEUR *heur)
{
  int found, infeasible, nInSolution;
  unsigned int stored;
  int nvars;
  int *covered, n, m, nCovered, numareas;
  SCIP_VAR *var, **solution, **varlist;
  //  SCIP* scip_cp;
  SCIP_Real valor, bestUb;
  SCIP_PROBDATA *probdata;
  int i;
  instanceT *I;
  Professor *professores;
  Turma *turmas;

  found      = 0;
  infeasible = 0;

#ifdef DEBUG_GRASP
  printf("\n============== New grasp heur at node: %lld\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
#endif

  /* recover the problem data*/
  probdata = SCIPgetProbData(scip);
  assert(probdata != NULL);

  nvars       = SCIPprobdataGetNVars(probdata);
  varlist     = SCIPprobdataGetVars(probdata);
  I           = SCIPprobdataGetInstance(probdata);
  n           = I->n;  // number of the professors
  m           = I->m;  // number of the class
  numareas    = I->numAreas;


  // professores = (Professor *) malloc(sizeof(Professor) * n);
  // turmas      = (Turma *) malloc(sizeof(Turma) * m);
  solution    = (SCIP_VAR **) malloc(sizeof(SCIP_VAR *) * (m * n));
  covered     = (int *) calloc(m, sizeof(int));
  nInSolution = 0;
  nCovered    = 0;

  // first, select all variables already fixed in 1.0
  for (i = 0; i < nvars; i++)
  {
    var = varlist[i];
    if (SCIPvarGetLbLocal(var) > 1.0 - EPSILON)
    {  // var >= 1.0
      solution[nInSolution++] = var;
      covered[i % m]          = 1;
      nCovered++;
#ifdef DEBUG_GRASP
      printf("\nSelected fixed var= %s. TotalItems=%d infeasible=%d", SCIPvarGetName(var), nInSolution, infeasible);
#endif
    }
    else
    {  // discard items fixed in 0.0
      if (SCIPvarGetUbLocal(var) < EPSILON)
      {  // var fixed in 0.0
        //covered[i] = 1;
        //nCovered++;
      }
    }
  }

  // copiando as insfo para vars auxiliares
  professores = I->professores;
  turmas      = I->turmas;
  int j, pref, score, alfa, nscore;
  reset_class(turmas, m);
  adaptive_edges(professores, turmas, n, m, numareas);

  qsort(turmas, m, sizeof(Turma), compareClasses);  // ordenando as turmas pelo grau do vertice

  construct_solution(scip, varlist, solution, I->professores, I->turmas, &nInSolution, covered, &nCovered, m, n, numareas, 0.9);


  if (!infeasible)
  {
    /* create SCIP solution structure sol */
    SCIP_CALL(SCIPcreateSol(scip, sol, heur));
    //FILE *arquivo2 = fopen("SOLUTION.txt", "w");
    //printf("\n\n === VARS QUE ESTÃO NA SOLUÇÃO ==== %d \n\n", current_nInSolution);

    // save found solution in sol
    for (i = 0; i < nInSolution; i++)
    {
      var = solution[i];
      SCIP_CALL(SCIPsetSolVal(scip, *sol, var, 1.0));
      //fprintf(arquivo2, "VAR: %s\n", SCIPvarGetName(solution[i]));
    }

    //fclose(arquivo2);

    /* compute objective value of the built nlsolution */
    bestUb = SCIPgetPrimalbound(scip);
    valor  = SCIPsolGetOrigObj(*sol);
#ifdef DEBUG_GRASP
    printf("\nFound solution...\n");
    //      SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
    printf("\ninfeasible=%d value = %lf > bestUb = %lf? %d\n\n", infeasible, valor, bestUb, valor > bestUb + EPSILON);
#endif
    if (!infeasible && valor > bestUb + EPSILON)
    {
#ifdef DEBUG_GRASP
      printf("\nBest solution found...\n");
      SCIP_CALL(SCIPprintSol(scip, *sol, NULL, FALSE));
#endif

      /* check if the solution is feasible */
      SCIP_CALL(SCIPtrySolMine(scip, *sol, TRUE, TRUE, FALSE, TRUE, &stored));
      if (stored)
      {
#ifdef DEBUG_PRIMAL
        printf("\nSolution is feasible and was saved! Total of items = %d", nInSolution);
        SCIPdebugMessage("found feasible grasp solution:\n");
        SCIP_CALL(SCIPprintSol(scip, *sol, NULL, FALSE));
#endif
        // *result = SCIP_FOUNDSOL;
      }

      //
      else
      {
        found = 0;
#ifdef DEBUG_GRASP
        printf("\nCould not found\n. BestUb=%lf", bestUb);
#endif
      }

      FILE *arq   = fopen("solucao.txt", "w");
      SCIP_Real z = SCIPsolGetOrigObj(*sol);
      fprintf(arq, "%f ", z);
      fclose(arq);
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
static SCIP_DECL_HEUREXEC(heurExecGrasp)
{                /*lint --e{715}*/
  SCIP_SOL *sol; /**< solution to round */
  int nlpcands;

  assert(result != NULL);
  //   assert(SCIPhasCurrentNodeLP(scip));

  *result = SCIP_DIDNOTRUN;

  /* continue only if the LP is finished */
  if (SCIPgetLPSolstat(scip) != SCIP_LPSOLSTAT_OPTIMAL)
    return SCIP_OKAY;

  /* continue only if the LP value is less than the cutoff bound */
  if (SCIPisGE(scip, SCIPgetLPObjval(scip), SCIPgetCutoffbound(scip)))
    return SCIP_OKAY;


  /* check if there exists integer variables with fractionary values in the LP */
  SCIP_CALL(SCIPgetLPBranchCands(scip, NULL, NULL, NULL, &nlpcands, NULL, NULL));
  //Fractional implicit integer variables are stored at the positions *nlpcands to *nlpcands + *nfrac - 1

  /* stop if the LP solution is already integer   */
  if (nlpcands == 0)
    return SCIP_OKAY;

  /* solve grasp */
  if (grasp(scip, &sol, heur))
  {
    *result = SCIP_FOUNDSOL;
  }
  else
  {
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
        SCIP *scip /**< SCIP data structure */
)
{
  SCIP_HEURDATA *heurdata;
  SCIP_HEUR *heur;

  /* create grasp primal heuristic data */
  heurdata = NULL;

  heur     = NULL;

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
  SCIP_CALL(SCIPincludeHeurBasic(scip, &heur,
                                 HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, param.heur_round_freq, param.heur_round_freqofs,
                                 param.heur_round_maxdepth, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecGrasp, heurdata));

  assert(heur != NULL);

  /* set non fundamental callbacks via setter functions */
  SCIP_CALL(SCIPsetHeurCopy(scip, heur, heurCopyGrasp));
  SCIP_CALL(SCIPsetHeurFree(scip, heur, heurFreeGrasp));
  SCIP_CALL(SCIPsetHeurInit(scip, heur, heurInitGrasp));
  SCIP_CALL(SCIPsetHeurExit(scip, heur, heurExitGrasp));
  SCIP_CALL(SCIPsetHeurInitsol(scip, heur, heurInitsolGrasp));
  SCIP_CALL(SCIPsetHeurExitsol(scip, heur, heurExitsolGrasp));
#endif

  /* add grasp primal heuristic parameters */
  /* TODO: (optional) add primal heuristic specific parameters with SCIPaddTypeParam() here */

  return SCIP_OKAY;
}