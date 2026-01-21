#ifndef UTILS_H
#define UTILS_H

#include "heur_problem.h"

extern int mCourses;

int comparePreferences(const void *a, const void *b);
int compareProfessors(const void *a, const void *b);
int compareCandidates(const void *a, const void *b);
int compareCandidatesAscending(const void *a, const void *b);
void printOrderedProfessors(Professor *P, int nProfessors);

#endif  // UTILS_H
