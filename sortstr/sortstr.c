#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

void
PRINT_STR_ARRAY(int size, char *str[])
{
	for (int i = 0; i < size; i++) /*itera sobre los argumentos y los imprime en pantalla*/
		{
			printf("%s",str[i]);
			printf("\n");
		}
}/*PRINT_STR_ARRAY*/

int
CHECK_VOCALES(char *palabra)/*Comprueba si hay vocales. TRUE si hay, FALSE si no hay*/
{
	int hay_vocal = 0;
	for (int i = 0; i < strlen(palabra); i++)
	{
		if (palabra[i] == 'a' || palabra[i] == 'e' || palabra[i] == 'i' || palabra[i] == 'o' || palabra[i] == 'u')
		{
			hay_vocal = 1;
			return hay_vocal;
		}
	}
	return hay_vocal;
}/*CHECK_VOCALES*/

void
FILL_ARRAY(int max_len, int size, char *src[], char *arr[])
{
	
	int j = 0;
	
	for (int i = 1; i < size; i++)/*No cuenta argv[0]*/
	{
		if (!CHECK_VOCALES(src[i]))
		{
			i++; /*si no hay vocales no lo considera un argumento valido*/
		}
		else
		{
			printf("%s","copiando: ");
			printf("%s",src[i]);
			printf("\n");
			strncpy(arr[j], src[i], max_len);
			j++;
		}	
	}
}/*FILL_ARRAY*/

int
GET_MAX_LEN(int size, char *str[])
{
	int max_len = 0;
	for (int i = 1; i < size; i++)/*No cuenta argv[0]*/
	{
		if (!CHECK_VOCALES(str[i]))
		{
			i++; /*si no hay vocales no lo considera un argumento valido*/
		}
		else
		{
			if (strlen(str[i]) > max_len)
			{
				max_len = strlen(str[i]);
			}
		}
	}

	return max_len;
	
}/*GET_MAX_LEN*/

int
GET_VALID_ARGS(int size, char *str[])
{
	int valid_args = 0;
	for (int i = 1; i < size; i++)/*No cuenta argv[0]*/
	{
		if (!CHECK_VOCALES(str[i]))
		{
			i++; /*si no hay vocales no lo considera un argumento valido*/
		}
		else
		{
			valid_args++;
		}
	}

	return valid_args;
	
}/*GET_VALID_ARGS*/



int
main(int argc, char *argv[])
{

	//PRINT_STR_ARRAY(argc, argv);
	int max_len = GET_MAX_LEN(argc, argv);
	int valid_args = GET_VALID_ARGS(argc, argv);
	char *arg_depurado[valid_args * max_len];
	printf("%d", max_len);
	printf("\n");
	printf("%d", valid_args);
	printf("\n");
	FILL_ARRAY(max_len, argc, argv, arg_depurado);
	PRINT_STR_ARRAY((valid_args * max_len), arg_depurado);

	exit(EXIT_SUCCESS);
}
