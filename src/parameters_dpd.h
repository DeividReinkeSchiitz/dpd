#ifndef __SCIP_PARAMETERS_MOCHILA__
#define __SCIP_PARAMETERS_MOCHILA__

#define MAXINT 1000
typedef struct
{
  // global settings
  int time_limit;   /* limit of execution time (in sec). Default = 1800 (-1: unlimited) */
  int display_freq; /* frequency to display information about B&B enumeration. Default = 50 (-1: never) */
  int nodes_limit;  /* limit of nodes to B&B procedure. Default = -1: unlimited (1: onlyrootnode) */

  // parameter stamp
  char parameter_stamp[100]; /* fixed-size buffer for parameter stamp */

  // primal heuristic
  int heur_bad_sol;
  int heur_rounding;
  int heur_round_freq;
  int heur_round_maxdepth;
  int heur_round_freqofs;
  int area_penalty;
  int heur_lns;
  double lns_perc;
  int lns_time;
  char lns_order[20]; /* order for LNS candidates: "crescente" or "decrescente". Default = "decrescente" */
  int heur_rf;
  int heur_grasp;

  // GRASP parameters
  int grasp_max_iter;     // max iterations for GRASP. Default = 10
  double grasp_alpha;     // alpha parameter for GRASP RCL. Default = 0.4 
  int grasp_local_search; // enable/disable local search in GRASP. Default = 1
} parametersT;

int setParameters(int argc, char **argv, parametersT *Param);

extern parametersT param;
extern char current_path[512];
#endif
