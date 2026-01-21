/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2014 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   probdata_dpd.c
 * @brief  Problem data for DPD (Professor Assignment) problem
 * @author Edna Hoshino (based on template by Timo Berthold and Stefan Heinz)
 *
 * This file handles the main problem data for the Professor Assignment Problem.
 * It implements the SCIP problem data structure and callbacks for managing
 * variables, constraints, and instance data lifecycle.
 *
 * The problem models assigning professors to courses with constraints on:
 * - Course coverage (each course assigned to exactly one professor)
 * - Professor workload (minimum and maximum per semester/year)
 * - Professor expertise areas (soft constraint with penalties)
 * - Professor preferences (objective coefficients)
 **/
#include <assert.h>
#include <string.h>

#include "probdata_dpd.h"
#include "scip/scipdefplugins.h"

/* Constants */
#define BINARY_STRING_SIZE 33

/* Local function prototypes */
static SCIP_RETCODE createVariables(SCIP *scip, SCIP_VAR ***vars, int *nvars,
                                    Instance *I, int relaxed, int *fixed);
static SCIP_RETCODE createConstraints(SCIP *scip, SCIP_CONS ***conss, int *ncons,
                                      SCIP_VAR **vars, Instance *I);
static SCIP_RETCODE addCourseAssignmentConstraints(SCIP *scip, SCIP_CONS **conss,
                                                   int *ncons, SCIP_VAR **vars, Instance *I);
static SCIP_RETCODE addWorkloadConstraints(SCIP *scip, SCIP_CONS **conss,
                                           int *ncons, SCIP_VAR **vars, Instance *I);
static int calculateAptitudeCoefficient(Instance *I, int professor_idx, int course_idx);
static SCIP_Bool isProfessorEligibleForCourse(Instance *I, int professor_idx, int course_idx);

/**@name Utility functions
 * @{
 */

/** Convert unsigned long to binary string representation */
static void convertToBinaryString(unsigned long num, char *str, int len)
{
  assert(str != NULL);
  assert(len > 0);

  memset(str, '0', len);
  str[len] = '\0';

  for (int i = len - 1; i >= 0; i--)
  {
    if (num & 1)
      str[i] = '1';
    num >>= 1;
  }
}

/** Legacy function for binary string conversion (kept for compatibility) */
static void bin2str(unsigned long num, char *str, int len)
{
  convertToBinaryString(num, str, len);
}

/** Check if professor has expertise in course area */
static SCIP_Bool isProfessorEligibleForCourse(Instance *I, int professor_idx, int course_idx)
{
  assert(I != NULL);
  assert(professor_idx >= 0 && professor_idx < I->nProfessors);
  assert(course_idx >= 0 && course_idx < I->nCourses);

  unsigned int professor_areas = I->professors[professor_idx].areas;
  unsigned int course_areas    = I->courses[course_idx].subject.areas;

  return (professor_areas & course_areas) > 0;
}

/** Calculate aptitude coefficient for professor-course assignment */
static int calculateAptitudeCoefficient(Instance *I, int professor_idx, int course_idx)
{
  assert(I != NULL);
  assert(professor_idx >= 0 && professor_idx < I->nProfessors);
  assert(course_idx >= 0 && course_idx < I->nCourses);

  // Check if preferences array is properly initialized
  if (I->professors[professor_idx].preferences == NULL)
  {
    return -I->area_penalty;
  }

  if (isProfessorEligibleForCourse(I, professor_idx, course_idx))
  {
    int preference_weight = I->professors[professor_idx].preferences[course_idx].weight;
    return (preference_weight > 0) ? preference_weight : 1;
  }
  else
  {
    return -I->area_penalty;
  }
}

/** Legacy function for area checking (kept for compatibility) */
int checaArea(unsigned int areaProfessor, unsigned int areaTurma)
{
  //printf("Area Professor: %u, Area Turma: %u\n", areaProfessor, areaTurma);
  unsigned int result = areaProfessor & areaTurma;
  //printf("Area Check Result: %u\n", result > 0 ? 1 : 0);

  return result > 0 ? 1 : 0;
}

/**@} */

/**@name Problem data management
 * @{
 */

/** Creates problem data structure */
static SCIP_RETCODE probdataCreate(
        SCIP *scip,               /**< SCIP data structure */
        SCIP_PROBDATA **probdata, /**< pointer to problem data */
        const char *probname,     /**< problem name */
        SCIP_VAR **vars,          /**< array of variables */
        SCIP_CONS **conss,        /**< all constraints */
        int nvars,                /**< size of vars */
        int ncons,                /**< number of constraints */
        Instance *I,              /**< pointer to the instance data */
        SCIP_Bool owns_instance   /**< whether this probdata owns the instance */
)
{
  assert(scip != NULL);
  assert(probdata != NULL);

  /* Allocate memory for problem data */
  SCIP_CALL(SCIPallocMemory(scip, probdata));

  /* Initialize variables array */
  if (nvars > 0)
  {
    assert(vars != NULL);
    SCIP_CALL(SCIPduplicateMemoryArray(scip, &(*probdata)->vars, vars, nvars));
  }
  else
  {
    (*probdata)->vars = NULL;
  }

  /* Initialize constraints array */
  SCIP_CALL(SCIPduplicateMemoryArray(scip, &(*probdata)->conss, conss, ncons));

  /* Set problem data fields */
  (*probdata)->I             = I;
  (*probdata)->nvars         = nvars;
  (*probdata)->ncons         = ncons;
  (*probdata)->probname      = probname;
  (*probdata)->owns_instance = owns_instance;

  return SCIP_OKAY;
}

/** Frees the memory of the given problem data */
static SCIP_RETCODE probdataFree(
        SCIP *scip,               /**< SCIP data structure */
        SCIP_PROBDATA **probdata, /**< pointer to problem data */
        SCIP_Bool transformed     /**< whether we are in transformed problem */
)
{
  assert(scip != NULL);
  assert(probdata != NULL);
  assert(*probdata != NULL);

  /* Release all variables */
  for (int i = 0; i < (*probdata)->nvars; ++i)
  {
    SCIP_CALL(SCIPreleaseVar(scip, &(*probdata)->vars[i]));
  }

  /* Release all constraints */
  for (int i = 0; i < (*probdata)->ncons; ++i)
  {
    SCIP_CALL(SCIPreleaseCons(scip, &(*probdata)->conss[i]));
  }

  /* Free memory arrays */
  if ((*probdata)->vars != NULL)
    SCIPfreeMemoryArray(scip, &(*probdata)->vars);
  SCIPfreeMemoryArray(scip, &(*probdata)->conss);

  /* Free instance data if we own it and not in transformed problem */
  if (!transformed && (*probdata)->owns_instance && (*probdata)->I != NULL)
  {
    freeInstance((*probdata)->I);
  }

  /* Free problem data structure */
  SCIPfreeMemory(scip, probdata);

  return SCIP_OKAY;
}

/**@} */

/**@name Variable creation
 * @{
 */

/** Creates all decision variables for the professor assignment problem */
static SCIP_RETCODE createVariables(
        SCIP *scip,       /**< SCIP data structure */
        SCIP_VAR ***vars, /**< pointer to array of variables */
        int *nvars,       /**< pointer to number of variables */
        Instance *I,      /**< instance data */
        int relaxed,      /**< should variables be relaxed? */
        int *fixed        /**< vector of fixed items (can be NULL) */
)
{
  SCIP_VAR *var;
  char name[SCIP_MAXSTRLEN];
  double lower_bound, upper_bound;
  int variable_count = 0;

  assert(scip != NULL);
  assert(vars != NULL);
  assert(nvars != NULL);
  assert(I != NULL);

  /* Allocate memory for variables array */
  int total_vars = I->nProfessors * I->nCourses;
  SCIP_CALL(SCIPallocBufferArray(scip, vars, total_vars));

  /* Create variables x_{i,j} for each professor-course pair */
  for (int i = 0; i < I->nProfessors; i++)
  {
    for (int j = 0; j < I->nCourses; j++)
    {
      /* Generate variable name */
      (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "x_%d_%d", i, j);

      /* Determine variable bounds */
      if (fixed == NULL)
      {
        lower_bound = 0.0;
        upper_bound = 1.0;
      }
      else
      {
        int idx     = i * I->nCourses + j;
        lower_bound = (fixed[idx] == 1) ? 1.0 : 0.0;
        upper_bound = (fixed[idx] == -1) ? 0.0 : 1.0;
      }

      /* Calculate objective coefficient */
      int objective_coeff   = calculateAptitudeCoefficient(I, i, j);

      /* Create variable */
      SCIP_VARTYPE var_type = relaxed ? SCIP_VARTYPE_CONTINUOUS : SCIP_VARTYPE_BINARY;
      SCIP_CALL(SCIPcreateVarBasic(scip, &var, name, lower_bound, upper_bound,
                                   (double) objective_coeff, var_type));

      assert(var != NULL);
      (*vars)[variable_count] = var;
      SCIP_CALL(SCIPaddVar(scip, var));
      variable_count++;
    }
  }

  *nvars = variable_count;
  return SCIP_OKAY;
}

/**@} */

/**@name Constraint creation
 * @{
 */

/** Creates all constraints for the professor assignment problem */
static SCIP_RETCODE createConstraints(
        SCIP *scip,         /**< SCIP data structure */
        SCIP_CONS ***conss, /**< pointer to array of constraints */
        int *ncons,         /**< pointer to number of constraints */
        SCIP_VAR **vars,    /**< array of variables */
        Instance *I         /**< instance data */
)
{
  assert(scip != NULL);
  assert(conss != NULL);
  assert(ncons != NULL);
  assert(vars != NULL);
  assert(I != NULL);

  /* Allocate memory for constraints array */
  int max_constraints = I->nCourses + 3 * I->nProfessors;
  SCIP_CALL(SCIPallocBufferArray(scip, conss, max_constraints));

  *ncons = 0;

  /* Add course assignment constraints */
  SCIP_CALL(addCourseAssignmentConstraints(scip, *conss, ncons, vars, I));

  /* Add workload constraints */
  SCIP_CALL(addWorkloadConstraints(scip, *conss, ncons, vars, I));

  return SCIP_OKAY;
}

/** Adds course assignment constraints: each course assigned to exactly one professor */
static SCIP_RETCODE addCourseAssignmentConstraints(
        SCIP *scip,        /**< SCIP data structure */
        SCIP_CONS **conss, /**< array of constraints */
        int *ncons,        /**< pointer to number of constraints */
        SCIP_VAR **vars,   /**< array of variables */
        Instance *I        /**< instance data */
)
{
  char name[SCIP_MAXSTRLEN];

  assert(scip != NULL);
  assert(conss != NULL);
  assert(ncons != NULL);
  assert(vars != NULL);
  assert(I != NULL);

  for (int j = 0; j < I->nCourses; j++)
  {
    (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "course_assignment_%d", j);

    /* Create constraint: sum over professors = 1 */
    SCIP_CALL(SCIPcreateConsBasicLinear(scip, &conss[*ncons], name, 0, NULL, NULL, 1.0, 1.0));
    SCIP_CALL(SCIPaddCons(scip, conss[*ncons]));

    /* Add variables for all professors for this course */
    for (int i = 0; i < I->nProfessors; i++)
    {
      int var_idx = i * I->nCourses + j;
      SCIP_CALL(SCIPaddCoefLinear(scip, conss[*ncons], vars[var_idx], 1.0));
    }

    (*ncons)++;
  }

  return SCIP_OKAY;
}

/** Adds workload constraints for professors */
static SCIP_RETCODE addWorkloadConstraints(
        SCIP *scip,        /**< SCIP data structure */
        SCIP_CONS **conss, /**< array of constraints */
        int *ncons,        /**< pointer to number of constraints */
        SCIP_VAR **vars,   /**< array of variables */
        Instance *I        /**< instance data */
)
{
  char name[SCIP_MAXSTRLEN];

  assert(scip != NULL);
  assert(conss != NULL);
  assert(ncons != NULL);
  assert(vars != NULL);
  assert(I != NULL);

  for (int i = 0; i < I->nProfessors; i++)
  {
    /* Annual minimum workload constraint */
    (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "min_workload_%d", i);
    SCIP_CALL(SCIPcreateConsBasicLinear(scip, &conss[*ncons], name, 0, NULL, NULL,
                                        I->professors[i].minWorkload, SCIPinfinity(scip)));
    SCIP_CALL(SCIPaddCons(scip, conss[*ncons]));

    for (int j = 0; j < I->nCourses; j++)
    {
      int var_idx = i * I->nCourses + j;
      SCIP_CALL(SCIPaddCoefLinear(scip, conss[*ncons], vars[var_idx], I->courses[j].workload));
    }
    (*ncons)++;

    /* First semester maximum workload constraint */
    (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "max_workload_sem1_%d", i);
    SCIP_CALL(SCIPcreateConsBasicLinear(scip, &conss[*ncons], name, 0, NULL, NULL,
                                        -SCIPinfinity(scip), I->professors[i].maxWorkload1));
    SCIP_CALL(SCIPaddCons(scip, conss[*ncons]));

    for (int j = 0; j < I->nCourses; j++)
    {
      if (I->courses[j].semester == 1)
      {
        int var_idx = i * I->nCourses + j;
        SCIP_CALL(SCIPaddCoefLinear(scip, conss[*ncons], vars[var_idx], I->courses[j].workload));
      }
    }
    (*ncons)++;

    /* Second semester maximum workload constraint */
    (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "max_workload_sem2_%d", i);
    SCIP_CALL(SCIPcreateConsBasicLinear(scip, &conss[*ncons], name, 0, NULL, NULL,
                                        -SCIPinfinity(scip), I->professors[i].maxWorkload2));
    SCIP_CALL(SCIPaddCons(scip, conss[*ncons]));

    for (int j = 0; j < I->nCourses; j++)
    {
      if (I->courses[j].semester == 2)
      {
        int var_idx = i * I->nCourses + j;
        SCIP_CALL(SCIPaddCoefLinear(scip, conss[*ncons], vars[var_idx], I->courses[j].workload));
      }
    }
    (*ncons)++;
  }

  return SCIP_OKAY;
}

/**@} */

/**@name SCIP callbacks
 * @{
 */
/** Frees user data of original problem (called when the original problem is freed) */
static SCIP_DECL_PROBDELORIG(probdelorigDPD)
{
  SCIPdebugMessage("free original problem data\n");
  SCIP_CALL(probdataFree(scip, probdata, FALSE));
  return SCIP_OKAY;
}

/** Creates user data of transformed problem by transforming the original user problem data */
static SCIP_DECL_PROBTRANS(probtransDPD)
{
  /* Create transform probdata */
  SCIP_CALL(probdataCreate(scip, targetdata, sourcedata->probname, sourcedata->vars,
                           sourcedata->conss, sourcedata->nvars, sourcedata->ncons,
                           sourcedata->I, sourcedata->owns_instance));

  /* Transform all constraints */
  SCIP_CALL(SCIPtransformConss(scip, (*targetdata)->ncons, sourcedata->conss, (*targetdata)->conss));

  /* Transform all variables */
  SCIP_CALL(SCIPtransformVars(scip, (*targetdata)->nvars, sourcedata->vars, (*targetdata)->vars));

  return SCIP_OKAY;
}

/** Frees user data of transformed problem (called when the transformed problem is freed) */
static SCIP_DECL_PROBDELTRANS(probdeltransDPD)
{
  SCIPdebugMessage("free transformed problem data\n");
  SCIP_CALL(probdataFree(scip, probdata, TRUE));
  return SCIP_OKAY;
}

/** Solving process initialization method of transformed data */
static SCIP_DECL_PROBINITSOL(probinitsolDPD)
{
  assert(probdata != NULL);
  return SCIP_OKAY;
}

/** Solving process deinitialization method of transformed data */
static SCIP_DECL_PROBEXITSOL(probexitsolDPD)
{
  assert(probdata != NULL);
  return SCIP_OKAY;
}

/**@} */

/**@name Interface methods
 *
 * @{
 */

/** Sets up the problem data for main SCIP instance */
SCIP_RETCODE SCIPprobdataCreate(
        SCIP *scip,           /**< SCIP data structure */
        const char *probname, /**< problem name */
        Instance *I,          /**< instance data */
        int relaxed,          /**< should variables be relaxed? */
        int *fixed,           /**< vector of fixed items (can be NULL) */
        SCIP_Bool owns_instance)
{
  SCIP_PROBDATA *probdata;
  SCIP_CONS **conss;
  SCIP_VAR **vars;
  int nvars, ncons;

  assert(scip != NULL);
  assert(I != NULL);

  /* Create problem in SCIP and set callbacks */
  SCIP_CALL(SCIPcreateProbBasic(scip, probname));
  SCIP_CALL(SCIPsetProbDelorig(scip, probdelorigDPD));
  SCIP_CALL(SCIPsetProbTrans(scip, probtransDPD));
  SCIP_CALL(SCIPsetProbDeltrans(scip, probdeltransDPD));
  SCIP_CALL(SCIPsetProbInitsol(scip, probinitsolDPD));
  SCIP_CALL(SCIPsetProbExitsol(scip, probexitsolDPD));

  /* Set problem properties */
  SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MAXIMIZE));
  SCIP_CALL(SCIPsetObjIntegral(scip));

  /* Create variables */
  SCIP_CALL(createVariables(scip, &vars, &nvars, I, relaxed, fixed));

  /* Create constraints */
  SCIP_CALL(createConstraints(scip, &conss, &ncons, vars, I));

  /* Create problem data structure */
  SCIP_CALL(probdataCreate(scip, &probdata, probname, vars, conss, nvars, ncons, I, owns_instance));

  /* Set problem data in SCIP */
  SCIP_CALL(SCIPsetProbData(scip, probdata));

  /* Clean up temporary arrays */
  for (int i = 0; i < nvars; ++i)
    SCIP_CALL(SCIPreleaseVar(scip, &vars[i]));
  for (int i = 0; i < ncons; ++i)
    SCIP_CALL(SCIPreleaseCons(scip, &conss[i]));

  SCIPfreeBufferArray(scip, &conss);
  SCIPfreeBufferArray(scip, &vars);

  return SCIP_OKAY;
}

Instance *SCIPprobdataGetInstance(
        SCIP_PROBDATA *probdata)
{
  return probdata->I;
}

/** returns array of all variables in the way they got generated */
SCIP_VAR **SCIPprobdataGetVars(
        SCIP_PROBDATA *probdata /**< problem data */
)
{
  return probdata->vars;
}

/** returns number of variables */
int SCIPprobdataGetNVars(
        SCIP_PROBDATA *probdata /**< problem data */
)
{
  return probdata->nvars;
}
/** returns the array of constrains */
SCIP_CONS **SCIPprobdataGetConss(
        SCIP_PROBDATA *probdata /**< problem data */
)
{
  return probdata->conss;
}

/** returns the total of constrains */
int SCIPprobdataGetNcons(
        SCIP_PROBDATA *probdata /**< problem data */
)
{
  return probdata->ncons;
}
/** returns Probname of the instance */
const char *SCIPprobdataGetProbname(
        SCIP_PROBDATA *probdata /**< problem data */
)
{
  return probdata->probname;
}

/**@} */
