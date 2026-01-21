/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "problem.h"
#include "scip/scip.h"

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

/**@file   probdata_mochila.h
 * @brief  Problem data for mochila problem
 * @author Timo Berthold
 * @author Stefan Heinz
 *
 * This file handles the main problem data used in that project. For more details see \ref PROBLEMDATA page.
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_PROBDATA_MOCHILA__
#define __SCIP_PROBDATA_MOCHILA__

#include "problem.h"
#include "scip/scip.h"

/* constants */

/* macros */
#define EPSILON 0.000001
#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

int checaArea(unsigned int areaProfessor, unsigned int areaTurma);

/** @brief Problem data which is accessible in all places
 *
 * This problem data is used to store the input of the mochila, all variables which are created initially
 */
struct SCIP_ProbData
{
  const char *probname;    /**< problem name */
  SCIP_VAR **vars;         /**< array of variables */
  SCIP_CONS **conss;       /**< all constraints */
  int nvars;               /**< total of vars */
  int ncons;               /**< number of constraints */
  Instance *I;             /**< instance of knapsack */
  SCIP_Bool owns_instance; /**< whether this probdata owns the instance and should free it */
};

/** sets up the problem data */
extern SCIP_RETCODE SCIPprobdataCreate(
        SCIP *scip,           /**< SCIP data structure */
        const char *probname, /**< problem name */
        Instance *I,          /**< instance of K-coloring */
        int relaxed,          /**< should be relaxed? */
        int *fixed,           /**< vector of fixed items (can be NULL) */
        SCIP_Bool owns_instance);

/** adds given variable to the problem data */
extern SCIP_RETCODE SCIPprobdataAddVar(
        SCIP *scip,              /**< SCIP data structure */
        SCIP_PROBDATA *probdata, /**< problem data */
        SCIP_VAR *var            /**< variables to add */
);

/** returns Probname of the instance */
extern const char *SCIPprobdataGetProbname(
        SCIP_PROBDATA *probdata /**< problem data */
);
/** returns array of all variables ordered in the way they got generated */
extern SCIP_VAR **SCIPprobdataGetVars(
        SCIP_PROBDATA *probdata /**< problem data */
);

/** returns number of variables */
extern int SCIPprobdataGetNVars(
        SCIP_PROBDATA *probdata /**< problem data */
);

/** returns array of set partitioning constrains */
extern SCIP_CONS **SCIPprobdataGetConss(
        SCIP_PROBDATA *probdata /**< problem data */
);

/** returns array of set partitioning constrains */
extern int SCIPprobdataGetNcons(
        SCIP_PROBDATA *probdata /**< problem data */
);

/** returns instance I */
extern Instance *SCIPprobdataGetInstance(
        SCIP_PROBDATA *probdata /**< problem data */
);
#endif
