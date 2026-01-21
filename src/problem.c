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
    if (I->professors)
    {
      for (int i = 0; i < I->nProfessors; i++)
      {
        if (I->professors[i].preferences)
        {
          free(I->professors[i].preferences);
          I->professors[i].preferences = NULL;
        }
      }
      free(I->professors);
      I->professors = NULL;
    }
    if (I->courses)
    {
      free(I->courses);
      I->courses = NULL;
    }
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
    printf("%d;%d;T%d;%s;%s;%d;%u\n", i + 1, I->courses[i].semester, I->courses[i].number, I->courses[i].subject.name, I->courses[i].programs, I->courses[i].workload, I->courses[i].subject.areas);
  }
  printf("\nname;minWorkload;maxWorkload1;maxWorkload2;preferences;areas\n");
  for (int i = 0; i < I->nProfessors; i++)
  {
    printf("%s;%d;%d;%d;%d;%u\n", I->professors[i].name, I->professors[i].minWorkload, I->professors[i].maxWorkload1, I->professors[i].maxWorkload2, I->professors[i].numPreferences, I->professors[i].areas);
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

  // Clean the string by removing newlines and other whitespace
  char *clean_str      = str;
  while (*clean_str && (*clean_str == '\n' || *clean_str == '\r' || *clean_str == ' ' || *clean_str == '\t'))
    clean_str++;

  len = strlen(clean_str);
  // Remove trailing whitespace
  while (len > 0 && (clean_str[len - 1] == '\n' || clean_str[len - 1] == '\r' || clean_str[len - 1] == ' ' || clean_str[len - 1] == '\t'))
  {
    clean_str[len - 1] = '\0';
    len--;
  }

  for (int i = 0; i < len; i++)
  {
    if (clean_str[i] == '1' || clean_str[i] == '0')  // Only process valid binary digits
    {
      result <<= 1;  // Shift left by 1 bit
      if (clean_str[i] == '1')
        result |= 1;  // Set the least significant bit to 1
    }
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
    sscanf(linha, "%*d;%d;T%d;%99[^;];%24[^;];%d;%99[^;]", &((*I)->courses[i].semester), &((*I)->courses[i].number), (*I)->courses[i].subject.name, (*I)->courses[i].programs, &(*I)->courses[i].workload, areas_str);
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
    // Use safer format for areas_str and cap p to m
    sscanf(linha, "%99[^;];%d;%d;%d;%d;%99[^;\n]", (*I)->professors[i].name, &((*I)->professors[i].minWorkload), &((*I)->professors[i].maxWorkload1), &((*I)->professors[i].maxWorkload2), &p, areas_str);
    if (p > m) p = m;  // Prevent buffer overflow if input is malformed
    (*I)->professors[i].areas          = str2bin(areas_str);
    (*I)->professors[i].label          = i;
    (*I)->professors[i].level          = 0;  // initializing the professors level with 0
    float sum                          = 0;
    (*I)->professors[i].numPreferences = p;
    for (int j = 0; j < p && j < m; j++)  // Ensure we do not write past preferences array
    {
      fgets(linha, sizeof(linha), f);
      int courseIndex, weight;
      sscanf(linha, "%d;%d", &courseIndex, &weight);
      if (courseIndex > 0 && courseIndex <= m)
      {
        // Store preference in the course-indexed position, not file-order position
        int courseIdx                                         = courseIndex - 1;  // Convert 1-based to 0-based index
        (*I)->professors[i].preferences[courseIdx].weight     = weight;
        (*I)->professors[i].preferences[courseIdx].course_ptr = &(*I)->courses[courseIdx];
        sum += weight;
      }
    }

    // Professors who have no weight for a course but share the area with the course get EPSILON weight for the greedy heuristic
    for (int j = 0; j < m; j++)
    {
      if (checaArea((*I)->professors[i].areas, (*I)->courses[j].subject.areas) == 1)
      {
        if ((*I)->professors[i].preferences[j].weight == 0)
          (*I)->professors[i].preferences[j].weight = EPSILON;
      }
      // Only set course_ptr if it hasn't been set already
      if ((*I)->professors[i].preferences[j].course_ptr == NULL)
      {
        (*I)->professors[i].preferences[j].course_ptr = &(*I)->courses[j];
      }
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

  ret_code = SCIPprobdataCreate(scip, probname, I, relaxed, fixed, TRUE);
  if (ret_code != SCIP_OKAY)
    return 0;
  return 1;
}

int loadProblemSubSCIP(SCIP *scip, char *probname, Instance *I, int relaxed, int *fixed)
{
  SCIP_RETCODE ret_code;

  ret_code = SCIPprobdataCreate(scip, probname, I, relaxed, fixed, FALSE);
  if (ret_code != SCIP_OKAY)
    return 0;
  return 1;
}