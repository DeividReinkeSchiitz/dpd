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

/**@file   heur_badFeasible.c
 * @brief  badFeasible primal heuristic
 * @author Edna Hoshino (based on template provided by Tobias Achterberg)
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "heur_badFeasible.h"
#include "heur_problem.h"
#include "parameters_dpd.h"
#include "probdata_dpd.h"

// Add necessary includes for file operations
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>  // For malloc, realloc, free
#include <string.h>

// Corrected definition of FEASIBLE_SOLUTION

const char *FEASIBLE_SOLUTION[]      = {"x_0_24",
                                        "x_0_90",
                                        "x_0_105",
                                        "x_0_151",
                                        "x_1_52",
                                        "x_1_86",
                                        "x_1_113",
                                        "x_1_118",
                                        "x_1_140",
                                        "x_1_145",
                                        "x_1_184",
                                        "x_3_17",
                                        "x_3_39",
                                        "x_3_45",
                                        "x_3_222",
                                        "x_4_117",
                                        "x_4_172",
                                        "x_4_207",
                                        "x_4_223",
                                        "x_5_64",
                                        "x_5_68",
                                        "x_5_85",
                                        "x_5_132",
                                        "x_5_171",
                                        "x_6_98",
                                        "x_6_114",
                                        "x_6_193",
                                        "x_6_197",
                                        "x_7_34",
                                        "x_7_125",
                                        "x_7_211",
                                        "x_7_215",
                                        "x_8_3",
                                        "x_8_91",
                                        "x_8_92",
                                        "x_8_93",
                                        "x_8_170",
                                        "x_9_23",
                                        "x_9_33",
                                        "x_9_141",
                                        "x_9_181",
                                        "x_12_49",
                                        "x_12_158",
                                        "x_12_175",
                                        "x_12_183",
                                        "x_13_67",
                                        "x_13_153",
                                        "x_13_173",
                                        "x_13_194",
                                        "x_13_198",
                                        "x_14_30",
                                        "x_14_37",
                                        "x_14_54",
                                        "x_14_109",
                                        "x_14_196",
                                        "x_15_8",
                                        "x_15_29",
                                        "x_15_102",
                                        "x_15_107",
                                        "x_15_160",
                                        "x_16_26",
                                        "x_16_94",
                                        "x_16_97",
                                        "x_16_139",
                                        "x_17_18",
                                        "x_17_56",
                                        "x_17_71",
                                        "x_17_179",
                                        "x_19_38",
                                        "x_19_121",
                                        "x_19_130",
                                        "x_19_133",
                                        "x_20_73",
                                        "x_20_131",
                                        "x_20_142",
                                        "x_20_148",
                                        "x_21_27",
                                        "x_21_48",
                                        "x_21_136",
                                        "x_21_218",
                                        "x_22_19",
                                        "x_22_87",
                                        "x_22_120",
                                        "x_22_147",
                                        "x_22_165",
                                        "x_22_203",
                                        "x_23_25",
                                        "x_23_32",
                                        "x_23_51",
                                        "x_23_138",
                                        "x_23_152",
                                        "x_24_157",
                                        "x_24_176",
                                        "x_27_21",
                                        "x_27_31",
                                        "x_27_50",
                                        "x_27_57",
                                        "x_27_81",
                                        "x_27_189",
                                        "x_28_101",
                                        "x_28_146",
                                        "x_28_210",
                                        "x_29_2",
                                        "x_29_115",
                                        "x_29_156",
                                        "x_30_43",
                                        "x_30_70",
                                        "x_30_89",
                                        "x_30_128",
                                        "x_31_14",
                                        "x_31_35",
                                        "x_31_112",
                                        "x_31_163",
                                        "x_32_116",
                                        "x_32_178",
                                        "x_33_12",
                                        "x_33_61",
                                        "x_33_166",
                                        "x_33_200",
                                        "x_34_4",
                                        "x_34_20",
                                        "x_34_96",
                                        "x_34_167",
                                        "x_34_205",
                                        "x_37_103",
                                        "x_37_144",
                                        "x_38_41",
                                        "x_38_66",
                                        "x_38_119",
                                        "x_38_154",
                                        "x_38_155",
                                        "x_39_59",
                                        "x_39_164",
                                        "x_39_202",
                                        "x_39_217",
                                        "x_40_44",
                                        "x_40_62",
                                        "x_40_186",
                                        "x_40_191",
                                        "x_41_15",
                                        "x_41_79",
                                        "x_41_95",
                                        "x_42_7",
                                        "x_42_9",
                                        "x_42_58",
                                        "x_42_99",
                                        "x_42_161",
                                        "x_42_180",
                                        "x_42_187",
                                        "x_44_88",
                                        "x_44_111",
                                        "x_44_162",
                                        "x_44_204",
                                        "x_44_214",
                                        "x_45_143",
                                        "x_45_195",
                                        "x_46_53",
                                        "x_46_124",
                                        "x_46_129",
                                        "x_46_206",
                                        "x_47_82",
                                        "x_47_83",
                                        "x_47_150",
                                        "x_48_5",
                                        "x_48_28",
                                        "x_48_46",
                                        "x_48_75",
                                        "x_48_76",
                                        "x_49_84",
                                        "x_49_177",
                                        "x_49_221",
                                        "x_50_11",
                                        "x_50_36",
                                        "x_50_80",
                                        "x_50_134",
                                        "x_51_108",
                                        "x_51_127",
                                        "x_51_135",
                                        "x_51_169",
                                        "x_52_42",
                                        "x_52_55",
                                        "x_52_72",
                                        "x_52_137",
                                        "x_53_10",
                                        "x_53_16",
                                        "x_53_104",
                                        "x_53_174",
                                        "x_53_199",
                                        "x_53_201",
                                        "x_54_60",
                                        "x_54_63",
                                        "x_54_110",
                                        "x_54_123",
                                        "x_54_126",
                                        "x_54_212",
                                        "x_54_219",
                                        "x_55_22",
                                        "x_55_106",
                                        "x_55_213",
                                        "x_56_40",
                                        "x_56_69",
                                        "x_56_209",
                                        "x_56_216",
                                        "x_57_1",
                                        "x_57_6",
                                        "x_57_65",
                                        "x_57_100",
                                        "x_57_188",
                                        "x_58_0",
                                        "x_58_13",
                                        "x_58_78",
                                        "x_58_149",
                                        "x_58_168",
                                        "x_58_190",
                                        "x_58_192",
                                        "x_58_220",
                                        "x_59_122",
                                        "x_59_159",
                                        "x_59_182",
                                        "x_59_185",
                                        "x_60_47",
                                        "x_60_74",
                                        "x_60_77",
                                        "x_60_208"};

// Calculate the number of elements in FEASIBLE_SOLUTION
const int NUM_FEASIBLE_SOLUTION_VARS = sizeof(FEASIBLE_SOLUTION) / sizeof(FEASIBLE_SOLUTION[0]);

#define DEBUG_BAD_FEASIBLE 1
/* configuracao da heuristica */
#define HEUR_NAME "badFeasibleSolution"
#define HEUR_DESC "primal heuristic template"
#define HEUR_DISPCHAR 'b'
#define HEUR_PRIORITY 3                       /**< heuristics of high priorities are called first */
#define HEUR_FREQ 1                           /**< heuristic call frequency. 1 = in all levels of the B&B tree */
#define HEUR_FREQOFS 0                        /**< starts of level 0 (root node) */
#define HEUR_MAXDEPTH 2                       /**< maximal level to be called. -1 = no limit */
#define HEUR_TIMING SCIP_HEURTIMING_AFTERNODE /**< when the heuristic should be called? SCIP_HEURTIMING_DURINGLPLOOP or SCIP_HEURTIMING_AFTERNODE */
#define HEUR_USESSUBSCIP TRUE                 /**< does the heuristic use a secondary SCIP instance? */

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*
 * Global variables for heuristic data
 */
static char **g_heuristic_solution_var_names = NULL;   // Array to hold variable names from FEASIBLE_SOLUTION
static int g_num_heuristic_solution_vars     = 0;      // Number of variables in the heuristic solution
static SCIP_Bool g_is_heuristic_initialized  = FALSE;  // Flag to check if the heuristic has been initializeds

/*
 * Local methods
 */

/*
 * Callback methods of primal heuristic
 */

/* TODO: Implement all necessary primal heuristic methods. The methods with an #if 0 ... #else #define ... are optional */

/** copy method for primal heuristic plugins (called when SCIP copies plugins) */
static SCIP_DECL_HEURCOPY(heurCopyBadFeasible)
{ /*lint --e{715}*/
  /* Indicate that this heuristic does not support copying by returning SCIP_OKAY
   * and doing nothing. If it needs to be copyable, data would need deep copying.
   * For now, global data is shared if SCIP instances are copied, which might be an issue.
   * However, standard SCIP usage doesn't typically copy heuristics with global state like this.
   */
  return SCIP_OKAY;
}

/** destructor of primal heuristic to free user data (called when SCIP is exiting) */
static SCIP_DECL_HEURFREE(heurFreeBadFeasible)
{ /*lint --e{715}*/
  assert(scip != NULL);
  if (g_heuristic_solution_var_names != NULL)
  {
    for (int i = 0; i < g_num_heuristic_solution_vars; ++i)
    {
      if (g_heuristic_solution_var_names[i] != NULL)
      {
        SCIPfreeBlockMemory(scip, &g_heuristic_solution_var_names[i]);
      }
    }
    SCIPfreeBufferArray(scip, &g_heuristic_solution_var_names);
  }
  g_num_heuristic_solution_vars = 0;
  g_is_heuristic_initialized    = FALSE;
  return SCIP_OKAY;
}

/** initialization method of primal heuristic (called after problem was transformed) */
static SCIP_DECL_HEURINIT(heurInitBadFeasible)
{ /*lint --e{715}*/
  if (!g_is_heuristic_initialized)
  {
    SCIP_CALL(SCIPallocBufferArray(scip, &g_heuristic_solution_var_names, NUM_FEASIBLE_SOLUTION_VARS));
    if (g_heuristic_solution_var_names == NULL)
    {
      SCIPerrorMessage("Memory allocation failed for g_heuristic_solution_var_names buffer in heurInitBadFeasible.\\n");
      return SCIP_NOMEMORY;
    }

    for (int i = 0; i < NUM_FEASIBLE_SOLUTION_VARS; ++i)
    {
      size_t len = strlen(FEASIBLE_SOLUTION[i]);
      SCIP_CALL(SCIPallocBlockMemoryArray(scip, &g_heuristic_solution_var_names[i], len + 1));
      if (g_heuristic_solution_var_names[i] == NULL)
      {
        SCIPerrorMessage("Memory allocation failed for a var name string in heurInitBadFeasible.\\n");
        // Free already allocated strings
        for (int k = 0; k < i; ++k)
        {
          if (g_heuristic_solution_var_names[k] != NULL) SCIPfreeBlockMemory(scip, &g_heuristic_solution_var_names[k]);
        }
        SCIPfreeBufferArray(scip, &g_heuristic_solution_var_names);
        return SCIP_NOMEMORY;
      }
      strncpy(g_heuristic_solution_var_names[i], FEASIBLE_SOLUTION[i], len + 1);
    }
    g_num_heuristic_solution_vars = NUM_FEASIBLE_SOLUTION_VARS;
    g_is_heuristic_initialized    = TRUE;
  }
  return SCIP_OKAY;
}

/** deinitialization method of primal heuristic (called before transformed problem is freed) */
static SCIP_DECL_HEUREXIT(heurExitBadFeasible)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** solving process initialization method of primal heuristic (called when branch and bound process is about to begin) */
static SCIP_DECL_HEURINITSOL(heurInitsolBadFeasible)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/** solving process deinitialization method of primal heuristic (called before branch and bound process data is freed) */
static SCIP_DECL_HEUREXITSOL(heurExitsolBadFeasible)
{ /*lint --e{715}*/

  return SCIP_OKAY;
}

/**
 * @brief Core of the heuristic: it builds one solution based on variables listed in a temporary file.
 *
 * @param scip problem
 * @param sol pointer to the solution structure where the solution wil be saved
 * @param heur pointer to the heuristic handle (to contabilize statistics)
 * @return int 1 if a solution is found and stored, 0 otherwise.
 */
int bad_feasible_solution(SCIP *scip, SCIP_SOL **sol, SCIP_HEUR *heur)
{
  printf("\nBad Feasible heuristic called.\n");

  /* SCIP specific types */
  SCIP_PROBDATA *probdata       = NULL;
  SCIP_VAR **vars_for_sol_array = NULL;
  SCIP_VAR *current_scip_var    = NULL;

  /* Custom SCIP data types */
  Instance *I                   = NULL;

  /* Integers / Booleans */
  int i;
  int found_solution_flag                   = 0;
  unsigned int solution_stored_successfully = FALSE;
  int num_vars_added_to_solution            = 0;

#ifdef DEBUG_BAD_FEASIBLE
  SCIPinfoMessage(scip, NULL, "\\n============== New heuristic: building solution from FEASIBLE_SOLUTION at node: %lld\\n", SCIPnodeGetNumber(SCIPgetCurrentNode(scip)));
#endif

  if (!g_is_heuristic_initialized)
  {
#ifdef DEBUG_BAD_FEASIBLE
    SCIPinfoMessage(scip, NULL, "Heuristic from FEASIBLE_SOLUTION: Data not initialized.\\n");
#endif
    return 0;
  }

  if (g_num_heuristic_solution_vars == 0)
  {
#ifdef DEBUG_BAD_FEASIBLE
    SCIPinfoMessage(scip, NULL, "Heuristic from FEASIBLE_SOLUTION: No variables were loaded (global list is empty).\\n");
#endif
    return 0;  // No variables to form a solution
  }

  /* recover the problem data*/
  probdata = SCIPgetProbData(scip);
  assert(probdata != NULL);

  I = SCIPprobdataGetInstance(probdata);
  assert(I != NULL);

  /* nvars = SCIPprobdataGetNVars(probdata); // For context if needed, not for array sizing here */

  if (g_num_heuristic_solution_vars > 0)
  {
    SCIP_CALL(SCIPallocBufferArray(scip, &vars_for_sol_array, g_num_heuristic_solution_vars));
  }

  for (i = 0; i < g_num_heuristic_solution_vars; ++i)
  {
    const char *var_name_from_tmp = g_heuristic_solution_var_names[i];
    current_scip_var              = SCIPfindVar(scip, var_name_from_tmp);

    if (current_scip_var != NULL)
    {
      /* We sized vars_for_sol_array for g_num_heuristic_solution_vars, so this check is always true if var is found */
      /* and num_vars_added_to_solution will not exceed g_num_heuristic_solution_vars */
      vars_for_sol_array[num_vars_added_to_solution++] = current_scip_var;
    }
    else
    {
#ifdef DEBUG_BAD_FEASIBLE
      SCIPinfoMessage(scip, NULL, "Heuristic from FEASIBLE_SOLUTION: Variable '%s' (from global list) not found in SCIP problem.\\n", var_name_from_tmp);
#endif
    }
  }

  if (num_vars_added_to_solution > 0)
  {
    SCIP_CALL(SCIPcreateSol(scip, sol, heur));
    for (i = 0; i < num_vars_added_to_solution; ++i)
    {
      SCIP_CALL(SCIPsetSolVal(scip, *sol, vars_for_sol_array[i], 1.0));
    }

    SCIP_CALL(SCIPtrySol(scip, *sol, TRUE, TRUE, TRUE, TRUE, TRUE, &solution_stored_successfully));

    if (solution_stored_successfully)
    {
      found_solution_flag = 1;
#ifdef DEBUG_BAD_FEASIBLE
      SCIPinfoMessage(scip, NULL, "Heuristic from FEASIBLE_SOLUTION: Solution with %d variables (from global list) successfully created and stored.\\n", num_vars_added_to_solution);
      SCIP_CALL(SCIPprintSol(scip, *sol, NULL, FALSE));
#endif
    }
    else
    {
#ifdef DEBUG_BAD_FEASIBLE
      SCIPinfoMessage(scip, NULL, "Heuristic from FEASIBLE_SOLUTION: SCIPtrySol did not store the solution (num_vars: %d from global list).\\n", num_vars_added_to_solution);
#endif
    }
  }
  else
  {
#ifdef DEBUG_BAD_FEASIBLE
    SCIPinfoMessage(scip, NULL, "Heuristic from FEASIBLE_SOLUTION: No valid SCIP variables found from the global list to form a solution.\\n");
#endif
  }

  if (vars_for_sol_array != NULL)
  {
    SCIPfreeBufferArray(scip, &vars_for_sol_array);
  }

  return found_solution_flag;
}

/** execution method of primal heuristic */
static SCIP_DECL_HEUREXEC(heurExecBadFeasible)
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

  /* solve bad_feasible_solution */
  if (bad_feasible_solution(scip, &sol, heur))
  {
    *result = SCIP_FOUNDSOL;
  }
  else
  {
    *result = SCIP_DIDNOTFIND;
#ifdef DEBUG_PRIMAL
    printf("\nBadFeasible could not find feasible solution!");
#endif
  }
  return SCIP_OKAY;
}

/*
 * primal heuristic specific interface methods
 */

/** creates the badFeasible_crtp primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPincludeHeurBadFeasibleSolution(
        SCIP *scip /**< SCIP data structure */
)
{
  SCIP_HEURDATA *heurdata;
  SCIP_HEUR *heur;

  /* create rounding primal heuristic data */
  heurdata = NULL;

  heur     = NULL;

  /* include primal heuristic */
#if 0
   /* use SCIPincludeHeur() if you want to set all callbacks explicitly and realize (by getting compiler errors) when
    * new callbacks are added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeHeur(scip, HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, param.heur_freq, param.heur_freqofs,
         param.heur_maxdepth, HEUR_TIMING, HEUR_USESSUBSCIP,
         heurCopyBadFeasible, heurFreeBadFeasible, heurInitBadFeasible, heurExitBadFeasible, heurInitsolBadFeasible, heurExitsolBadFeasible, heurExecBadFeasible,
         heurdata) );
#else
  /* use SCIPincludeHeurBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
  SCIP_CALL(SCIPincludeHeurBasic(scip, &heur,
                                 HEUR_NAME, HEUR_DESC, HEUR_DISPCHAR, HEUR_PRIORITY, HEUR_FREQ, HEUR_FREQOFS,
                                 HEUR_MAXDEPTH, HEUR_TIMING, HEUR_USESSUBSCIP, heurExecBadFeasible, heurdata));

  assert(heur != NULL);

  /* set non fundamental callbacks via setter functions */
  SCIP_CALL(SCIPsetHeurCopy(scip, heur, heurCopyBadFeasible));
  SCIP_CALL(SCIPsetHeurFree(scip, heur, heurFreeBadFeasible));
  SCIP_CALL(SCIPsetHeurInit(scip, heur, heurInitBadFeasible));
  SCIP_CALL(SCIPsetHeurExit(scip, heur, heurExitBadFeasible));
  SCIP_CALL(SCIPsetHeurInitsol(scip, heur, heurInitsolBadFeasible));
  SCIP_CALL(SCIPsetHeurExitsol(scip, heur, heurExitsolBadFeasible));
#endif

  /* add badFeasible primal heuristic parameters */
  /* TODO: (optional) add primal heuristic specific parameters with SCIPaddTypeParam() here */

  return SCIP_OKAY;
}
