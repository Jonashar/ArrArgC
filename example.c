#include "arrargc.h"

static struct config_s {
  int optiona;
  IntArray intarray;
  int optionb;
  int optionc;
  float optiond;
  double optione;
  StringArray strings;
  char* name;
} configuration;

static Argument arguments[] = {
  {"optiona", "a", PARAM_INT, "this is option a", &configuration.optiona},
  {"array", "", PARAM_INT_ARRAY, "this is an option array", &configuration.intarray},
  {"optionbhasaverylongname", "b", PARAM_INT, "this is option b", &configuration.optionb},
  {"optionc", NULL, PARAM_INT, "this is option c", &configuration.optionc},
  {"localvar", NULL, PARAM_INT, "this is a local variable", NULL}, // do not set parameter to NULL
  {"optionc", NULL, PARAM_INT, "this is option c", &configuration.optionc},
  {"optiond", NULL, PARAM_FLOAT, "this is option d", &configuration.optiond},
  {"optione", NULL, PARAM_DOUBLE, "this is option e", &configuration.optione},
  {"strings", "s", PARAM_STRING_ARRAY, "this is a string array", &configuration.strings},
  {"name", "n", PARAM_STRING, "this is the name", &configuration.name},
  {NULL}
};

int main(int argc, char *argv[]) {
  int localvar;
  arguments[4].parameter = &localvar; // little hack to test an uninitialized stack variable (since our other static options are initialized with 0)

  processArguments(arguments, argc, argv);

  printf("option a is %d\n", configuration.optiona);
  printf("int array of size %d:\n     ", configuration.intarray.size);
  for (int i = 0; i < configuration.intarray.size; i++) {
    printf("%d ", configuration.intarray.elements[i]);
  }
  printf("\n");
  printf("option b is %d\n", configuration.optionb);
  printf("option c is %d\n", configuration.optionc);
  printf("localvar is %d\n", localvar);

  printf("option d is %f\n", configuration.optiond);
  printf("option e is %lf\n", configuration.optione);

  printf("some strings given:\n    ");
  for (int i = 0; i < configuration.strings.size; i++) {
    printf("%s ", configuration.strings.elements[i]);
  }
  printf("\n");

  freeArguments(arguments);
  return 0;
}
