#include "utils.h"
#include "heur_lns.h"
#include "parameters_dpd.h"
#include "probdata_dpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

int mCourses = 0;

int comparePreferences(const void *a, const void *b)
{
  Preference *prefA = (Preference *) a;
  Preference *prefB = (Preference *) b;

  if (prefA->weight > prefB->weight)
    return 1;
  if (prefA->weight < prefB->weight)
    return -1;
  return 0;
}

int compareProfessors(const void *a, const void *b)
{
  Professor *profA = (Professor *) a;
  Professor *profB = (Professor *) b;

  // sort preferences in descending order
  qsort(profA->preferences, mCourses, sizeof(Preference), comparePreferences);

  if (profA->avgPreferenceWeight < profB->avgPreferenceWeight)
    return 1;
  if (profA->avgPreferenceWeight > profB->avgPreferenceWeight)
    return -1;
  return 0;
}

int compareCandidates(const void *a, const void *b)
{
  LNS_Candidate *candA = (LNS_Candidate *) a;
  LNS_Candidate *candB = (LNS_Candidate *) b;

  Professor *profA     = (Professor *) candA->professor_ptr;
  Professor *profB     = (Professor *) candB->professor_ptr;

  // sort preferences in descending order (default behavior - maior para menor)
  qsort(profA->preferences, mCourses, sizeof(Preference), comparePreferences);

  if (profA->avgPreferenceWeight > profB->avgPreferenceWeight)
    return -1;  // Changed from 1 to -1 for descending order
  if (profA->avgPreferenceWeight < profB->avgPreferenceWeight)
    return 1;   // Changed from -1 to 1 for descending order
  return 0;
}

int compareCandidatesAscending(const void *a, const void *b)
{
  LNS_Candidate *candA = (LNS_Candidate *) a;
  LNS_Candidate *candB = (LNS_Candidate *) b;

  Professor *profA     = (Professor *) candA->professor_ptr;
  Professor *profB     = (Professor *) candB->professor_ptr;

  // sort preferences in ascending order (menor para maior)
  qsort(profA->preferences, mCourses, sizeof(Preference), comparePreferences);

  if (profA->avgPreferenceWeight < profB->avgPreferenceWeight)
    return -1;
  if (profA->avgPreferenceWeight > profB->avgPreferenceWeight)
    return 1;
  return 0;
}

void printOrderedProfessors(Professor *P, int nProfessors)
{
  printf("-------------------------\n");
  // print professors ordered by avgPreferenceWeight
  for (int i = 0; i < nProfessors; i++)
  {
    printf("%s: avgPreferenceWeight=%.2f\n", P[i].name, P[i].avgPreferenceWeight);
    // print preferences
    for (int j = 0; j < mCourses; j++)
    {
      if (P[i].preferences[j].weight >= EPSILON)
        printf("Professor %s eligible for course %d with weight %f\n", P[i].name, P[i].preferences[j].course_ptr->label, P[i].preferences[j].weight);
    }
  }
}
