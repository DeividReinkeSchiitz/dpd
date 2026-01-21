#ifndef __PROBLEM__
#define __PROBLEM__
#include "scip/scip.h"
#include <stdint.h>
#include <stdio.h>

// Area type (32 bits)
typedef uint32_t Area;

// Subject structure
typedef struct
{
  char name[100];    // name of the subject
  char acronym[20];  // acronym of the subject
  Area areas;        // 32 bits // areas of the subject, represented as a bitmask
} Subject;

// Course structure
typedef struct
{
  int label;        // label of the course. This is used to identify the course in the problem.
  Subject subject;  // subject of the course
  int number;       // number of the course
  int workload;     // workload of the course
  int level;        // level of the Course vertex in the graph based on the number of professors that can teach it
  enum Period
  {
    morning,
    afternoon,
    night
  } period;           // period of the course
  int semester;       // 1 or 2
  char programs[25];  // programs that the course belongs to
} Course;

// Preference structure
typedef struct Preference
{
  Course *course_ptr;  // pointer to the course
  double weight;       // weight of the preference
} Preference;

// Professor structure
typedef struct
{
  int label;         // label of the professor. This is used to identify the professor in the problem.
  int minWorkload;   // minimum workload from the year
  int maxWorkload1;  // maximum workload for first semester
  int maxWorkload2;  // maximum workload for second semester
  int active;        // boolean indicating if the professor is active
  char name[100];    // name of the professor
  Area areas;        // 32 bits // areas of the professor, represented as a bitmask
  //char *myareas;

  // preferences is a dynamically allocated array of Preference, with one entry per course
  int numPreferences;         // number of preferences
  Preference *preferences;    // array of preferences for the courses
  float avgPreferenceWeight;  // average weight of the preferences

  // Portuguese field names for backward compatibility with GRASP heuristic
  int current_CH1;  // alias for currentWorkload1
  int current_CH2;  // alias for currentWorkload2
  int level;        // alias for numSuitableCourses
} Professor;

// Instance structure
typedef struct
{
  int nProfessors;  // number of professors
  int nCourses;     // number of courses
  int nAreas;       // number of different areas in the file
  int *C;
  Professor *professors;  // array of professors
  Course *courses;        // array of courses
  int area_penalty;       // penalty for not assigning a course to a professor in the same area
} Instance;

void freeInstance(Instance *I);
void createInstance(Instance **I, int nProfessors, int nCourses, int nAreas);
void printInstance(Instance *I);
// load instance from a file
int loadInstance(char *filename, Instance **I, int area_penalty);
// load instance problem into SCIP
int loadProblem(SCIP *scip, char *probname, Instance *instance, int relaxed, int *fixed);

// Load problem for sub-SCIP (doesn't own instance)
int loadProblemSubSCIP(SCIP *scip, char *probname, Instance *instance, int relaxed, int *fixed);
#endif
