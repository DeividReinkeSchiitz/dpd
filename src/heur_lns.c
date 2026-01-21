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
#include <string.h>

extern char output_path[512];

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
#define HEUR_PRIORITY 1                     /**< heuristics of high priorities are called first */
#define HEUR_FREQ 10                          /**< heuristic call frequency. 1 = in all levels of the B&B tree */
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
struct SCIP_HeurData
{
  SCIP *subscip;             /**< sub-SCIP instance for LNS */
  int *fixed;                /**< array to store fixed variables */
  int *fixedCourse;          /**< array to store fixed courses */
  LNS_Candidate *candidates; /**< array for LNS candidates */
  int arraySize;             /**< size of allocated arrays */
  SCIP_Bool initialized;     /**< whether the data structures are initialized */
  SCIP_Real lastSolValue;    /**< objective value of the last processed solution */
};
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
  SCIP_HEURDATA *heurdata;

  heurdata = SCIPheurGetData(heur);
  assert(heurdata != NULL);

  /* free sub-SCIP instance */
  if (heurdata->subscip != NULL)
  {
    SCIP_CALL(SCIPfree(&heurdata->subscip));
  }

  /* free allocated arrays */
  if (heurdata->fixed != NULL)
    SCIPfreeMemoryArray(scip, &heurdata->fixed);

  if (heurdata->fixedCourse != NULL)
    SCIPfreeMemoryArray(scip, &heurdata->fixedCourse);

  if (heurdata->candidates != NULL)
    SCIPfreeMemoryArray(scip, &heurdata->candidates);

  /* free heuristic data */
  SCIPfreeMemory(scip, &heurdata);
  SCIPheurSetData(heur, NULL);

  return SCIP_OKAY;
}

/** initialization method of primal heuristic (called after problem was transformed) */
static SCIP_DECL_HEURINIT(heurInitLns)
{ /*lint --e{715}*/
  SCIP_HEURDATA *heurdata;
  SCIP_PROBDATA *probdata;
  Instance *instance;

  heurdata = SCIPheurGetData(heur);
  assert(heurdata != NULL);

  if (heurdata->initialized)
    return SCIP_OKAY;

  /* get problem data to determine array sizes */
  probdata = SCIPgetProbData(scip);
  if (probdata == NULL)
    return SCIP_OKAY;

  instance = SCIPprobdataGetInstance(probdata);
  if (instance == NULL)
    return SCIP_OKAY;

  /* calculate array size */
  heurdata->arraySize = instance->nProfessors * instance->nCourses;

  /* allocate memory for arrays */
  SCIP_CALL(SCIPallocMemoryArray(scip, &heurdata->fixed, heurdata->arraySize));
  SCIP_CALL(SCIPallocMemoryArray(scip, &heurdata->fixedCourse, instance->nCourses));
  SCIP_CALL(SCIPallocMemoryArray(scip, &heurdata->candidates, instance->nCourses));

  /* Create sub-SCIP instance for reuse */
  parametersT lnsparam;
  lnsparam.time_limit    = param.lns_time;
  lnsparam.display_freq  = param.display_freq;
  lnsparam.area_penalty  = param.area_penalty;
  lnsparam.nodes_limit   = -1;
  lnsparam.heur_rounding = 0;
  lnsparam.heur_bad_sol  = 0;
  lnsparam.heur_lns      = 0;
  lnsparam.heur_grasp    = 0;

  SCIP_RETCODE retcode   = configScip(&heurdata->subscip, lnsparam);
  /* Apply basic SCIP parameters for sub-problem */

  SCIP_CALL(SCIPsetIntParam(heurdata->subscip, "presolving/maxrounds", 0));
  SCIP_CALL(SCIPsetIntParam(heurdata->subscip, "display/verblevel", 0));

  // /* Stop after finding the first feasible solution */
  // SCIP_CALL(SCIPsetIntParam(heurdata->subscip, "limits/solutions", 3));

  if (!loadProblemSubSCIP(heurdata->subscip, "lns", instance, 0, heurdata->fixed))
  {
    printf("\nProblem to load instance problem\n");
    getchar();
    return -1;
  }

  if (retcode != SCIP_OKAY)
  {
    PRINTFLNS("\nError creating subscip instance for LNS heuristic");
    getchar();  // Wait for user input to see the message
    return retcode;
  };

  heurdata->initialized = TRUE;
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
  // Only run LNS if there is already a feasible solution
  if (SCIPgetNSols(scip) == 0)
  {
    PRINTFLNS("LNS skipped: no feasible solution available.");
    getchar();  // Wait for user input to see the message
    return 0;
  }

  PRINTFLNS("\nLNS heuristic called.\n");

  SCIP_HEURDATA *heurdata;

  int found;
  int *fixed, *fixedCourse;
  SCIP_VAR *var, **vars, **vars2;
  LNS_Candidate *cand;
  SCIP_PROBDATA *probdata, *probdata2;
  SCIP_SOL *lnsSol, *sol;
  Instance *I;
  double z, lnsZ, bestUb;
  SCIP_Real val;

  int nCands, toRemove, idx;
#ifdef DEBUG_LNS
  unsigned int status;
#endif
  found = 0;
#ifdef DEBUG_LNS
  PRINTFLNS("\n============== New lns heur at node: %lld\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
#endif

  /* get heuristic data */
  heurdata = SCIPheurGetData(heur);
  assert(heurdata != NULL);
  assert(heurdata->initialized);

  probdata = SCIPgetProbData(scip);
  assert(probdata != NULL);
  vars        = SCIPprobdataGetVars(probdata);
  I           = SCIPprobdataGetInstance(probdata);
  z           = -SCIPsolGetOrigObj(initsol);  // objective function for the initial solution

  /* use pre-allocated arrays from heuristic data */
  fixed       = heurdata->fixed;
  fixedCourse = heurdata->fixedCourse;
  cand        = heurdata->candidates;
  nCands      = 0;

  /* reset arrays for this iteration */
  memset(fixed, 0, heurdata->arraySize * sizeof(int));
  memset(fixedCourse, 0, I->nCourses * sizeof(int));
  memset(cand, 0, I->nCourses * sizeof(LNS_Candidate));

  // Apply safe global bound fixings: keep good choices from presolve, avoid local-node artifacts
  for (int i = 0; i < I->nCourses; i++)
  {
    for (int j = 0; j < I->nProfessors; j++)
    {
      idx           = (j * I->nCourses) + i;
      SCIP_Real lbg = SCIPvarGetLbGlobal(vars[idx]);
      SCIP_Real ubg = SCIPvarGetUbGlobal(vars[idx]);

      if (lbg > 1.0 - EPSILON)
      {
        fixed[idx]     = 1;  // globally fixed to 1 is safe
        fixedCourse[i] = 1;  // course i is fixed (assigned)
      }
      else if (ubg < EPSILON)
      {
        fixed[idx]     = -1;  // forbid this assignment, but do not mark course as fixed
        fixedCourse[i] = 1;   // course i is fixed (assigned)
      }
    }
  }

  Professor *cur_professor;
  Preference *cur_pref;

  int carga_s1[I->nProfessors];
  int carga_s2[I->nProfessors];

  for (int i = 0; i < I->nProfessors; i++)
  {
    carga_s1[i] = 0;
    carga_s2[i] = 0;
  }

  for (int i = 0; i < I->nCourses; i++)
  {
    if (fixedCourse[i] == 1)
      continue;

    for (int j = 0; j < I->nProfessors; j++)
    {
      cur_professor = &I->professors[j];
      cur_pref      = &cur_professor->preferences[i];
      idx           = (j * I->nCourses) + i;

      val           = SCIPgetSolVal(scip, initsol, vars[idx]);
      if (val > EPSILON && fixed[idx] == 0)
      {

        if (cur_pref->course_ptr->semester == 1)
        {
          if (carga_s1[j] + cur_pref->course_ptr->workload > cur_professor->maxWorkload1)
            continue;
          carga_s1[j] += cur_pref->course_ptr->workload;
        }
        else if (cur_pref->course_ptr->semester == 2)
        {
          if (carga_s2[j] + cur_pref->course_ptr->workload > cur_professor->maxWorkload2)
            continue;
          carga_s2[j] += cur_pref->course_ptr->workload;
        }

        fixed[idx]                 = 1;
        cand[nCands].course_ptr    = cur_pref->course_ptr;
        cand[nCands].professor_ptr = cur_professor;
        nCands++;
        PRINTFLNS("Adding candidate assignment x_%d_%d (Course: %s, Professor: %s) - idx: %d - value: %.2f", j, i, cur_pref->course_ptr->subject.name, cur_professor->name, idx, val);
        break;  // only one assignment per course
      }
    }
  }

  // print all candidates
  PRINTFLNS("Candidates found:");
  for (int i = 0; i < nCands; i++)
  {
    LNS_Candidate *cur_cand = &cand[i];
    Course *course          = cur_cand->course_ptr;
    Professor *prof         = cur_cand->professor_ptr;
    PRINTFLNS("Candidate x_%d_%d (Course: %s, Professor: %s)", prof->label, course->label, course->subject.name, prof->name);
  }

  // order candidate professors by avgPreferenceWeight according to lns_order parameter
  if (strcmp(param.lns_order, "crescente") == 0)
  {
    PRINTFLNS("Ordering candidates in ASCENDING order (crescente)");
    qsort(cand, nCands, sizeof(LNS_Candidate), compareCandidatesAscending);
  }
  else
  {
    PRINTFLNS("Ordering candidates in DESCENDING order (decrescente)");
    qsort(cand, nCands, sizeof(LNS_Candidate), compareCandidates);
  }

  // print ordered candidates
  PRINTFLNS("Ordered candidates:");
  for (int i = 0; i < nCands; i++)
  {
    LNS_Candidate *cur_cand = &cand[i];
    Course *course          = cur_cand->course_ptr;
    Professor *prof         = cur_cand->professor_ptr;
    idx                     = prof->label * I->nCourses + course->label;
    PRINTFLNS("Candidate x_%d_%d (Course: %s, Professor: %s, avgPreferenceWeight: %.2f) - idx: %d", prof->label, course->label, course->subject.name, prof->name, prof->avgPreferenceWeight, idx);
  }

  // Obs: nCandsProfessors is smaller than I->nProfessors
  toRemove = nCands * (param.lns_perc);
  PRINTFLNS("Number of candidate assignments: %d", nCands);
  PRINTFLNS("Number of Courses to remove: %d", toRemove);

  for (int j = 0; j < toRemove; j++)
  {
    LNS_Candidate *cur_cand = &cand[j];
    Course *course          = cur_cand->course_ptr;
    Professor *prof         = cur_cand->professor_ptr;
    idx                     = prof->label * I->nCourses + course->label;

    PRINTFLNS("Removing candidate assignment x_%d_%d (Course: %s, Professor: %s, avgPreferenceWeight: %.2f) - idx: %d", prof->label, course->label, course->subject.name, prof->name, prof->avgPreferenceWeight, idx);
    fixed[idx] = 0;  // free this assignment in the neighborhood
  }

  probdata2 = SCIPgetProbData(heurdata->subscip);
  assert(probdata2 != NULL);
  vars2 = SCIPprobdataGetVars(probdata2);

  /* Set the cutoff bound: stop if we find a solution better than current + small epsilon */
  SCIP_CALL(SCIPsetObjlimit(heurdata->subscip, z + EPSILON));
  PRINTFLNS("Sub-SCIP objective limit set to: %.6f (will stop when finding better solution)", z + EPSILON);

#if DEBUG_LNS
  /* Override display level for debug mode */
  SCIP_CALL(SCIPsetIntParam(heurdata->subscip, "display/verblevel", 4));
#endif

#if DEBUG_LNS
  // Write LP only in debug to avoid heavy I/O in production
  char lns_lp_path[600];
  snprintf(lns_lp_path, sizeof(lns_lp_path), "%s/lns.lp", output_path);
  PRINTFLNS("Writing LNS problem to %s", lns_lp_path);
  SCIP_CALL(SCIPwriteOrigProblem(heurdata->subscip, lns_lp_path, "lp", TRUE));
#endif

  SCIP_CALL(SCIPsolve(heurdata->subscip));
  // Check if subSCIP is infeasible
  SCIP_STATUS subscip_status = SCIPgetStatus(heurdata->subscip);
  if (subscip_status == SCIP_STATUS_INFEASIBLE)
  {
    PRINTFLNS("SubSCIP is infeasible for the current neighborhood.");
#if DEBUG_LNS
    getchar();  // Wait for user input to see the message (debug only)
    exit(1);    // In debug mode, exit to check why subscip is infeasible
#endif
  }

#ifdef DEBUG_LNS
  SCIP_CALL(SCIPprintBestSol(heurdata->subscip, NULL, FALSE));
  status = SCIPgetStatus(heurdata->subscip);
  printf("\nstatus=%d", status);
#endif

  // Recupera solucao
  lnsSol = SCIPgetBestSol(heurdata->subscip);
  lnsZ   = SCIPgetPrimalbound(heurdata->subscip);  // o.f. for the solution found by LNS

  PRINTFLNS("\nLNS solution found with objective value: %lf", lnsZ);
  PRINTFLNS("Initial solution objective value: %lf\n", z);

  // --- ADD: Transfer LNS solution to main SCIP only if better ---
  if (lnsSol != NULL && lnsZ >= z + EPSILON)
  {
    SCIP_CALL(SCIPcreateSol(scip, &sol, heur));
    for (int idx = 0; idx < I->nProfessors * I->nCourses; idx++)
    {
      SCIP_Real val = SCIPgetSolVal(heurdata->subscip, lnsSol, vars2[idx]);
      if (val > EPSILON)
        SCIP_CALL(SCIPsetSolVal(scip, sol, vars[idx], val));
    }

    // check if the solution found by LNS is better than the current bestsolution
    bestUb = SCIPgetPrimalbound(scip);
    if (lnsZ >= bestUb + EPSILON)
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
  else
  {
    PRINTFLNS("LNS solution %lf not better than initial solution %lf, not transferring to main SCIP.", lnsZ, z + EPSILON);
  }

  /* Arrays and sub-SCIP instance are managed by heuristic data and will be reused */
  /* No cleanup needed here - everything will be freed in heurFreeLns */

  return found;
}

/** execution method of primal heuristic */
static SCIP_DECL_HEUREXEC(heurExecLns)
{                /*lint --e{715}*/
  SCIP_SOL *sol; /**< solution to improve */
  SCIP_HEURDATA *heurdata;
  SCIP_Real currentSolValue;
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

  /* get heuristic data */
  heurdata = SCIPheurGetData(heur);
  assert(heurdata != NULL);

  /* get current solution objective value */
  currentSolValue = -SCIPsolGetOrigObj(sol);

  /* check if solution value has changed since last LNS execution */
  if (SCIPisEQ(scip, currentSolValue, heurdata->lastSolValue))
  {
    PRINTFLNS("LNS skipped: solution value has not changed (current: %.6f, last: %.6f)",
              currentSolValue, heurdata->lastSolValue);
    *result = SCIP_DIDNOTRUN;
    return SCIP_OKAY;
  }

  PRINTFLNS("LNS executing: solution value changed from %.6f to %.6f",
            heurdata->lastSolValue, currentSolValue);

  /* update last solution value */
  heurdata->lastSolValue = currentSolValue;
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
  SCIP_CALL(SCIPallocMemory(scip, &heurdata));
  heurdata->subscip      = NULL;
  heurdata->fixed        = NULL;
  heurdata->fixedCourse  = NULL;
  heurdata->candidates   = NULL;
  heurdata->arraySize    = 0;
  heurdata->initialized  = FALSE;
  heurdata->lastSolValue = -SCIPinfinity(scip); /* Initialize with -infinity */

  heur                   = NULL;

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
