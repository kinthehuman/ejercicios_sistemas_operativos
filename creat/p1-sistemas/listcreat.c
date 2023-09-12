#include "listcreat.h"

Creat *
newCreat(char *e, char *c, int a, int p, long long i)
{
	Creat *new = malloc(sizeof(Creat));

	new->especie = malloc(64 * sizeof(char));
	strcpy(new->especie, e);

	new->color = malloc(64 * sizeof(char));
	strcpy(new->color, c);

	new->altura = a;
	new->peso = p;
	new->identificador = i;
	new->next = NULL;
	new->prev = NULL;

	return new;
}

void
destroyCreat(Creat * del)
{
	free(del->especie);
	free(del->color);
	free(del);
}

void
printCreat(Creat * criatura)
{
	printf("Especie: %s\n", criatura->especie);
	printf("Color: %s\n", criatura->color);
	printf("Altura: %dmm\n", criatura->altura);
	printf("Peso: %dg\n", criatura->peso);
	printf("Identificador: %lld\n\n", criatura->identificador);
}

List *
createList()
{
	List *LISTA = malloc(sizeof(List));

	LISTA->Head = NULL;
	LISTA->Tail = NULL;
	LISTA->size = 0;
	pthread_mutex_init(&(LISTA->lock), NULL);
	return LISTA;
}

int
isEmpty(List * lista)
{
	return (lista->Head == NULL);
}

void
printList(List * lista)
{
	for (Creat * it = lista->Head; it != NULL; it = it->next) {
		printCreat(it);
	}
}

void
addHead(List * lista, Creat * c)
{
	Creat *criatura = newCreat(c->especie, c->color, c->altura, c->peso,
				   c->identificador);
				   
	pthread_mutex_lock(&(lista->lock));
	if (lista->Head == NULL) {
		lista->Head = criatura;
		lista->Tail = criatura;
	} else {
		lista->Head->prev = criatura;
		criatura->next = lista->Head;
		lista->Head = criatura;
	}
	lista->size++;
	
	pthread_mutex_unlock(&(lista->lock));
}

void
addTail(List * lista, Creat * c)
{
	Creat *criatura = newCreat(c->especie, c->color, c->altura, c->peso,
				   c->identificador);
				   
	pthread_mutex_lock(&(lista->lock));
	if (lista->Head == NULL) {
		lista->Head = criatura;
		lista->Tail = criatura;
	} else {
		lista->Tail->next = criatura;
		criatura->prev = lista->Tail;
		lista->Tail = criatura;
	}
	lista->size++;
	
	pthread_mutex_unlock(&(lista->lock));
}

Creat* 
extractfirst(List* lista){

	pthread_mutex_lock(&(lista->lock));
	
	Creat* result = newCreat(lista->Head->especie, lista->Head->color, lista->Head->altura, lista->Head->peso, lista->Head->identificador);
	removeCreat(lista, lista->Head);
	
	pthread_mutex_unlock(&(lista->lock));
	
	return result;
	
}

Creat* 
extractlast(List* lista){

	pthread_mutex_lock(&(lista->lock));
	
	Creat* result = newCreat(lista->Tail->especie, lista->Tail->color, lista->Tail->altura, lista->Tail->peso, lista->Tail->identificador);
	removeCreat(lista, lista->Tail);
	
	pthread_mutex_unlock(&(lista->lock));
	
	return result;
	
}

Creat *
find(List * lista, long long ident)
{
	Creat *result = malloc(sizeof(Creat));

	result->identificador = -1;
	for (Creat * it = lista->Head; it != NULL; it = it->next) {
		if (it->identificador == ident) {
			if (result->identificador == -1) {
				free(result);
				result = it;
			} else {
				fprintf(stderr,
					"ERROR: Se encontraron dos criaturas con el identificador %lld: %s y %s\n",
					ident, result->especie, it->especie);
				return result;
			}
		}
	}
	if (result->identificador == -1) {
		free(result);
		return NULL;
	}
	return result;
}

void
removeCreat(List * lista, Creat * criatura)
{

	Creat *del = find(lista, criatura->identificador);

	if (del != NULL) {
		if (del->prev != NULL) {
			del->prev->next = del->next;
		} else {
			lista->Head = del->next;
		}

		if (del->next != NULL) {
			del->next->prev = del->prev;
		} else {
			lista->Tail = del->prev;
		}

		lista->size--;
		destroyCreat(del);
	} else {
		fprintf(stderr,
			"ERROR: %s, con identificador %lld no se ha podido encontrar en la lista\n",
			criatura->especie, criatura->identificador);
	}
}

int
getSize(List * lista)
{
	return lista->size;
}

void
destroyList(List * lista)
{
	Creat *it = lista->Head;

	while (!isEmpty(lista)) {
		it = lista->Head;
		removeCreat(lista, it);
	}
	free(lista);
}













