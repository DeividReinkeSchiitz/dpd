/**@file   problem.c
 * @brief  This file contains routines specific for the problem and the functions loadInstance(), freeInstance, 
 * printInstance, and loadProblem must be implemented 
 *
 **/
#include "probdata_dpd.h"
#include "scip/scip.h"
#include <math.h>

void freeInstance(Instance *I)
{
  if (I)
  {
    for (int i = 0; i < I->nProfessors; i++)
    {
      free(I->professors[i].preferences);
    }
    free(I->professors);
    free(I->courses);
    free(I);
    I = NULL;
  }
}
void createInstance(Instance **I, int nProfessors, int nCourses, int numAreas)
{
  *I               = (Instance *) malloc(sizeof(Instance));
  (*I)->professors = (Professor *) malloc(sizeof(Professor) * nProfessors);
  (*I)->courses    = (Course *) malloc(sizeof(Course) * nCourses);

  // allocate preferences for professors
  for (int i = 0; i < nProfessors; i++)
  {
    (*I)->professors[i].preferences = (Preference *) calloc(nCourses, sizeof(Preference));
  }

  (*I)->nProfessors = nProfessors;
  (*I)->nCourses    = nCourses;
  (*I)->nAreas      = numAreas;
}

void printInstance(Instance *I)
{
  printf("%d;%d;%d\n", I->nCourses, I->nProfessors, I->nAreas);
  printf("code;semester;number;name;program;workload;area\n");
  for (int i = 0; i < I->nCourses; i++)
  {
    printf("%d;%d;T%d;%s;%s;%d;%lu\n", i + 1, I->courses[i].semester, I->courses[i].number, I->courses[i].subject.name, I->courses[i].programs, I->courses[i].workload, I->courses[i].subject.areas);
  }
  printf("\nname;minWorkload;maxWorkload1;maxWorkload2;preferences;areas\n");
  for (int i = 0; i < I->nProfessors; i++)
  {
    printf("%s;%d;%d;%d;%d;%lu\n", I->professors[i].name, I->professors[i].minWorkload, I->professors[i].maxWorkload1, I->professors[i].maxWorkload2, I->professors[i].numPreferences, I->professors[i].areas);
    for (int j = 0; j < I->professors[i].numPreferences; j++)
    {

      if (I->professors[i].preferences[j].weight != 0)
      {
        printf("%d;%.2f\n", j + 1, I->professors[i].preferences[j].weight);
      }
    }
  }
}

// Convert binary string to unsigned long
unsigned long str2bin(char *str)
{
  unsigned long result = 0;
  int len              = strlen(str);

  for (int i = 0; i < len; i++)
  {
    result <<= 1;  // Shift left by 1 bit
    if (str[i] == '1')
      result |= 1;  // Set the least significant bit to 1
  }
  return result;
}

int loadInstance(char *filename, Instance **I, int area_penalty)
{
  FILE *f;
  int n, m = 0, numareas;
  f = fopen(filename, "r");
  if (!f)
  {
    printf("\nProblem to open file %s\n", filename);
    return 0;
  }
  char linha[400];
  fgets(linha, sizeof(linha), f);
  sscanf(linha, "%d;%d;%d", &m, &n, &numareas);
  createInstance(I, n, m, numareas);
  (*I)->area_penalty  = area_penalty;
  char areas_str[100] = {0};  // Buffer to store the areas string
  // read header
  fgets(linha, sizeof(linha), f);

  for (int i = 0; i < m; i++)
  {
    fgets(linha, sizeof(linha), f);
    (*I)->courses[i].subject.areas = 0;
    sscanf(linha, "%*d;%d;T%d;%99[^;];%99[^;];%d;%99[^;]", &((*I)->courses[i].semester), &((*I)->courses[i].number), (*I)->courses[i].subject.name, (*I)->courses[i].programs, &(*I)->courses[i].workload, areas_str);
    (*I)->courses[i].subject.areas = str2bin(areas_str);  // Convert areas_str to binary representation
    (*I)->courses[i].label         = i;
  }

  fgets(linha, sizeof(linha), f);  // blank line
  fgets(linha, sizeof(linha), f);  // professors header

  // read professors
  for (int i = 0; i < n; i++)
  {
    fgets(linha, sizeof(linha), f);
    int p = 0;
    sscanf(linha, "%99[^;];%d;%d;%d;%d;%s", (*I)->professors[i].name, &((*I)->professors[i].minWorkload), &((*I)->professors[i].maxWorkload1), &((*I)->professors[i].maxWorkload2), &p, areas_str);
    (*I)->professors[i].areas          = str2bin(areas_str);
    (*I)->professors[i].numPreferences = p;
    (*I)->professors[i].label          = i;
    float sum                          = 0;
    for (int j = 0; j < p; j++)
    {
      fgets(linha, sizeof(linha), f);
      int courseIndex, weight;
      sscanf(linha, "%d;%d", &courseIndex, &weight);
      (*I)->professors[i].preferences[j].weight     = weight;
      (*I)->professors[i].preferences[j].course_ptr = &(*I)->courses[courseIndex - 1];
      sum += weight;
    }

    // Professors who have no weight for a course but share the area with the course get EPSILON weight for the greedy heuristic
    for (int j = 0; j < m; j++)
    {
      if (checaArea((*I)->professors[i].areas, (*I)->courses[j].subject.areas) == 1 && (*I)->professors[i].preferences[j].weight == 0)
      {
        (*I)->professors[i].preferences[j].weight = EPSILON;
      }
      (*I)->professors[i].preferences[j].course_ptr = &(*I)->courses[j];
    }

    if (p > 0)
    {
      (*I)->professors[i].avgPreferenceWeight = (sum / p) + (m / p);
    }
    else
    {
      (*I)->professors[i].avgPreferenceWeight = 0;
    }
  }
  return 1;
}

// load instance problem into SCIP
int loadProblem(SCIP *scip, char *probname, Instance *I, int relaxed, int *fixed)
{
  SCIP_RETCODE ret_code;

  ret_code = SCIPprobdataCreate(scip, probname, I, relaxed, fixed);
  if (ret_code != SCIP_OKAY)
    return 0;
  return 1;
}