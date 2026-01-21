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

// reset level of each course
void reset_course_level(CourseAux *courses_aux, int m)
{
  for (int t = 0; t < m; t++)
  {
    courses_aux[t].course->level = 0;
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

// calculate the score of the edges
int calculaScore(int peso_preferencia, int alfa)
{
  return peso_preferencia + alfa * 1;
}

// compare level of two courses
int compareCourses(const void *a, const void *b)
{
  const CourseAux *courseA = (const CourseAux *) a;
  const CourseAux *courseB = (const CourseAux *) b;
  return (courseA->course->level - courseB->course->level);
}

// compare level of two professors
int compareProfessoresByLevel(const void *a, const void *b)
{
  const ProfessorAux *profA = (const ProfessorAux *) a;
  const ProfessorAux *profB = (const ProfessorAux *) b;

  if (profA->n < profB->n) return -1;  // a comes first
  if (profA->n > profB->n) return 1;   // b comes first
  return 0;                            // equal
}

// check if professor area matches course area using bitmask
int check_area_bitmask(Area prof_areas, Area course_areas)
{
  return (prof_areas & course_areas) != 0 ? 1 : 0;
}

// check preference weight for a given course (using course label)
double check_preference_weight(Preference *preferences, int num_preferences, int course_label)
{
  if (course_label < 0 || course_label >= num_preferences)
    return 0.0;

  return preferences[course_label].weight;
}

// create the edges between professors and courses
void adaptive_edges(ProfessorAux *profs_aux, CourseAux *courses_aux, int n, int m)
{
  double peso;
  int score, n_pref;

  /*idea:
  1° compute degrees (professors possible assignments and courses possible professors)
  2° for each feasible (prof, course) pair compute a score that favors low degree vertices
    for that: use a formula that combines the original preference weight with inverse degree terms
  */

  // zero degrees and temp counts
  for (int p = 0; p < n; p++)
  {
    profs_aux[p].n = 0;
  }
  for (int t = 0; t < m; t++)
  {
    courses_aux[t].course->level = 0;
  }

  // 1°: count possible edges (degrees) without setting preference list
  for (int p = 0; p < n; p++)
  {
    Professor *prof = profs_aux[p].professor;
    for (int t = 0; t < m; t++)
    {
      if (courses_aux[t].covered == 1) continue;
      Course *course = courses_aux[t].course;

      peso           = check_preference_weight(prof->preferences, m, course->label);
      if (peso > EPSILON)
      {
        profs_aux[p].n++;
        courses_aux[t].course->level++;
      }
      else if (check_area_bitmask(prof->areas, course->subject.areas) == 1)
      {
        profs_aux[p].n++;
        courses_aux[t].course->level++;
      }
    }
  }

  // 2°: fill pref lists and compute scores using degrees
  const double K = 5.0; /* weight factor to amplify inverse-degree effect */
  for (int p = 0; p < n; p++)
  {
    Professor *prof = profs_aux[p].professor;
    n_pref          = 0;  // index to fill profs_aux[p].pref

    // if no possible courses, ensure n is zero
    if (profs_aux[p].n == 0)
    {
      profs_aux[p].n = 0;
      continue;
    }

    for (int t = 0; t < m; t++)
    {
      if (courses_aux[t].covered == 1) continue;
      Course *course = courses_aux[t].course;

      peso           = check_preference_weight(prof->preferences, m, course->label);
      if (peso > EPSILON)
      {
        // combine base preference with inverse-degree bonuses
        double degp                            = (double) profs_aux[p].n;                // level of the current professor
        double degt                            = (double) courses_aux[t].course->level;  // level of the current course
        double base                            = peso;
        double bonus                           = K * (1.0 / (1.0 + degp) + 1.0 / (1.0 + degt));
        score                                  = (int) ceil(base * (1.0 + bonus));

        profs_aux[p].pref[n_pref].course_label = course->label;
        profs_aux[p].pref[n_pref].score        = score;
        n_pref++;
      }
      else if (check_area_bitmask(prof->areas, course->subject.areas) == 1)
      {
        double degp                            = (double) profs_aux[p].n;                // level of the current professor
        double degt                            = (double) courses_aux[t].course->level;  // level of the current course
        double base                            = 1.0;                                    // small base weight for area-based edges
        double bonus                           = K * (1.0 / (1.0 + degp) + 1.0 / (1.0 + degt));
        score                                  = (int) ceil(base * (1.0 + bonus));

        profs_aux[p].pref[n_pref].course_label = course->label;
        profs_aux[p].pref[n_pref].score        = score;
        n_pref++;
      }
    }
    profs_aux[p].n = n_pref;  // update actual filled count
  }
}

int grasp_randomized_selection(PreferenceAux *candidates, int num_candidates, float alpha)
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
        SCIP *scip,               // SCIP instance
        SCIP_VAR **varlist,       // array of all variables
        SCIP_VAR **solution,      // outupt sulution array
        ProfessorAux *profs_aux,  // auxiliar professor structures
        CourseAux *courses_aux,   // auxiliar course structures
        int *nInSolution,         // number of variables in solution
        int *nCovered,            // number of covered couses
        int m,                    // number of couses
        int n,                    // number of professors
        float alpha)
{

  CourseAux *courses_sem_profs       = (CourseAux *) malloc(sizeof(CourseAux) * m);  // buffer sized to m
  int n_sem_prof                     = 0, num_candidates, prof_label;

  // track assigned variable per course label (index = label)
  SCIP_VAR **assigned_var_for_course = (SCIP_VAR **) calloc(m, sizeof(SCIP_VAR *));
  for (int i = 0; i < m; i++)
  {
    assigned_var_for_course[i] = NULL;
  }

  // build mapping label -> CourseAux* (courses_aux array may be reordered with qsort)
  CourseAux **course_by_label = (CourseAux **) malloc(sizeof(CourseAux *) * m);
  for (int idx = 0; idx < m; idx++)
  {
    course_by_label[idx] = NULL;
  }
  while (*nCovered < m)
  {
    int t;
    for (t = 0; t < m; t++)
    {
      CourseAux course_aux            = courses_aux[t];
      Course *course                  = course_aux.course;
      PreferenceAux *candidate_scores = (PreferenceAux *) malloc(sizeof(PreferenceAux) * n);
      SCIP_VAR **candidate_vars       = (SCIP_VAR **) malloc(sizeof(SCIP_VAR *) * n);
      num_candidates                  = 0;  // number of the candidates professors for the current course

      for (int p = 0; p < n; p++)
      {
        prof_label = profs_aux[p].professor->label;

        for (int s = 0; s < profs_aux[p].n && profs_aux[p].pref[s].course_label <= course->label; s++)
        {

          if (course->label == profs_aux[p].pref[s].course_label)
          {
            if (course->semester == 1)
            {

              if (profs_aux[p].professor->current_CH1 + course->workload <= profs_aux[p].professor->maxWorkload1)
              {
                candidate_scores[num_candidates].course_label = p;
                candidate_scores[num_candidates].score        = profs_aux[p].pref[s].score;
                candidate_vars[num_candidates]                = varlist[prof_label * m + course->label];
                num_candidates++;
              }
            }
            else
            {
              if (profs_aux[p].professor->current_CH2 + course->workload <= profs_aux[p].professor->maxWorkload2)
              {
                candidate_scores[num_candidates].course_label = p;
                candidate_scores[num_candidates].score        = profs_aux[p].pref[s].score;
                candidate_vars[num_candidates]                = varlist[prof_label * m + course->label];
                num_candidates++;
              }
            }
            break;
          }
        }
      }

      if (num_candidates > 0)
      {
        int selected                           = grasp_randomized_selection(candidate_scores, num_candidates, alpha);
        int p                                  = candidate_scores[selected].course_label;

        // register provisional assignment into mapping (we will rebuild solution list later)
        assigned_var_for_course[course->label] = candidate_vars[selected];
        courses_aux[t].covered                 = 1;
        courses_aux[t].current_prof            = p;
        (*nCovered)++;

        if (course->semester == 1)
        {
          profs_aux[p].professor->current_CH1 += course->workload;
        }
        else
        {
          profs_aux[p].professor->current_CH2 += course->workload;
        }

        reset_course_level(courses_aux, m);
        adaptive_edges(profs_aux, courses_aux, n, m);
        qsort(courses_aux, m, sizeof(CourseAux), compareCourses);  // ordering the courses by the level of the vertex
      }
      else
      {
        courses_sem_profs[n_sem_prof] = course_aux;
        n_sem_prof++;
      }

      free(candidate_scores);
      free(candidate_vars);
    }

    // create mapping label -> CourseAux*
    // (courses_aux array was possibly reordered by qsort)
    for (int idx = 0; idx < m; idx++)
    {
      if (courses_aux[idx].course->label >= 0 && courses_aux[idx].course->label < m)
        course_by_label[courses_aux[idx].course->label] = &courses_aux[idx];
    }

    if (*nCovered < m)
    {
      for (int i = 0; i < n_sem_prof; i++)
      {
        int course_label = courses_sem_profs[i].course->label;

        if (courses_sem_profs[i].course->semester == 1)
        {
          for (int p = 0; p < n; p++)
          {
            prof_label = profs_aux[p].professor->label;

            if (profs_aux[p].professor->current_CH1 + courses_sem_profs[i].course->workload <= profs_aux[p].professor->maxWorkload1)
            {
              profs_aux[p].professor->current_CH1 += courses_sem_profs[i].course->workload;
              assigned_var_for_course[course_label] = varlist[prof_label * m + course_label];

              // update the correct CourseAux using the mapping
              if (course_by_label[course_label] != NULL)
              {
                course_by_label[course_label]->covered      = 1;
                course_by_label[course_label]->current_prof = prof_label;
              }
              (*nCovered)++;
              break;
            }
          }
        }
        else
        {

          for (int p = 0; p < n; p++)
          {
            prof_label = profs_aux[p].professor->label;

            if (profs_aux[p].professor->current_CH2 + courses_sem_profs[i].course->workload <= profs_aux[p].professor->maxWorkload2)
            {
              profs_aux[p].professor->current_CH2 += courses_sem_profs[i].course->workload;
              assigned_var_for_course[course_label] = varlist[prof_label * m + course_label];

              // update the correct CourseAux using the mapping
              if (course_by_label[course_label] != NULL)
              {
                course_by_label[course_label]->covered      = 1;
                course_by_label[course_label]->current_prof = prof_label;
              }
              (*nCovered)++;
              break;
            }
          }
        }
      }
    }
  }

  // mapping was already created above, but refresh it in case anything changed
  // (this is now redundant but kept for safety)
  for (int idx = 0; idx < m; idx++)
  {
    if (courses_aux[idx].course->label >= 0 && courses_aux[idx].course->label < m)
      course_by_label[courses_aux[idx].course->label] = &courses_aux[idx];
  }

  // repair phase: try to move assigned courses to professors who are below minWorkload without breaking others minWorkload/maxWorkload
  int repairs = 0;
  for (int i = 0; i < n; i++)
  {
    int changed = 0;
    for (int p = 0; p < n; p++)
    {
      int total_workload = profs_aux[p].professor->current_CH1 + profs_aux[p].professor->current_CH2;
      if (total_workload >= profs_aux[p].professor->minWorkload) continue;
      // try to find a course j that can be moved to p
      for (int j = 0; j < m; j++)
      {
        SCIP_VAR *varj = assigned_var_for_course[j];
        if (varj == NULL) continue;

        CourseAux *ct = course_by_label[j];
        if (ct == NULL) continue;

        int owner = ct->current_prof;
        if (owner == p) continue;

        // check semester capacity for p
        if (ct->course->semester == 1)
        {
          if (profs_aux[p].professor->current_CH1 + ct->course->workload > profs_aux[p].professor->maxWorkload1) continue;
        }
        else
        {
          if (profs_aux[p].professor->current_CH2 + ct->course->workload > profs_aux[p].professor->maxWorkload2) continue;
        }

        // ensure owner wont fall below minWorkload after removing this course
        int owner_total = profs_aux[owner].professor->current_CH1 + profs_aux[owner].professor->current_CH2;
        if (owner_total - ct->course->workload < profs_aux[owner].professor->minWorkload) continue;

        // in that is possible perform move!
        if (ct->course->semester == 1)
        {
          profs_aux[p].professor->current_CH1 += ct->course->workload;      // adding to new professor
          profs_aux[owner].professor->current_CH1 -= ct->course->workload;  // removing from old
        }
        else
        {
          profs_aux[p].professor->current_CH2 += ct->course->workload;
          profs_aux[owner].professor->current_CH2 -= ct->course->workload;
        }
        ct->current_prof           = p;

        // update assigned var mapping to point to the var of new professor
        int new_prof_label         = profs_aux[p].professor->label;
        assigned_var_for_course[j] = varlist[new_prof_label * m + j];
        changed                    = 1;
        repairs++;
        break;  // try to satisfy next underloaded prof
      }
    }
    if (!changed) break;
  }

  int i = 0;
  for (int p = 0; p < n; p++)
  {
    if (profs_aux[p].professor->current_CH1 + profs_aux[p].professor->current_CH2 < profs_aux[p].professor->minWorkload) i++;
  }

  // rebuild solution array from assigned_var_for_course
  *nInSolution = 0;
  for (int j = 0; j < m; j++)
  {
    if (assigned_var_for_course[j] != NULL)
    {
      solution[(*nInSolution)++] = assigned_var_for_course[j];
    }
  }

  free(assigned_var_for_course);
  free(course_by_label);
  free(courses_sem_profs);
}

SCIP_Real local_search(
        SCIP *scip,               // SCIP instance
        SCIP_VAR **varlist,       // array of all variables
        SCIP_VAR **solution,      // array of all variables fixed in 1
        int *nInSolution,         // number of variables in solution
        ProfessorAux *profs_aux,  // auxiliar professor structures
        Instance *I,              // instance with all courses data
        int m,
        int n)
{
  int improved                   = 1;
  int iterations                 = 0;
  const int MAX_LOCAL_ITERATIONS = 10;  // limit local search iterations

  // build mapping: course_label -> assigned professor
  int *course_to_prof            = (int *) malloc(sizeof(int) * m);
  for (int i = 0; i < m; i++)
  {
    course_to_prof[i] = -1;
  }
  // fill mapping from current solution
  for (int i = 0; i < *nInSolution; i++)
  {
    const char *varname = SCIPvarGetName(solution[i]);
    int prof_id, course_id;
    if (sscanf(varname, "x#%d#%d", &prof_id, &course_id) == 2)
    {
      course_to_prof[course_id] = prof_id;
    }
  }

  // verify that all courses are assigned (solution must be complete)
  int num_assigned = 0;
  for (int c = 0; c < m; c++)
  {
    if (course_to_prof[c] >= 0) num_assigned++;
  }

  // nf solution is incomplete, abort local search
  if (num_assigned < m)
  {

    free(course_to_prof);

    // return current objective without changes
    SCIP_Real obj_value = 0.0;
    for (int i = 0; i < *nInSolution; i++)
    {
      obj_value += SCIPvarGetObj(solution[i]);
    }
    return obj_value;
  }

  while (improved && iterations < MAX_LOCAL_ITERATIONS)
  {
    improved = 0;
    iterations++;

    // try swap movements: reassign courser to different professors
    for (int c1 = 0; c1 < m && !improved; c1++)
    {
      int prof1 = course_to_prof[c1];
      if (prof1 < 0) continue;  // course not assigned

      // access course directly from instance using label (c1 == label)
      Course *course1 = &I->courses[c1];

      // try to reassign course1 to a different professor
      for (int prof2 = 0; prof2 < n && !improved; prof2++)
      {
        if (prof2 == prof1) continue;

        // check if prof2 can take course1 (capacity constraints)
        int can_assign = 0;
        if (course1->semester == 1)
        {
          if (profs_aux[prof2].professor->current_CH1 + course1->workload <= profs_aux[prof2].professor->maxWorkload1 &&
              profs_aux[prof1].professor->current_CH1 - course1->workload >= 0)
          {
            can_assign = 1;
          }
        }
        else
        {
          if (profs_aux[prof2].professor->current_CH2 + course1->workload <= profs_aux[prof2].professor->maxWorkload2 &&
              profs_aux[prof1].professor->current_CH2 - course1->workload >= 0)
          {
            can_assign = 1;
          }
        }

        if (!can_assign) continue;

        // check if this swap could improve objective (prefer higher preference weights)
        double weight_prof1 = check_preference_weight(profs_aux[prof1].professor->preferences, m, c1);
        double weight_prof2 = check_preference_weight(profs_aux[prof2].professor->preferences, m, c1);

        // only swap if prof2 has higher preference than prof1
        if (weight_prof2 > weight_prof1 + EPSILON)
        {
          // check minWorkload constraint: ensure prof1 won't fall below minWorkload after removal
          int prof1_total_after = profs_aux[prof1].professor->current_CH1 + profs_aux[prof1].professor->current_CH2 - course1->workload;
          if (prof1_total_after < profs_aux[prof1].professor->minWorkload)
          {
            continue;  // skip this swap to avoid violating minWorkload
          }

          // perform swap
          if (course1->semester == 1)
          {
            profs_aux[prof1].professor->current_CH1 -= course1->workload;
            profs_aux[prof2].professor->current_CH1 += course1->workload;
          }
          else
          {
            profs_aux[prof1].professor->current_CH2 -= course1->workload;
            profs_aux[prof2].professor->current_CH2 += course1->workload;
          }

          // update mapping
          course_to_prof[c1] = prof2;
          improved           = 1;
        }
      }
    }
  }

  // rebuild solution array from updated mapping
  *nInSolution = 0;
  for (int c = 0; c < m; c++)
  {
    int prof = course_to_prof[c];
    if (prof >= 0)
    {
      solution[(*nInSolution)] = varlist[prof * m + c];
      (*nInSolution)++;
    }
  }

  free(course_to_prof);

  // calculate objective value of improved solution
  SCIP_Real obj_value = 0.0;
  for (int i = 0; i < *nInSolution; i++)
  {
    obj_value += SCIPvarGetObj(solution[i]);
  }

  return obj_value;
}

int grasp(SCIP *scip, SCIP_SOL **sol, SCIP_HEUR *heur)
{
  int found, infeasible, nInSolution, nInBestSolution;
  unsigned int stored;
  int nvars;
  int *covered, n, m, nCovered;
  SCIP_VAR *var, **solution, **best_solution, **varlist;
  SCIP_Real valor, bestUb, best_obj_value;
  SCIP_PROBDATA *probdata;
  int i, k;
  Instance *I;

  // GRASP parameters (read from configuration file)
  const int MAX_ITERATIONS   = param.grasp_max_iter;
  const float ALPHA          = param.grasp_alpha;
  const int USE_LOCAL_SEARCH = param.grasp_local_search;  // flag to enable/disable local search phase

  // auxiliary structures for GRASP
  ProfessorAux *profs_aux;
  CourseAux *courses_aux;

  found      = 0;
  infeasible = 0;

  /* recover the problem data */
  probdata   = SCIPgetProbData(scip);
  assert(probdata != NULL);

  nvars       = SCIPprobdataGetNVars(probdata);
  varlist     = SCIPprobdataGetVars(probdata);
  I           = SCIPprobdataGetInstance(probdata);
  n           = I->nProfessors;  // number of professors
  m           = I->nCourses;     // number of courses

  // allocate auxiliary structures (shared across iterations)
  profs_aux   = (ProfessorAux *) malloc(sizeof(ProfessorAux) * n);
  courses_aux = (CourseAux *) malloc(sizeof(CourseAux) * m);

  // initialize auxiliary professor structures
  for (i = 0; i < n; i++)
  {
    profs_aux[i].professor = &I->professors[i];
    profs_aux[i].n         = 0;
    profs_aux[i].pref      = (PreferenceAux *) malloc(sizeof(PreferenceAux) * m);
  }

  // initialize auxiliary course structures
  for (i = 0; i < m; i++)
  {
    courses_aux[i].course       = &I->courses[i];
    courses_aux[i].covered      = 0;
    courses_aux[i].current_prof = -1;
  }

  solution        = (SCIP_VAR **) malloc(sizeof(SCIP_VAR *) * (m * n));
  best_solution   = (SCIP_VAR **) malloc(sizeof(SCIP_VAR *) * (m * n));
  covered         = (int *) calloc(m, sizeof(int));

  // initialize best solution as infinity
  best_obj_value  = -SCIPinfinity(scip);
  nInBestSolution = 0;

  // identify variables already fixed to 1.0 (these must be in every solution)
  int nFixed      = 0;
  for (i = 0; i < nvars; i++)
  {
    var = varlist[i];
    if (SCIPvarGetLbLocal(var) > 1.0 - EPSILON)
    {  // var >= 1.0
      nFixed++;
#ifdef DEBUG_GRASP
      printf("\nFixed var= %s", SCIPvarGetName(var));
#endif
    }
  }

  // GRASP main loop - iterate maxitr times
  for (k = 1; k <= MAX_ITERATIONS; k++)
  {
#ifdef DEBUG_GRASP
    printf("\n--- GRASP Iteration %d/%d ---\n", k, MAX_ITERATIONS);
#endif

    // reset structures for new iteration
    nInSolution = 0;
    nCovered    = 0;
    memset(covered, 0, m * sizeof(int));

    // reset professor workloads for new iteration
    for (i = 0; i < n; i++)
    {
      profs_aux[i].professor->current_CH1 = 0;
      profs_aux[i].professor->current_CH2 = 0;
    }

    // reset courses_aux to original order and state
    for (i = 0; i < m; i++)
    {
      courses_aux[i].course       = &I->courses[i];  // restore original order
      courses_aux[i].covered      = 0;
      courses_aux[i].current_prof = -1;
    }

    //include fixed variables in current solution
    int nFixedInIteration = 0;
    for (i = 0; i < nvars; i++)
    {
      var = varlist[i];
      if (SCIPvarGetLbLocal(var) > 1.0 - EPSILON)
      {  // var >= 1.0
        solution[nInSolution++] = var;
        nFixedInIteration++;

        // extract professor and course indices from variable name
        const char *varname = SCIPvarGetName(var);
        int prof_id, course_id;
        if (sscanf(varname, "x#%d#%d", &prof_id, &course_id) == 2)
        {
          covered[course_id] = 1;
          nCovered++;

          // update course_aux to mark as covered
          // find the course in courses_aux array (may be reordered, so search by label)
          for (int c = 0; c < m; c++)
          {
            if (courses_aux[c].course->label == course_id)
            {
              courses_aux[c].covered      = 1;
              courses_aux[c].current_prof = prof_id;
              break;
            }
          }

          // update professor workload
          Course *fixed_course = &I->courses[course_id];
          if (fixed_course->semester == 1)
          {
            profs_aux[prof_id].professor->current_CH1 += fixed_course->workload;
          }
          else
          {
            profs_aux[prof_id].professor->current_CH2 += fixed_course->workload;
          }
        }
      }
    }

    // construction phase - build a randomized greedy solution
    reset_course_level(courses_aux, m);
    adaptive_edges(profs_aux, courses_aux, n, m);
    qsort(courses_aux, m, sizeof(CourseAux), compareCourses);

    construct_solution(scip, varlist, solution, profs_aux, courses_aux, &nInSolution, &nCovered, m, n, ALPHA);

    // local search phase (try) improve constructed solution
    if (USE_LOCAL_SEARCH && nInSolution > 0)
    {
      SCIP_Real local_obj = local_search(scip, varlist, solution, &nInSolution, profs_aux, I, m, n);
#ifdef DEBUG_GRASP
      printf("After local search: nInSolution=%d, obj=%.2f\n", nInSolution, local_obj);
#endif
    }

    // Calculate objective value of current solution
    SCIP_SOL *current_sol;
    SCIP_CALL(SCIPcreateSol(scip, &current_sol, heur));

    for (i = 0; i < nInSolution; i++)
    {
      SCIP_CALL(SCIPsetSolVal(scip, current_sol, solution[i], 1.0));
    }

    valor = SCIPsolGetOrigObj(current_sol);

#ifdef DEBUG_GRASP
    printf("Current solution objective: %.4f (best so far: %.4f)\n", valor, best_obj_value);
#endif

    // Step 5: Update best solution if current is better
    if (valor > best_obj_value + EPSILON)
    {
      best_obj_value  = valor;
      nInBestSolution = nInSolution;

      // Copy current solution to best solution
      for (i = 0; i < nInSolution; i++)
      {
        best_solution[i] = solution[i];
      }

#ifdef DEBUG_GRASP
      printf("*** New best solution found! Objective: %.4f ***\n", best_obj_value);
#endif
    }

    // Free current iteration solution
    SCIP_CALL(SCIPfreeSol(scip, &current_sol));
  }

  // ================= END OF GRASP ===============================================

  // After all iterations, store the best solution found
  if (nInBestSolution > 0)
  {
    /* create SCIP solution structure sol */
    SCIP_CALL(SCIPcreateSol(scip, sol, heur));

    // save best solution in sol
    for (i = 0; i < nInBestSolution; i++)
    {
      var = best_solution[i];
      SCIP_CALL(SCIPsetSolVal(scip, *sol, var, 1.0));
    }

    // compute objective value of the best solution
    bestUb = SCIPgetPrimalbound(scip);
    valor  = SCIPsolGetOrigObj(*sol);

#ifdef DEBUG_GRASP
    printf("\n=== GRASP finished ===\n");
    printf("Best solution value: %.4f, Current upper bound: %.4f\n", valor, bestUb);
#endif

#ifdef DEBUG_GRASP
    printf("Trying to store solution...\n");
    SCIP_CALL(SCIPprintSol(scip, *sol, NULL, FALSE));
#endif

    /* check if the solution is feasible */
    SCIP_CALL(SCIPtrySol(scip, *sol, FALSE, FALSE, TRUE, TRUE, TRUE, &stored));
    if (stored)
    {
#ifdef DEBUG_GRASP
      printf("\nSolution is feasible and was stored! Total assignments = %d\n", nInBestSolution);
#endif
      found = 1;
    }
    else
    {
      found = 0;
#ifdef DEBUG_GRASP
      printf("\nSolution was not stored (rejected by SCIP). BestUb=%.4f\n", bestUb);
#endif
    }
  }

  // Free auxiliary structures
  for (i = 0; i < n; i++)
  {
    free(profs_aux[i].pref);
  }
  free(profs_aux);
  free(courses_aux);
  free(solution);
  free(best_solution);
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