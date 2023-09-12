#include "listcreat.h"
#include <unistd.h>

//Struct that conatins the args for the thread routine
typedef struct targ targ;
struct targ{
	List* l;
	int id;
	int * contador;
	long long * idents;
};

//Thread routine function
void* routine(void* p){
	
	targ* a = (targ*)p;
	List* lista = a->l;
	int i = a->id;
	long long * idents = a->idents;
	int* contador = a->contador;
	*contador = 0;
	Creat* tmp = newCreat("hormiga", "negro", 2, 10, 0);
	
	//Inserts creatures
	for(int j = 0; j < 100; j++){
	
		long long id = 100*i + j;
		tmp->identificador = id;

		addHead(lista, tmp);
		
	}
	destroyCreat(tmp);
	
	//Extracts creatures
	for(int k = 0; k < 40; k++){
		
		tmp = extractlast(lista);

		if(tmp->identificador < (100*i) || tmp->identificador >= (100*(i+1))){
			idents[*contador] = tmp->identificador;
			*contador = *contador + 1;
		}
		
		destroyCreat(tmp);
	}
	printf("Thread %d, ha extraído %d criaturas que no le corresponden\n\n", i, *contador);
	
	pthread_exit(&i);

}


int main(int argc, char *argv[])
{
	
	List* lista = createList();
	void *status;
	long long total = 0;
	int error = 0;
	targ* targs = malloc(100*sizeof(targ));
	pthread_t* tids = malloc(100*sizeof(pthread_t));
	long long * totalidents = malloc(100*40*sizeof(long long));
	
	
	//Launches threads with their respective arguments
	for(int i = 0; i < 100; i++){
		tids[i] = i;
		targs[i].l = lista;
		targs[i].id = i;
		targs[i].contador = malloc(sizeof(int));
		targs[i].idents = malloc(40*sizeof(long long));
		pthread_create(&tids[i], NULL, routine, &targs[i]);	
		
	}
	
	//Collects threads and puts all the extracted creature identifiers in the same array
	for(int i = 0; i < 100; i++){
		pthread_join(tids[i], &status);
		for(int j = 0; j < *targs[i].contador; j++){
			totalidents[total+j] = targs[i].idents[j];
		}
		total = total+(*targs[i].contador);
		free(targs[i].contador);
		free(targs[i].idents);
	}
	
	
	//Checks the creature identifiers array
	for(int j = 0; j < 100; j++){
	
		int id = 100*j;
		int idmin = 100*j;
		int idmax = 100*(j+1);
		
		for(int i = 0; i < total; i++){
			if(idmin <= totalidents[i] && totalidents[i] < idmax){
				if(totalidents[i] >= id){
					id = totalidents[i];
				}
				else{
					printf("Error de orden, la criatura %d ha sido extraida antes que la criatura %lld\n\n", id, totalidents[i]);
					id = totalidents[i];
					error++;
				}
			}
		}
	}
	
	free(targs);
	free(tids);
	free(totalidents);
	
	printf("Tamaño final de la lista: %d\n\n", lista->size);
	if(lista->size == 60*100){
		printf("Tamaño correcto\n\n");
	}
	printf("Numero de errores de orden: %d\n\n", error);
	
	destroyList(lista);
	
	exit(EXIT_SUCCESS);
}
