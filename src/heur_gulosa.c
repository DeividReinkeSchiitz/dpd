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

#include "heur_gulosa.h"
#include "heur_problem.h"
#include "parameters_dpd.h"
#include "probdata_dpd.h"

#define DEBUG_GULOSA 1
/* configuracao da heuristica */
#define HEUR_NAME "gulosa"
#define HEUR_DESC "gulosa primal heuristic template"
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
static SCIP_DECL_HEURCOPY(heurCopyGulosa)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static SCIP_DECL_HEURFREE(heurFreeGulosa)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}


/** initialization method of primal heuristic (called after problem was transformed) */
static SCIP_DECL_HEURINIT(heurInitGulosa)
{ /*lint --e{715}*/


  return SCIP_OKAY;
}


/** deinitialization method of primal heuristic (called before transformed problem is freed) */
static SCIP_DECL_HEUREXIT(heurExitGulosa)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}


/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static SCIP_DECL_HEURINITSOL(heurInitsolGulosa)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}


/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static SCIP_DECL_HEUREXITSOL(heurExitsolGulosa)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}


/**
 * @brief Core of the gulosa heuristic: it builds one solution for the problem by gulosa procedure.
 *
 * @param scip problem
 * @param sol pointer to the solution structure where the solution wil be saved
 * @param heur pointer to the gulosa heuristic handle (to contabilize statistics)
 * @return int 1 if solutions is found, 0 otherwise.
 */
int gulosa(SCIP *scip, SCIP_SOL **sol, SCIP_HEUR *heur)
{
  int found, infeasible, nInSolution;
  unsigned int stored;
  int nvars;
  int *covered, n, custo, nCovered;
  SCIP_VAR *var, **solution, **varlist;
  //  SCIP* scip_cp;
  SCIP_Real valor, bestUb;
  SCIP_PROBDATA *probdata;
  int i, residual;
  instanceT *I;

  found      = 0;
  infeasible = 0;

#ifdef DEBUG_GULOSA
  printf("\n============== New gulosa heur at node: %lld\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
#endif

  /* recover the problem data*/
  probdata = SCIPgetProbData(scip);
  assert(probdata != NULL);

  nvars       = SCIPprobdataGetNVars(probdata);
  varlist     = SCIPprobdataGetVars(probdata);
  I           = SCIPprobdataGetInstance(probdata);
  n           = I->n;

  solution    = (SCIP_VAR **) malloc(sizeof(SCIP_VAR *) * n);
  covered     = (int *) calloc(n, sizeof(int));
  nInSolution = 0;
  nCovered    = 0;
  custo       = 0;
  residual    = I->C[0];

#ifdef DEBUG_GULOSA
  printf("\nGULOSA: nvars=%d n=%d residual=%d\n", nvars, n, residual);
#endif

  //     // first, select all variables already fixed in 1.0
  //     for (i = 0; i < nvars; i++)
  //     {
  //         var = varlist[i];
  //         if (SCIPvarGetLbLocal(var) > 1.0 - EPSILON)
  //         {  // var >= 1.0
  //             solution[nInSolution++] = var;
  //             // update residual capacity
  //             //residual -= I->item[i].weight; comented
  //             covered[i]              = 1;
  //             nCovered++;
  //             //custo += I->item[i].value; comented
  //             infeasible = residual < 0 ? 1 : 0;
  // #ifdef DEBUG_GULOSA
  //             printf("\nSelected fixed var= %s. TotalItems=%d value=%d residual=%d infeasible=%d", SCIPvarGetName(var), nInSolution, custo, residual, infeasible);
  // #endif
  //         }
  //         else
  //         {  // discard items fixed in 0.0
  //             if (SCIPvarGetUbLocal(var) < EPSILON)
  //             {  // var fixed in 0.0
  //                 covered[i] = 1;
  //                 nCovered++;
  //             }
  //         }
  //     }


  // complete solution using items not fixed (not covered)
  for (i = 0; i < n && nCovered < n && residual > 0; i++)
  {
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
  if (!infeasible)
  {
    /* create SCIP solution structure sol */
    SCIP_CALL(SCIPcreateSol(scip, sol, heur));
    // save found solution in sol
    for (i = 0; i < nInSolution; i++)
    {
      var = solution[i];
      SCIP_CALL(SCIPsetSolVal(scip, *sol, var, 1.0));
    }
    valor  = custo;  //createSolution(scip, *sol, solution, nInSolution, &infeasible, covered);
    bestUb = SCIPgetPrimalbound(scip);
#ifdef DEBUG_GULOSA
    printf("\nFound solution...\n");
    //      SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
    printf("\ninfeasible=%d value = %lf > bestUb = %lf? %d\n\n", infeasible, valor, bestUb, valor > bestUb + EPSILON);
#endif
    if (!infeasible && valor > bestUb + EPSILON)
    {
#ifdef DEBUG_GULOSA
      printf("\nBest solution found...\n");
      SCIP_CALL(SCIPprintSol(scip, *sol, NULL, FALSE));
#endif

      /* check if the solution is feasible */
      SCIP_CALL(SCIPtrySolMine(scip, *sol, TRUE, TRUE, FALSE, TRUE, &stored));
      if (stored)
      {
#ifdef DEBUG_PRIMAL
        printf("\nSolution is feasible and was saved! Total of items = %d", nInSolution);
        SCIPdebugMessage("found feasible gulosa solution:\n");
        SCIP_CALL(SCIPprintSol(scip, *sol, NULL, FALSE));
#endif
        //       *result = SCIP_FOUNDSOL;
      }
      else
      {
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
static SCIP_DECL_HEUREXEC(heurExecGulosa)
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

  /* solve gulosa */
  if (gulosa(scip, &sol, heur))
  {
    *result = SCIP_FOUNDSOL;
  }
  else
  {
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
        SCIP *scip /**< SCIP data structure */
)
{
  SCIP_HEURDATA *heurdata;
  SCIP_HEUR *heur;

  /* create gulosa primal heuristic data */
  heurdata = NULL;

  heur     = NULL;

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
  SCIP_CALL(SCIPincludeHeurBasic(scip, &heur,
                                 HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, param.heur_round_freq, param.heur_round_freqofs,
                                 param.heur_round_maxdepth, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecGulosa, heurdata));

  assert(heur != NULL);

  /* set non fundamental callbacks via setter functions */
  SCIP_CALL(SCIPsetHeurCopy(scip, heur, heurCopyGulosa));
  SCIP_CALL(SCIPsetHeurFree(scip, heur, heurFreeGulosa));
  SCIP_CALL(SCIPsetHeurInit(scip, heur, heurInitGulosa));
  SCIP_CALL(SCIPsetHeurExit(scip, heur, heurExitGulosa));
  SCIP_CALL(SCIPsetHeurInitsol(scip, heur, heurInitsolGulosa));
  SCIP_CALL(SCIPsetHeurExitsol(scip, heur, heurExitsolGulosa));
#endif

  /* add gulosa primal heuristic parameters */
  /* TODO: (optional) add primal heuristic specific parameters with SCIPaddTypeParam() here */

  return SCIP_OKAY;
}
