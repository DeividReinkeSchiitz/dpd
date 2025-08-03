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

/**@file   heur_lns.c
 * @brief  lns primal heuristic
 * @author Edna Hoshino (based on template provided by Tobias Achterberg)
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "heur_lns.h"
#include "parameters_dpd.h"
#include "probdata_dpd.h"
#include "scip/scip.h"
#include "utils.h"
#include <assert.h>

extern char output_path[256];

#define DEBUG_LNS 1
#if DEBUG_LNS
#define PRINTFLNS(...) printf("\nLNS: " __VA_ARGS__)
#else
#define PRINTFLNS(...)
#endif

/* configuracao da heuristica */
#define HEUR_NAME "lns"
#define HEUR_DESC "lns primal heuristic template"
#define HEUR_DISPCHAR 'l'
#define HEUR_PRIORITY 1                       /**< heuristics of high priorities are called first */
#define HEUR_FREQ 1                           /**< heuristic call frequency. 1 = in all levels of the B&B tree */
#define HEUR_FREQOFS 0                        /**< starts of level 0 (root node) */
#define HEUR_MAXDEPTH -1                      /**< maximal level to be called. -1 = no limit */
#define HEUR_TIMING SCIP_HEURTIMING_AFTERNODE /**< when the heuristic should be called? SCIP_HEURTIMING_DURINGLPLOOP or SCIP_HEURTIMING_AFTERNODE */
#define HEUR_USESSUBSCIP TRUE                 /**< does the heuristic use a secondary SCIP instance? */

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
SCIP_RETCODE configScip(SCIP **pscip, parametersT param);

/*
 * Local methods
 */
/*
 * Callback methods of primal heuristic
 */
int comparePreferences(const void *a, const void *b);
int compareProfessors(const void *a, const void *b);

/* TODO: Implement all necessary primal heuristic methods. The methods with an #if 0 ... #else #define ... are optional */

/** copy method for primal heuristic plugins (called when SCIP copies plugins) */
static SCIP_DECL_HEURCOPY(heurCopyLns)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static SCIP_DECL_HEURFREE(heurFreeLns)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** initialization method of primal heuristic (called after problem was transformed) */
static SCIP_DECL_HEURINIT(heurInitLns)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** deinitialization method of primal heuristic (called before transformed problem is freed) */
static SCIP_DECL_HEUREXIT(heurExitLns)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static SCIP_DECL_HEURINITSOL(heurInitsolLns)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static SCIP_DECL_HEUREXITSOL(heurExitsolLns)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/**
 * @brief Core of the lns heuristic: it builds one solution for the problem by lns procedure.
 *
 * @param scip problem
 * @param sol pointer to the solution structure to be improved
 * @param heur pointer to the lns heuristic handle (to contabilize statistics)
 * @return int 1 if solutions is found, 0 otherwise.
 */
int lns(SCIP *scip, SCIP_SOL *initsol, SCIP_HEUR *heur)
{
  PRINTFLNS("\nLNS heuristic called.\n");

  parametersT lnsparam;
  int found, nInSolution;
  unsigned int stored;
  int custo;
  int *fixed;
  SCIP_VAR *var, **vars, **vars2;
  SCIP_Real value;
  SCIP_PROBDATA *probdata, *probdata2;
  SCIP_SOL *lnsSol, *sol;
  int i, ii;
  Instance *I;
  double z, lnsZ, bestUb;

  Professor *candProfessors;
  int nCandsProfessors, capacRes, toRemove, perda, nRemoved;
#ifdef DEBUG_LNS
  int infeasible;
  unsigned int status;
#endif
  found = 0;
#ifdef DEBUG_LNS
  infeasible = 0;
  PRINTFLNS("\n============== New lns heur at node: %lld\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
#endif

  probdata = SCIPgetProbData(scip);
  assert(probdata != NULL);
  vars           = SCIPprobdataGetVars(probdata);
  I              = SCIPprobdataGetInstance(probdata);
  candProfessors = (Professor *) calloc(I->nProfessors, sizeof(Professor));
  for (int k = 0; k < I->nProfessors; k++)
    candProfessors[k].name[0] = '\0';

  fixed            = (int *) calloc(I->nProfessors * I->nCourses, sizeof(int));
  nCandsProfessors = 0;

  // first, select all variables already fixed in 1.0
  for (int j = 0; j < I->nProfessors; j++)
  {
    for (int i = 0; i < I->nCourses; i++)
    {
      int idx      = (i * I->nProfessors) + j;

      var          = vars[idx];
      SCIP_Real lb = SCIPvarGetLbLocal(var);
      SCIP_Real ub = SCIPvarGetUbLocal(var);

      if (lb > 1.0 - EPSILON)
      {
        // x_ij is fixed at 1.0 (assigned)
        PRINTFLNS("Assignment x_%d_%d fixed at 1.0 (Course: %s, Professsor: %s)", j, i, I->courses[i].subject.name, I->professors[j].name);
        fixed[idx] = 1;
      }
      else if (ub < EPSILON)
      {
        // x_ij is fixed at 0.0 (not assigned)
        // PRINTFLNS("Assignment x_%d_%d fixed at 0.0 (Course: %s, Professor: %s)", j, i, I->courses[i].subject.name, I->professors[j].name);
        fixed[idx] = -1;
      }
    }
  }

  for (int j = 0; j < I->nProfessors; j++)
  {
    for (int i = 0; i < I->nCourses; i++)
    {
      int idx       = (i * I->nProfessors) + j;
      SCIP_Real val = SCIPgetSolVal(scip, initsol, vars[idx]);
      if (val > EPSILON && !fixed[i])
      {
        // x_ij is fixed at 1.0 (assigned)
        fixed[idx] = 1;

        if (candProfessors[nCandsProfessors].name[0] == '\0')
        {
          // First assignment for this professor
          candProfessors[nCandsProfessors++] = I->professors[j];
        }
      }
    }
  }

  // order candidate professors by avgPreferenceWeight biggest to smallest
  qsort(candProfessors, nCandsProfessors, sizeof(Professor), compareProfessors);

  // Obs: nCandsProfessors is smaller than I->nProfessors
  toRemove = nCandsProfessors * (param.lns_perc);
  for (int i = nCandsProfessors - 1; i >= toRemove; i--)
  {
    Professor *prof = &candProfessors[i];
    for (int j = 0; j < I->nCourses; j++)
    {
      Course *course = prof->preferences[j].course_ptr;
      int idx        = (course->label * I->nProfessors) + prof->label;
      fixed[idx]     = 0;
    }
  }

  lnsparam.time_limit      = param.lns_time;
  lnsparam.display_freq    = param.display_freq;
  lnsparam.area_penalty    = param.area_penalty;
  lnsparam.nodes_limit     = -1;
  lnsparam.heur_rounding   = 0;
  lnsparam.heur_bad_sol    = 0;
  lnsparam.heur_lns        = 0;
  lnsparam.heur_gulosa     = 0;

  static int subscip_count = 0;
  PRINTFLNS("Creating subscip instance #%d", ++subscip_count);

  SCIP *subscip        = NULL;
  SCIP_RETCODE retcode = configScip(&subscip, lnsparam);
  if (retcode != SCIP_OKAY)
  {
    PRINTFLNS("\nError creating subscip instance for LNS heuristic");
    getchar();  // Wait for user input to see the message
  }

  //   /* disable output to console */

  // load problem into scip
  if (!loadProblem(subscip, "lns", I, 0, fixed))
  {
    printf("\nProblem to load instance problem\n");
    free(fixed);
    free(candProfessors);

    return -1;
  }
  probdata2 = SCIPgetProbData(subscip);
  assert(probdata2 != NULL);

  vars2 = SCIPprobdataGetVars(probdata2);

  SCIP_CALL(SCIPsetIntParam(subscip, "display/verblevel", 4));

  // Use global output_folder for all output files
  char lns_lp_path[256];
  snprintf(lns_lp_path, sizeof(lns_lp_path), "%s/lns.lp", output_path);

  PRINTFLNS("Writing LNS problem to %s", lns_lp_path);
  SCIP_CALL(SCIPwriteOrigProblem(subscip, lns_lp_path, "lp", TRUE));

  SCIP_CALL(SCIPsolve(subscip));
  // Check if subSCIP is infeasible
  SCIP_STATUS subscip_status = SCIPgetStatus(subscip);
  if (subscip_status == SCIP_STATUS_INFEASIBLE)
  {
    PRINTFLNS("SubSCIP is infeasible for the current neighborhood.");
    getchar();  // Wait for user input to see the message
  }

#ifdef DEBUG_LNS
  SCIP_CALL(SCIPprintBestSol(subscip, NULL, FALSE));
  status = SCIPgetStatus(subscip);
  printf("\nstatus=%d", status);
#endif

  // Recupera solucao
  lnsSol = SCIPgetBestSol(subscip);
  lnsZ   = SCIPgetPrimalbound(subscip);  // o.f. for the solution found by LNS
  z      = -SCIPsolGetOrigObj(initsol);  // objective function for the initial solution

  PRINTFLNS("\nLNS solution found with objective value: %lf", lnsZ);
  PRINTFLNS("Initial solution objective value: %lf", z);

  // --- ADD: Transfer LNS solution to main SCIP only if better ---
  if (lnsSol != NULL && lnsZ > z + EPSILON)
  {
    SCIP_CALL(SCIPcreateSol(scip, &sol, heur));
    for (int idx = 0; idx < I->nProfessors * I->nCourses; idx++)
    {
      SCIP_Real val = SCIPgetSolVal(subscip, lnsSol, vars2[idx]);
      if (val > EPSILON)
        SCIP_CALL(SCIPsetSolVal(scip, sol, vars[idx], val));
    }

    // check if the solution found by LNS is better than the current bestsolution
    bestUb = SCIPgetPrimalbound(scip);
    if (lnsZ > bestUb + EPSILON)
    {
      SCIP_Bool stored = FALSE;
      SCIP_CALL(SCIPtrySolMine(scip, sol, TRUE, TRUE, TRUE, TRUE, &stored));
      if (stored)
      {
        PRINTFLNS("LNS solution transferred and accepted in main SCIP!\n");
        found = 1;
      }
      else
      {
        PRINTFLNS("LNS solution transferred but NOT accepted in main SCIP!\n");
      }
    }
  }

  // Free all allocated memory before freeing subscip
  free(fixed);
  free(candProfessors);
  if (subscip != NULL)
    SCIP_CALL(SCIPfree(&subscip));

  return found;
}

/** execution method of primal heuristic */
static SCIP_DECL_HEUREXEC(heurExecLns)
{                /*lint --e{715}*/
  SCIP_SOL *sol; /**< solution to improve */
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

  sol = SCIPgetBestSol(scip);
  if (sol == NULL)
    return SCIP_OKAY;
  /* solve lns */
  if (lns(scip, sol, heur))
  {
    *result = SCIP_FOUNDSOL;
  }
  else
  {
    *result = SCIP_DIDNOTFIND;
#ifdef DEBUG_PRIMAL
    printf("\nLns could not find feasible solution!");
#endif
  }
  return SCIP_OKAY;
}

/*
 * primal heuristic specific intelnsace methods
 */

/** creates the lns_crtp primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPincludeHeurLns(
        SCIP *scip /**< SCIP data structure */
)
{
  SCIP_HEURDATA *heurdata;
  SCIP_HEUR *heur;

  /* create lns primal heuristic data */
  heurdata = NULL;

  heur     = NULL;

  /* include primal heuristic */
#if 0
   /* use SCIPincludeHeur() if you want to set all callbacks explicitly and realize (by getting compiler errors) when
    * new callbacks are added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeHeur(scip, HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, HEUR_FREQ, HEUR_FREQOFS,
         HEUR_MAXDEPTH, HEUR_TIMING, HEUR_USESSUBSCIP,
         heurCopyLns, heulnsreeLns, heurInitLns, heurExitLns, heurInitsolLns, heurExitsolLns, heurExecLns,
         heurdata) );
#else
  /* use SCIPincludeHeurBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
  SCIP_CALL(SCIPincludeHeurBasic(scip, &heur,
                                 HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, HEUR_FREQ, HEUR_FREQOFS,
                                 HEUR_MAXDEPTH, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecLns, heurdata));

  assert(heur != NULL);

  /* set non fundamental callbacks via setter functions */
  SCIP_CALL(SCIPsetHeurCopy(scip, heur, heurCopyLns));
  SCIP_CALL(SCIPsetHeurFree(scip, heur, heurFreeLns));
  SCIP_CALL(SCIPsetHeurInit(scip, heur, heurInitLns));
  SCIP_CALL(SCIPsetHeurExit(scip, heur, heurExitLns));
  SCIP_CALL(SCIPsetHeurInitsol(scip, heur, heurInitsolLns));
  SCIP_CALL(SCIPsetHeurExitsol(scip, heur, heurExitsolLns));
#endif

  /* add lns primal heuristic parameters */
  /* TODO: (optional) add primal heuristic specific parameters with SCIPaddTypeParam() here */

  return SCIP_OKAY;
}
