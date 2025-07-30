/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*      This file is based on other part of the program and library          */
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

/**@file   cmain.c
 * @brief  It is an example of a branch-and-bound code using SCIP as library 
 *
 * @author Edna Hoshino (based on template codified by Timo Berthold and Stefan Heinz)
 *
 * This is an example for solving the knapsack problem. 
 * The goal of this problem is finding a subset of items from a input set,
 * to maximize the sum of the values of selected items subject to their weights do not exceed a given capacity C.
 * We also use a naive primal heuristic to generate a feasible solution quickly.
 * 
 **/

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "heur_badFeasible.h"
#include "heur_gulosa.h"
#include "heur_lns.h"

#include "parameters_dpd.h"
#include "probdata_dpd.h"
#include "problem.h"
#include "scip/scip.h"
#include "scip/scipdefplugins.h"

parametersT param;
extern char output_path[256];

//
void removePath(char *fullfilename, char **filename);
void configOutputName(char *name, char *instance_filename, char *program);
SCIP_RETCODE printStatistic(SCIP *scip, double time, char *outputname);
void printSol(SCIP *scip, char *outputname);
SCIP_RETCODE configScip(SCIP **pscip, parametersT param);

//
SCIP_RETCODE printStatistic(SCIP *scip, double time, char *outputname)
{
  SCIP_Bool outputorigsol = TRUE;
  SCIP_SOL *bestSolution  = NULL;
  char filename[SCIP_MAXSTRLEN];
  FILE *fout;
  SCIP_HEUR *heur_hdlr;

  // Use output_path for output file location
  sprintf(filename, "%s/%s.out", output_path, outputname);
  fout = fopen(filename, "w");
  if (!fout)
  {
    printf("\nProblem to create file %s\n", filename);
    return 1;
  }

  /* I found the commands for those statistical information looking the scip source code at file scip.c (printPricerStatistics(), for instance)  */
  bestSolution = SCIPgetBestSol(scip);
  if (outputorigsol)
  {
    if (bestSolution == NULL)
      printf("\nno solution available\n");
    else
    {
      SCIP_SOL *origsol;
      SCIP_CALL(SCIPcreateSolCopy(scip, &origsol, bestSolution));
      SCIP_CALL(SCIPretransformSol(scip, origsol));
      SCIP_CALL(SCIPprintSol(scip, origsol, NULL, FALSE));
      SCIP_CALL(SCIPfreeSol(scip, &origsol));
    }
  }
  else
  {
    SCIP_CALL(SCIPprintBestSol(scip, NULL, FALSE));
  }
  SCIPinfoMessage(scip, NULL, "\nStatistics\n");
  SCIPinfoMessage(scip, NULL, "==========\n\n");
  SCIP_CALL(SCIPprintStatistics(scip, NULL));
  if (fout != NULL)
  {
    fprintf(fout, "%s;%lli;%lf;%lf;%lf;%lf;%lf;%lli;%d;%lf;%lf;%lli;%d;%d", SCIPgetProbName(scip), SCIPgetNRootLPIterations(scip), time, SCIPgetDualbound(scip), SCIPgetPrimalbound(scip), SCIPgetGap(scip), SCIPgetDualboundRoot(scip), SCIPgetNTotalNodes(scip), SCIPgetNNodesLeft(scip), SCIPgetSolvingTime(scip), SCIPgetTotalTime(scip), SCIPgetMemUsed(scip), SCIPgetNLPCols(scip), SCIPgetStatus(scip));
    if (bestSolution != NULL)
    {
      fprintf(fout, ";bestsol in %lld;%lf;%d;%s", SCIPsolGetNodenum(bestSolution), SCIPsolGetTime(bestSolution), SCIPsolGetDepth(bestSolution), SCIPsolGetHeur(bestSolution) != NULL ? SCIPheurGetName(SCIPsolGetHeur(bestSolution)) : (SCIPsolGetRunnum(bestSolution) == 0 ? "initial" : "relaxation"));
    }
    if (param.heur_lns)
    {
      heur_hdlr = SCIPfindHeur(scip, "lns");
      fprintf(fout, ";%lf;%lld;%lld;%lld;%s", SCIPheurGetTime(heur_hdlr), SCIPheurGetNCalls(heur_hdlr), SCIPheurGetNSolsFound(heur_hdlr), SCIPheurGetNBestSolsFound(heur_hdlr), SCIPheurGetName(heur_hdlr));
    }
    if (param.heur_bad_sol)
    {
      heur_hdlr = SCIPfindHeur(scip, "badFeasibleSolution");
      fprintf(fout, ";%lf;%lld;%lld;%lld;%s", SCIPheurGetTime(heur_hdlr), SCIPheurGetNCalls(heur_hdlr), SCIPheurGetNSolsFound(heur_hdlr), SCIPheurGetNBestSolsFound(heur_hdlr), SCIPheurGetName(heur_hdlr));
    }
    if (param.heur_gulosa)
    {
      heur_hdlr = SCIPfindHeur(scip, "gulosa");
      fprintf(fout, ";%lf;%lld;%lld;%lld;%s", SCIPheurGetTime(heur_hdlr), SCIPheurGetNCalls(heur_hdlr), SCIPheurGetNSolsFound(heur_hdlr), SCIPheurGetNBestSolsFound(heur_hdlr), SCIPheurGetName(heur_hdlr));
    }

    fprintf(fout, ";%s\n", param.parameter_stamp);
  }
  fclose(fout);
  return SCIP_OKAY;
}

/** 
 * creates a SCIP instance with default plugins, and set SCIP parameters 
 */
SCIP_RETCODE configScip(
        SCIP **pscip,
        parametersT param)
{

  SCIP *scip           = NULL;
  /* initialize SCIP */
  SCIP_RETCODE retcode = SCIPcreate(&scip);
  if (retcode != SCIP_OKAY || scip == NULL)
  {
    printf("SCIPcreate failed: retcode=%d, scip=%p\n", retcode, (void *) scip);
    return 1;
  }

  /* include default SCIP plugins */
  SCIP_CALL(SCIPincludeDefaultPlugins(scip));
  /* for column generation, disable restarts */
  SCIP_CALL(SCIPsetIntParam(scip, "presolving/maxrestarts", 0));
  /* disable presolving */
  SCIP_CALL(SCIPsetPresolving(scip, SCIP_PARAMSETTING_OFF, TRUE));  // turn off
  /* turn off all separation algorithms */
  SCIP_CALL(SCIPsetSeparating(scip, SCIP_PARAMSETTING_OFF, TRUE));  // turn off
  /* disable heuristics */
  SCIP_CALL(SCIPsetHeuristics(scip, SCIP_PARAMSETTING_OFF, TRUE));  // turn off
  /* for column generation, usualy we prefer branching using pscost instead of relcost  */
  SCIP_CALL(SCIPsetIntParam(scip, "branching/pscost/priority", 1000000));

  SCIP_CALL(SCIPsetIntParam(scip, "display/freq", param.display_freq));
  /* set time limit */
  SCIP_CALL(SCIPsetRealParam(scip, "limits/time", param.time_limit));
  // for only root, use 1
  SCIP_CALL(SCIPsetLongintParam(scip, "limits/nodes", param.nodes_limit));

  // active heuristic of rounding
  if (param.heur_bad_sol)
    SCIP_CALL(SCIPincludeHeurBadFeasibleSolution(scip));
  if (param.heur_lns)
    SCIP_CALL(SCIPincludeHeurLns(scip));
  // if (param.heur_rounding)
  // SCIP_CALL(SCIPincludeHeurMyRounding(scip));
  if (param.heur_gulosa)
    SCIP_CALL(SCIPincludeHeurGulosa(scip));

  *pscip = scip;

  return SCIP_OKAY;
}
// TODO: Get the best solution found and write the solution in a file. It depends on the problem!
void printSol(SCIP *scip, char *outputname)
{
  SCIP_PROBDATA *probdata;
  SCIP_SOL *bestSolution;
  SCIP_VAR **vars;
  SCIP_Real solval;
  FILE *file;
  int v, nvars;
  Instance *I;
  char filename[SCIP_MAXSTRLEN];
  struct tm *ct;
  const time_t t = time(NULL);

  assert(scip != NULL);
  bestSolution = SCIPgetBestSol(scip);
  if (bestSolution == NULL)
    return;
  probdata = SCIPgetProbData(scip);
  assert(probdata != NULL);

  I     = SCIPprobdataGetInstance(probdata);
  nvars = SCIPprobdataGetNVars(probdata);
  vars  = SCIPprobdataGetVars(probdata);

  (void) SCIPsnprintf(filename, SCIP_MAXSTRLEN, "%s/%s.sol", output_path, outputname);
  file = fopen(filename, "w");
  if (!file)
  {
    printf("\nProblem to create solution file: %s", filename);
    return;
  }
  fprintf(file, "\nValue: %lf\n", -SCIPsolGetOrigObj(bestSolution));

  for (int i = 0; i < I->nProfessors; i++)
  {
    fprintf(file, "%d - %s:\n", i + 1, I->professors[i].name);
    int hours = 0;
    int count = 0;
    int sum   = 0;
    float avg;
    for (int j = 0; j < I->professors[i].numPreferences; j++)
    {
      solval = SCIPgetSolVal(scip, bestSolution, vars[(i * (I->nCourses)) + j]);
      if (solval > EPSILON)
      {
        count++;
        sum += I->professors[i].preferences[j].weight;
        fprintf(file, "%d: %s (%d)\n", j + 1, I->courses[j].subject.name, I->professors[i].preferences[j].weight);
        hours += I->courses[j].workload;
      }
    }

    if (count > 0)
    {
      avg = (float) sum / count;
    }
    else
    {
      avg = 0;
    }

    fprintf(file, "Average weight assigned by the system: %.2f\n", avg);
    fprintf(file, "Average weight assigned by the professor: %.2f\n", I->professors[i].avgPreferenceWeight);
    fprintf(file, "Satisfaction coefficient: %.2f\n\n", I->professors[i].avgPreferenceWeight);
  }

  fprintf(file, "\n");
  //
  fprintf(file, "Parameters settings file=%s\n", param.parameter_stamp);
  fprintf(file, "Instance file=%s\n", SCIPgetProbName(scip));
  ct = localtime(&t);
  fprintf(file, "Date=%d-%.2d-%.2d\nTime=%.2d:%.2d:%.2d\n", ct->tm_year + 1900, ct->tm_mon, ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);
  fclose(file);
}

void removePath(char *fullfilename, char **filename)
{
  // remove path, if there exists on fullfilename
  *filename = strrchr(fullfilename, '/');
  if (*filename == NULL)
  {
    *filename = fullfilename;
  }
  else
  {
    (*filename)++;  // discard /
  }
}
void configOutputName(char *name, char *instance_filename, char *program)
{
  char *program_filename, *filename;

  // remove path, if there exists on program name
  removePath(program, &program_filename);
  removePath(instance_filename, &filename);

  // append program name and parameter stamp
  (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "%s/%s-%s-%s", current_path, filename, program_filename, param.parameter_stamp);
}

int main(int argc, char **argv)
{
  SCIP *scip;
  Instance *in;
  clock_t start, end;
  char outputname[SCIP_MAXSTRLEN];

  // set default+user parameters
  if (!setParameters(argc, argv, &param))
    return 0;

  // load instance file
  if (!loadInstance(argv[1], &in, param.area_penalty))
  {
    printf("\nProblem to read instance file %s\n", argv[1]);
    return 1;
  }
  //  printInstance(in);
  // create scip and set scip configurations
  configScip(&scip, param);
  // load problem into scip
  if (!loadProblem(scip, argv[1], in, 0, NULL))
  {
    printf("\nProblem to load instance problem\n");
    return 1;
  }
  // print problem
  char dpd_lp_path[256];
  snprintf(dpd_lp_path, sizeof(dpd_lp_path), "%s/dpd.lp", output_path);
  SCIP_CALL(SCIPwriteOrigProblem(scip, dpd_lp_path, "lp", TRUE));
  // solve scip problem
  start = clock();
  SCIP_CALL(SCIPsolve(scip));
  end = clock();
  // config output filename
  configOutputName(outputname, argv[1], argv[0]);
  // print statistics and print resume in output file
  printStatistic(scip, ((double) (end - start)) / CLOCKS_PER_SEC, outputname);
  // write the best solution in a file
  printSol(scip, outputname);
  //SCIP_CALL( SCIPfree(&scip) );
  BMScheckEmptyMemory();
  return 0;
}
