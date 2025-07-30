#include "parameters_dpd.h"
#include "probdata_dpd.h"
#include <scip/scip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

char output_path[256]  = "";
char current_path[256] = ".";

void init_output_folder()
{
  time_t now         = time(NULL);
  struct tm *tm_info = localtime(&now);
  strftime(output_path, sizeof(output_path), "output/%Y-%m-%d_%H:%M:%S", tm_info);
  mkdir(output_path, 0777);
}

/**
 * set default+user parameters
 **/
int setParameters(int argc, char **argv, parametersT *pparam)
{
  typedef struct
  {
    const char *description;
    const char *param_name;
    void *param_var;
    enum
    {
      INT,
      DOUBLE,
      STRING
    } type;
    int ilb;
    int iub;
    double dlb;
    double dub;
    int idefault;
    double ddefault;
  } settingsT;

  enum
  {
    time_limit,
    display_freq,
    nodes_limit,
    param_stamp,
    param_output_path,
    heur_rounding,
    heur_round_freq,
    heur_round_depth,
    heur_round_freqofs,
    heur_gulosa,
    area_penalty
  };

  settingsT parameters[] = {
          {"time limit", "--time", &(param.time_limit), INT, 0, 7200, 0, 0, 1800, 0},
          {"display freq", "--display", &(param.display_freq), INT, -1, MAXINT, 0, 0, 50, 0},
          {"nodes limit", "--nodes", &(param.nodes_limit), INT, -1, MAXINT, 0, 0, -1, 0},
          {"param stamp", "--param_stamp", &(param.parameter_stamp), STRING, 0, 0, 0, 0, 0, 0},
          {"heur rounding", "--heur_rounding", &(param.heur_rounding), INT, 0, 1, 0, 0, 0, 0},
          {"heur bad Solution", "--heur_bad_sol", &(param.heur_bad_sol), INT, 0, 1, 0, 0, 0, 0},
          {"heur lns", "--heur_lns", &(param.heur_lns), INT, 0, 1, 0, 0, 0, 0},
          {"lns perc", "--lns_perc", &(param.lns_perc), DOUBLE, 0, 0, 0, 1.0, 0, 0.3},
          {"lns_time", "--lns_time", &(param.lns_time), INT, 0, 3600, 0, 0, 30, 0},
          {"heur round freq", "--heur_round_freq", &(param.heur_round_freq), INT, 0, MAXINT, 0, 0, 1, 0},
          {"heur round maxdepth", "--heur_round_depth", &(param.heur_round_maxdepth), INT, -1, MAXINT, 0, 0, -1, 0},
          {"heur round freqofs", "--heur_round_freqofs", &(param.heur_round_freqofs), INT, 0, MAXINT, 0, 0, 0, 0},
          {"heur gulosa", "--heur_gulosa", &(param.heur_gulosa), INT, 0, 1, 0, 0, 0, 0},
          {"area penalty", "--penalty", &(param.area_penalty), INT, -100, MAXINT, 0, 0, 0, 0}};
  int i, j, ivalue, error;
  double dvalue;
  FILE *fin;

  int total_parameters = sizeof(parameters) / sizeof(parameters[0]);

  if (pparam == NULL)
    return 0;

  // check arguments
  if (argc < 2)
  {
    printf("\nSintaxe: ./bin/program <instance-file> <parameters-setting>.\n\t or Use ./bin/program --options to show options to parameters settings.\nExample of usage:\n\t ./bin/program data/t50-10-1000-2-s-1.mochila\n\t ./bin/program data/t50-10-1000-2-s-1.mochila --heur_rounding 1 --heur_round_freq 10 --param_stamp rounding.config\n\nIf no param_stamp is given by user, a new param stamp named dAAAAMMDDhHHMMSS will be created.\n\nIf the given param stamp is new (it does not exist in the current folder), it will be created to save all chosen parameters settings. Otherwise, if the param stamp file already exists, all saved parameters settings in the file must be the same as those parameters settings given in the command line.\n\nP.S.: To use a given param stamp file "
           "x.config"
           ", please use the command xargs as follows:\n\n \t xargs -a x.config ./bin/program data/t50-10-1000-2-s-1.mochila\n");
    return 0;
  }
  else if (argc == 2 && !strcmp(argv[1], "--options"))
  {  // show options
    //    showOptions();
    printf("\noption                : default -   range        : description");
    for (i = 0; i < total_parameters; i++)
    {
      switch (parameters[i].type)
      {
        case INT:
          printf("\n%-22s: %7d - [%3d,%8d] : %s", parameters[i].param_name, parameters[i].idefault, parameters[i].ilb, parameters[i].iub, parameters[i].description);
          break;
        case DOUBLE:
          printf("\n%-22s: %7.1lf - [%3.1lf,%8.1lf] : %s", parameters[i].param_name, parameters[i].ddefault, parameters[i].dlb, parameters[i].dub, parameters[i].description);
          break;
        case STRING:
          printf("\n%-22s:       * - [*,*]          : %s", parameters[i].param_name, parameters[i].description);
      }
    }
    printf("\n");
    return 0;
  }

  // check the existance of instance file
  fin = fopen(argv[1], "r");
  if (!fin)
  {
    printf("\nInstance file not found: %s\n", argv[1]);
    return 0;
  }
  fclose(fin);

  // set default parameters value
  for (i = 0; i < total_parameters; i++)
  {
    if (parameters[i].type == INT)
      *((int *) (parameters[i].param_var)) = parameters[i].idefault;
    else if (parameters[i].type == DOUBLE)
      *((double *) (parameters[i].param_var)) = parameters[i].ddefault;
    else if (strcmp(parameters[i].param_name, "--output_path") == 0)
      strcpy((char *) parameters[i].param_var, "output");  // default value for output_path
    else
      *((char **) (parameters[i].param_var)) = "";
  }
  init_output_folder();

  // set user parameters value
  error = 0;
  for (i = 2; i < argc && !error; i += 2)
  {
    for (j = 0; j < total_parameters && strcmp(argv[i], parameters[j].param_name); j++);
    if (j >= total_parameters || i == argc - 1)
    {
      printf("\nParameter (%s) invalid or uncompleted.", argv[i]);
      error = 1;
    }
    else
    {
      switch (parameters[j].type)
      {
        case INT:
          ivalue = atoi(argv[i + 1]);
          if (ivalue < parameters[j].ilb || ivalue > parameters[j].iub)
          {
            printf("\nParameter (%s) value (%d) out of range [%d,%d].", argv[i], ivalue, parameters[j].ilb, parameters[j].iub);
            error = 1;
            //          break;
          }
          else
          {
            *((int *) (parameters[j].param_var)) = ivalue;
          }
          break;
        case DOUBLE:
          dvalue = atof(argv[i + 1]);
          if (dvalue < parameters[j].dlb || dvalue > parameters[j].dub)
          {
            printf("\nParameter (%s) value (%lf) out of range [%lf,%lf].", argv[i], dvalue, parameters[j].dlb, parameters[j].dub);
            error = 1;
            //          break;
          }
          else
          {
            *((double *) (parameters[j].param_var)) = dvalue;
          }
          break;
        case STRING:
          if (strcmp(parameters[j].param_name, "--output_path") == 0)
            strcpy((char *) parameters[j].param_var, argv[i + 1]);
          else
            *((char **) (parameters[j].param_var)) = argv[i + 1];
      }
    }
  }

  // print parameters
  printf("\n\n----------------------------\nParameters settings");
  for (i = 0; i < total_parameters; i++)
  {
    printf("\nparameter %s (%s): default value=", parameters[i].param_name, parameters[i].description);
    switch (parameters[i].type)
    {
      case INT:
        printf("%d - value = %d", parameters[i].idefault, *((int *) parameters[i].param_var));
        break;
      case DOUBLE:
        printf("%lf - value = %lf", parameters[i].ddefault, *((double *) parameters[i].param_var));
        break;
      case STRING:
        printf("(null) - value = %s", *((char **) parameters[i].param_var));
        break;
    }
  }

  printf("\nerror = %d\n", error);
  if (!error)
  {
    FILE *fout;
    char foutname[SCIP_MAXSTRLEN];

    if (param.parameter_stamp != NULL)
    {
      // complete the fullname of the parameters stamp file
      sprintf(foutname, "%s/%s", output_path, param.parameter_stamp);
    }
    else
    {
      // define the parameters stamp file using stamp default = date-time
      struct tm *ct;
      const time_t t        = time(NULL);
      param.parameter_stamp = (char *) malloc(sizeof(char) * 100);
      ct                    = localtime(&t);
      snprintf(param.parameter_stamp, 100, "d%d%.2d%.2dh%.2d%.2d%.2d", ct->tm_year + 1900, ct->tm_mon, ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);

      sprintf(foutname, "%s/%s", output_path, param.parameter_stamp);
    }
    // check if the stamp already exists
    fout = fopen(foutname, "r");
    // TODO: opendir() should be done first to avoid open a directory!
    if (!fout)
    {
      // save parameters in the stamp file
      printf("\nwriting parameters in %s", foutname);
      fout = fopen(foutname, "w+");
      for (i = 0; i < total_parameters; i++)
      {
        fprintf(fout, "%s ", parameters[i].param_name);
        switch (parameters[i].type)
        {
          case INT:
            fprintf(fout, "%d\n", *((int *) parameters[i].param_var));
            break;
          case DOUBLE:
            fprintf(fout, "%lf\n", *((double *) parameters[i].param_var));
            break;
          case STRING:
            fprintf(fout, "%s\n", *((char **) parameters[i].param_var));
            break;
        }
      }
      fclose(fout);
    }
    else
    {
      // check if the stamp is valid
      char param_name[100];
      char svalue[100];
      while (!feof(fout))
      {
        fscanf(fout, "%s", param_name);
        for (j = 0; j < total_parameters && strcmp(param_name, parameters[j].param_name); j++);
        if (j >= total_parameters)
        {
          printf("\nParameter (%s) invalid or uncompleted.", param_name);
          error = 1;
          break;
        }
        else
        {
          switch (parameters[j].type)
          {
            case INT:
              fscanf(fout, "%d\n", &ivalue);
              if (ivalue < parameters[j].ilb || ivalue > parameters[j].iub)
              {
                printf("\nParameter (%s) value (%d) out of range [%d,%d].", param_name, ivalue, parameters[j].ilb, parameters[j].iub);
                error = 1;
              }
              else if (ivalue != *((int *) (parameters[j].param_var)))
              {
                printf("\nParameter (%s) value (%d) differs to saved value = %d.", param_name, ivalue, *((int *) (parameters[j].param_var)));
                error = 1;
              }
              break;
            case DOUBLE:
              fscanf(fout, "%lf\n", &dvalue);
              if (dvalue < parameters[j].dlb || dvalue > parameters[j].dub)
              {
                printf("\nParameter (%s) value (%lf) out of range [%lf,%lf].", param_name, dvalue, parameters[j].dlb, parameters[j].dub);
                error = 1;
              }
              else if (fabs(dvalue - *((double *) (parameters[j].param_var))) > EPSILON)
              {
                printf("\nParameter (%s) value (%lf) differs to saved value = %lf.", param_name, dvalue, *((double *) (parameters[j].param_var)));
                error = 1;
              }
              break;
            case STRING:
              fscanf(fout, "%s\n", svalue);
              if (strcmp(svalue, *((char **) (parameters[j].param_var))))
              {
                printf("\nParameter (%s) value (%s) differs to saved value = %s.", param_name, svalue, *((char **) (parameters[j].param_var)));
                error = 1;
              }
              break;
          }
        }  // each parameter
      }  // while !feof
      fclose(fout);
    }
  }
  return !error;
}