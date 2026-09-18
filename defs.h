#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/wait.h> 

#define TITLE 200
#define AUTHOR 200
#define PATH 60
#define KEYWORD 30

#define MAX_CHILDREN 10


typedef struct msg
{
    char type;             // 1
    int pid;               // 4
    char title[TITLE];     // 200
    char author[AUTHOR];   // 200
    char path[PATH];       // 64
    int year;              // 4
    int key;               // 4
    char keyword[KEYWORD]; // 30
    int nr_processes;      // 4
} Msg;

typedef struct res_a
{
    int key; // 4
} Res_a;

typedef struct res_c
{
    char title[TITLE];   // 200
    char author[AUTHOR]; // 200
    char path[PATH];     // 64
    int year;            // 4
} Res_c;

typedef struct res_d
{
    int key;     // 4
    int boolean; // 4
} Res_d;

typedef struct res_l
{
    int count; // 4
} Res_l;

// Primeiro envia a quantidade de números da lista
// depois envia todos os números, 1 de cada vez
typedef struct res_s
{
    int total; // 4
    int* keys;
} Res_s;

typedef struct doc
{
    int valid;           // 4
    char title[TITLE];   // 200
    char author[AUTHOR]; // 200
    char path[PATH];     // 64
    int year;            // 4
} Doc;
