#ifndef ARRARGC_H
#define ARRARGC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

// for timespec support
#include <stdint.h>
#include <time.h>

#define NSEC_PER_SEC ((int64_t)1e9)
static inline void convertNsToTimespec(int64_t* t, struct timespec* out) {
  out->tv_sec = (*t) / NSEC_PER_SEC;
  out->tv_nsec = (*t) % NSEC_PER_SEC;
}

#define ARRAYDELIMITER ","

#define SimpleArray(giventype, givenname) \
typedef struct givenname##_s {            \
  int size;                               \
  giventype *elements;                    \
} givenname

// TODO kinda ugly to predefine these generic structure names
SimpleArray(void*, DummyArray);
SimpleArray(char*, StringArray);
SimpleArray(int, IntArray);
SimpleArray(long, LongArray);
SimpleArray(float, FloatArray);
SimpleArray(double, DoubleArray);

typedef struct argument_s {
  char *longname;
  char *shortname;
  long  type;
  char *description;
  void *parameter;
} Argument;

typedef enum argumenttype_s {
  PARAM_BOOL,
  PARAM_STRING,
  PARAM_STRING_ARRAY,
  PARAM_INT,
  PARAM_INT_ARRAY,
  PARAM_LONG,
  PARAM_LONG_ARRAY,
  PARAM_FLOAT,
  PARAM_FLOAT_ARRAY,
  PARAM_DOUBLE,
  PARAM_DOUBLE_ARRAY,
  PARAM_TIMESPEC
} ArgumentType;

static inline int pcinternal_getStringArrayLength(char* stringifiedarray) {
  int length = strlen(stringifiedarray)+1;
  if (length > PATH_MAX) {
    fprintf(stderr, "array \"%s\" is too long\n", stringifiedarray);
    exit(EINVAL);
  }
  int arraylength = 1;
  char* head = stringifiedarray;
  while(*head != '\0') { // will count a last comma without following character as another element
    if (ARRAYDELIMITER[0] == head[0]) {
      arraylength++;
    }
    head++;
  }
  return arraylength;
}

static inline size_t pcinternal_getSizeOf(int type) {
  switch (type) {
  case PARAM_BOOL:
    return sizeof(bool);
  case PARAM_INT:
    return sizeof(int);
  case PARAM_LONG:
    return sizeof(long);
  case PARAM_FLOAT:
    return sizeof(float);
  case PARAM_DOUBLE:
    return sizeof(double);
  case PARAM_INT_ARRAY:
  case PARAM_LONG_ARRAY:
  case PARAM_FLOAT_ARRAY:
  case PARAM_DOUBLE_ARRAY:
  case PARAM_STRING_ARRAY:
  case PARAM_STRING:
    return sizeof(DummyArray);
  default:
    printf("using unsupported type %d\n", type);
    exit(EINVAL);
  }
}

static inline void pcinternal_getNextArgumentIndex(int argc, char* argv[], int* i) {
  (*i)++;
  if (((*i) >= argc) || (argv[*i][0] == '-')) {
    printf("missing parameter for argument %s\n", argv[(*i) - 1]);
    exit(EINVAL);
  }
}

// unused attribute to avoid warning
static __attribute__((unused)) void printArgumentHelp(Argument* arguments) {
  printf("Available Options:\n");
  Argument *argument = arguments;
  int maxlength = 0;
  while (argument->longname != NULL) {
    maxlength = strlen(argument->longname) > maxlength ? strlen(argument->longname) : maxlength;
    argument++;
  }
  argument = arguments;
  char format[PATH_MAX];
  while (argument->longname != NULL) {
    if ((argument->shortname == NULL) || (strlen(argument->shortname) == 0)) {
      snprintf(format, PATH_MAX, "    --%%-%ds  %%s\n", maxlength);
      printf(format, argument->longname, argument->description);
    } else {
      snprintf(format, PATH_MAX, " -%%s --%%-%ds  %%s\n", maxlength);
      printf(format, argument->shortname, argument->longname, argument->description);
    }
    argument++;
  }
  snprintf(format, PATH_MAX, " -h --help%%%ds  will show this help\n", maxlength-4);
  printf(format, "");
}

static inline void processArguments(Argument *arguments, int argc, char *argv[]) {
  bool showhelp = false;
  Argument *argument = arguments;
  while (argument->longname != NULL) {
    memset(argument->parameter, 0, pcinternal_getSizeOf(argument->type));
    argument++;
  }
  for (int i = 1; i < argc; i++) {
    bool recognizedarg = false;
    if (strcmp(argv[i], "--") == 0) {
      break; // stop argument parsing
    }
    if ((strncmp(argv[i], "-h", 2)==0) || (strncmp(argv[i], "--help", 6)==0)) {
      showhelp = true;
      recognizedarg = true;
      continue;
    }
    argument = arguments;
    while (argument->longname != NULL) {
      bool matchshort =
          ((argument->shortname != NULL && strlen(argument->shortname) != 0) &&
           (strncmp(argument->shortname, argv[i] + 1,
                    strlen(argument->shortname)) == 0));
      int argoffset;
      if (matchshort ||
          (strcmp(argument->longname, argv[i]+2) == 0) ) {
        recognizedarg = true;
        if (matchshort && strlen(argv[i]) != 2) {
          argoffset = 2;
        } else {
          argoffset = 0;
          if (argument->type != PARAM_BOOL) {
            pcinternal_getNextArgumentIndex(argc, argv, &i);
          }
        }
        switch (argument->type) {
        case PARAM_BOOL:
          *((bool *)argument->parameter) = true;
          break;
        case PARAM_INT:
          *((int *)argument->parameter) = atoi(argv[i] + argoffset);
          break;
        case PARAM_INT_ARRAY:
        {
          IntArray* array = (IntArray *)argument->parameter;
          char *strcopy = strdup(argv[i] + argoffset);
          char* strhead = strcopy;
          array->size = pcinternal_getStringArrayLength(strhead);
          array->elements = (int*)malloc(sizeof(int) * array->size);
          strtok(strhead, ARRAYDELIMITER);
          for (int i = 0; strhead; i++) {
            array->elements[i] = atoi(strhead);
            strhead = strtok(NULL, ",");
          }
          free(strcopy);
          break;
        }
        case PARAM_LONG:
          *((int *)argument->parameter) = atol(argv[i] + argoffset);
          break;
        case PARAM_LONG_ARRAY:
        {
          LongArray* array = (LongArray *)argument->parameter;
          char* strcopy = strdup(argv[i] + argoffset);
          array->size = pcinternal_getStringArrayLength(strcopy);
          array->elements = (long*)malloc(sizeof(long) * array->size);
          strtok(strcopy, ARRAYDELIMITER);
          for (int i = 0; strcopy; i++) {
            array->elements[i] = atol(strcopy);
            strcopy = strtok(NULL, ",");
          }
          free(strcopy);
          break;
        }
        case PARAM_FLOAT:
          *((float*)argument->parameter) = atof(argv[i] + argoffset);
          break;
        case PARAM_FLOAT_ARRAY:
        {
          FloatArray* array = (FloatArray*)argument->parameter;
          char* strcopy = strdup(argv[i] + argoffset);
          array->size = pcinternal_getStringArrayLength(strcopy);
          array->elements = (float*)malloc(sizeof(float) * array->size);
          strtok(strcopy, ARRAYDELIMITER);
          for (int i = 0; strcopy; i++) {
            array->elements[i] = atof(strcopy);
            strcopy = strtok(NULL, ",");
          }
          free(strcopy);
          break;
        }
        case PARAM_DOUBLE:
          *((double*)argument->parameter) = atof(argv[i] + argoffset);
          break;
        case PARAM_DOUBLE_ARRAY:
        {
          DoubleArray* array = (DoubleArray*)argument->parameter;
          char* strcopy = strdup(argv[i] + argoffset);
          array->size = pcinternal_getStringArrayLength(strcopy);
          array->elements = (double*)malloc(sizeof(double) * array->size);
          strtok(strcopy, ARRAYDELIMITER);
          for (int i = 0; strcopy; i++) {
            array->elements[i] = atof(strcopy);
            strcopy = strtok(NULL, ",");
          }
          free(strcopy);
          break;
        }
        case PARAM_STRING:
          *((char**)argument->parameter) = argv[i] + argoffset;
          break;
        case PARAM_STRING_ARRAY:
        {
          StringArray* array = (StringArray*)argument->parameter;
          char* strcopy = strdup(argv[i] + argoffset);
          array->size = pcinternal_getStringArrayLength(strcopy);
          array->elements = (char**)malloc(sizeof(char*) * array->size);
          strtok(strcopy, ARRAYDELIMITER);
          for (int i = 0; strcopy; i++) {
            array->elements[i] = strdup(strcopy);
            strcopy = strtok(NULL, ",");
          }
          free(strcopy);
          break;
        }
        case PARAM_TIMESPEC:
        {
          pcinternal_getNextArgumentIndex(argc, argv, &i);
          long val = atol(argv[i] + argoffset); // parse long int and convert to timespec
          if (val > 0) {
            convertNsToTimespec(&val, ((struct timespec*)argument->parameter));
          }
          break;
        }
        default:
          printArgumentHelp(arguments);
          printf("using unsupported type for argument %s\n", argument->longname);
          exit(EINVAL);
        }
      }
      if (recognizedarg) {
        break;
      }
      argument++;
    }
    if (!recognizedarg) {
      printArgumentHelp(arguments);
      printf("unregconized argument[%d]: %s\n", i, argv[i]);
      exit(EINVAL);
    }
  }
  if (showhelp) {
    printArgumentHelp(arguments);
    exit(EINVAL);
  }
}

static inline void freeArguments(Argument *arguments) {
  Argument *argument = arguments;
  while (argument->longname != NULL) {
    if (argument->parameter != NULL) {
      DummyArray* array;
      switch (argument->type) {
      case PARAM_STRING_ARRAY:
      {
        StringArray *strings = (StringArray *)argument->parameter;
        if (strings != NULL) {
          for (int i = 0; i < strings->size; i++) {
            free(strings->elements[i]);
          }
          free(strings->elements);
        }
        break;
      }
      case PARAM_INT_ARRAY:
      case PARAM_LONG_ARRAY:
      case PARAM_FLOAT_ARRAY:
      case PARAM_DOUBLE_ARRAY:
        array = (DummyArray*)argument->parameter;
        if (array != NULL) {
          free(array->elements);
        }
      }
    }
    argument++;
  }
}

#ifdef __cplusplus
}
#endif

#endif
