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

/**@file   heur_lns.h
 * @ingroup PRIMALHEURISTICS
 * @brief  lns primal heuristic
 * @author Edna Hoshino (based on template provided by Tobias Achterberg)
 *
 * template file for primal heuristic plugins
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_HEUR_LNS_H__
#define __SCIP_HEUR_LNS_H__

#include "problem.h"
#include "scip/scip.h"

typedef struct
{
  Course *course_ptr;       /**< pointer to the course */
  Professor *professor_ptr; /**< pointer to the professor */
} LNS_Candidate;

#ifdef __cplusplus
extern "C"
{
#endif

  int lns(SCIP *scip, SCIP_SOL *initsol, SCIP_HEUR *heur);

  /** creates the lns_crtp primal heuristic and includes it in SCIP */
  SCIP_RETCODE SCIPincludeHeurLns(
#ifdef __cplusplus
extern "C"
{
#endif
    int rounding(SCIP * scip, SCIP_SOL * *sol, SCIP_HEUR * heur);

    // Auxiliary structures to simulate removed fields from the new structure
    typedef struct
    {
      int course_label;  // label of the course
      int score;         // score for GRASP selection
    } PreferenceAux;

    typedef struct
    {
      Course *course;    // pointer to the original course
      int covered;       // 0 or 1 - if course is covered
      int current_prof;  // assigned professor label
    } CourseAux;

    typedef struct
    {
      Professor *professor;  // pointer to the original professor
      int n;                 // degree of vertex (number of courses professor can teach)
      PreferenceAux *pref;   // preferences list for GRASP (dynamically allocated)
    } ProfessorAux;

    int grasp(SCIP * scip, SCIP_SOL * *sol, SCIP_HEUR * heur);

    /** creates the grasp_crtp primal heuristic and includes it in SCIP */
    SCIP_RETCODE SCIPincludeHeurGrasp(
            SCIP * scip /**< SCIP data structure */
    );

#ifdef __cplusplus
}
#endif

#endif
