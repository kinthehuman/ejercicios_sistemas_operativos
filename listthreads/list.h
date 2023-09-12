#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

typedef struct Creat Creat;
struct Creat {
	char *especie;
	char *color;
	int altura;
	int peso;
	long long identificador;
	Creat *next;
	Creat *prev;
};

/*Prints Creat struct data using the following format:

Especie: criatura->especie
Color: criatura->color
Altura: criatura->altura mm
Peso: criatura->peso g
Identificador: criatura->identificador

*/
void printCreat(Creat *criatura);

//Creates a Creat struct allocating memory for it.
Creat *newCreat(char *e, char *c, int a, int p, long long i);

//Frees the memory of the Creat struct if it was created with newCreat().
void destroyCreat(Creat *criatura);

typedef struct List List;
struct List {
	Creat *Head;
	Creat *Tail;
	int size;
	pthread_mutex_t lock;
};

//Creates an empty list.
List *createList();

//Boolean function that tell you if the list is empty.
int isEmpty(List *lista);

//Prints iterating through it and calling printCreat().
void printList(List *lista);

//Adds element at the beggining of the list, calls newCreat().
void addHead(List *lista, Creat *c);

//Adds element at the end of the list, calls newCreat().
void addTail(List *lista, Creat *c);

//Atomic removal of first element.
Creat* extractfirst(List* lista);

//Atomic removal of last element.
Creat* extractlast(List* lista);

/*Returns Creat struct with same ident.
Will fail and return NULL if the creature doesn't exist or there are duplicates.
*/
Creat *find(List *lista, long long ident);

//Removes element with matching identifier if it's found in the list.
void removeCreat(List *lista, Creat *criatura);

//Returns number of elements in the list
int getSize(List *lista);

//Destroys and frees memory from all elements in the list and then the list itself.
void destroyList(List *lista);





