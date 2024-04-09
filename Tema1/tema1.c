// COMPILAR: gcc main.c -o main -lrt -lm
// EJECTUAR: ./main 7 5,4,8,9,3,1,4,7,8,9,5,4,8,7,9,6

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

#define MAX_ITEMS 20 // Maxima cantidad de items en el arreglo

// Estructura de datos de la memoria compartida
struct datos {
  int arr[MAX_ITEMS];
  char ordenados[2000]; // String para ir guardando los arrays ordenados
  char arbol[MAX_ITEMS][100];
};

// Función merge normal de mergesort
void merge(int arr[], int l, int m, int r) {
  int i, j, k;
  int n1 = m - l + 1;
  int n2 = r - m;

  int L[n1], R[n2];

  for (i = 0; i < n1; i++)
    L[i] = arr[l + i];
  for (j = 0; j < n2; j++)
    R[j] = arr[m + 1 + j];

  i = 0;
  j = 0;
  k = l;
  while (i < n1 && j < n2) {
    if (L[i] <= R[j]) {
      arr[k] = L[i];
      i++;
    } else {
      arr[k] = R[j];
      j++;
    }
    k++;
  }

  while (i < n1) {
    arr[k] = L[i];
    i++;
    k++;
  }

  while (j < n2) {
    arr[k] = R[j];
    j++;
    k++;
  }
}

// Función para retornar string de un array
char *array_to_string(int arr[], int l, int r) {
  char *result = (char *)malloc((r - l) * 2);

  int pos = 0;
  for (int i = l; i < r; i++) {
    pos += sprintf(result + pos, "%d,", arr[i]);
  }

  sprintf(result + pos, "%d", arr[r]);
  return result;
}

// Función para imprimir un array
void imprimir(int arr[], int l, int r) {
  for (int i = l; i < r; i++) {
    printf("%d,", arr[i]);
  }
  printf("%d\n", arr[r]);
}

// Función para imprimir visualmente un array de strings en forma de arbol
// binario
void imprimirArbol(char arr[][100], int n, int i, int depth) {
  if (i >= n)
    return;

  depth++; // Contar profundidad para imprimir espacios

  // En un array que representa un arbol binario los hijos del nodo i se
  // encuentran mediante las sgtes formulas
  int hijoDer = (2 * i) + 2;
  int hijoIzq = (2 * i) + 1;

  // Imprimir hijo derecho
  imprimirArbol(arr, n, hijoDer, depth);
  printf("\n");

  // Imprimir nodo actual
  for (int j = 1; j < depth; j++)
    printf("%*s", (int)strlen(arr[0]) / j, "");
  printf("%s\n", arr[i]);

  // Imprimir hijo izquierdo
  imprimirArbol(arr, n, hijoIzq, depth);
}

// Función para contar los strings que no estan vacios de un array de strings
int contar_no_vacios(char arr[][100]) {
  int contador = 0;
  int i;

  // Recorremos el array de strings
  for (i = 0; i < MAX_ITEMS; i++) {
    if (arr[i][0] != '\0') {
      contador++;
    }
  }

  return contador;
}

// Mergesort con procesos hasta cierta profundidad
void mergeSort(struct datos *shmptr, int l, int r, int depth, int x) {
  if (l < r) {
    if (depth > 0) { // Verificar profundidad para utliizar procesos
      int m = l + (r - l) / 2;
      pid_t left_child_pid, right_child_pid;

      left_child_pid = fork();
      if (left_child_pid < 0) {
        perror("Error en fork");
        exit(EXIT_FAILURE);

      } else if (left_child_pid == 0) { // Proceso hijo izquierdo
        // Imprimir mapeo
        printf("Proceso #%d ", getpid());
        printf("%s\n", array_to_string(shmptr->arr, l, m));

        char tmp[100];

        // Ir guardando en el arbol los arrays de cada proceso
        sprintf(tmp, "#%d: %s\n", getpid(), array_to_string(shmptr->arr, l, m));
        strcpy(shmptr->arbol[2 * x + 1], tmp);

        mergeSort(shmptr, l, m, depth - 1, 2 * x + 1);

        // Ir guardando en el arbol los arrays ordenados de cada proceso

        sprintf(tmp, "Proceso #%d lista ordenada: %s\n", getpid(),
                array_to_string(shmptr->arr, l, m));
        strcat(shmptr->ordenados, tmp);
        exit(EXIT_SUCCESS);

      } else {
        right_child_pid = fork();

        if (right_child_pid < 0) {
          perror("Error en fork");
          exit(EXIT_FAILURE);

        } else if (right_child_pid == 0) { // Proceso hijo derecho
          // Imprimir mapeo
          printf("Proceso #%d ", getpid());
          printf("%s\n", array_to_string(shmptr->arr, m + 1, r));

          char tmp[100];

          // Ir guardando en el arbol los arrays de cada proceso
          sprintf(tmp, "#%d: %s\n", getpid(),
                  array_to_string(shmptr->arr, m + 1, r));
          strcpy(shmptr->arbol[2 * x + 2], tmp);

          mergeSort(shmptr, m + 1, r, depth - 1, 2 * x + 2);

          // Ir guardando en el arbol los arrays ordenados de cada proceso
          sprintf(tmp, "Proceso #%d lista ordenada: %s\n", getpid(),
                  array_to_string(shmptr->arr, m + 1, r));
          strcat(shmptr->ordenados, tmp);
          exit(EXIT_SUCCESS);
        }
      }

      // Esperar que terminen los hijos
      waitpid(left_child_pid, NULL, 0);
      waitpid(right_child_pid, NULL, 0);

      merge(shmptr->arr, l, m, r);
    } else { // Mergesort sin procesos
      int m = l + (r - l) / 2;
      mergeSort(shmptr, l, m, 0, x);
      mergeSort(shmptr, m + 1, r, 0, x);
      merge(shmptr->arr, l, m, r);
    }
  }
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
    printf("Uso: %s <procesos> <array>\n", argv[0]);
    exit(1);
  }

  int cprocesos = atoi(argv[1]); // Cantidad de procesos

  float x = log(cprocesos+1) / log(2);
  if(x != (int)x) {
    printf("La cantidad de procesos debe representar un arbol binario completo (1, 3, 5, 7, 15, ...)\n");
    exit(1);
  }
  
  // Obtener la profundidad hasta la cual hay que usar procesos
  int max_depth = log(cprocesos + 1) / log(2) - 1;

  char *arrstr = argv[2]; // String del array
  int *arr = NULL;
  int c = 0; // Cantidad de elementos

  // Obtener los elementos del string
  char *token = strtok(arrstr, ",");
  while (token != NULL) {
    if (c >= MAX_ITEMS) {
      printf("Se ha superado el límite máximo de elementos (%d)\n", MAX_ITEMS);
      free(arr);
      exit(1);
    }
    arr = realloc(arr, (c + 1) * sizeof(int));
    arr[c++] = atoi(token);
    token = strtok(NULL, ",");
  }

  // Configurar la memoria compartida
  int shmid;               // Id de la memoria compartida
  struct datos *shmptr;    // Puntero de la memoria compartida
  key_t key = IPC_PRIVATE; // Key unica del proceso
  size_t shm_size = sizeof(struct datos);
  int i;

  // Crear la memoria compartida
  if ((shmid = shmget(key, shm_size, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    exit(1);
  }

  // Asignar la memoria compartida al puntero
  if ((shmptr = shmat(shmid, NULL, 0)) == (struct datos *)-1) {
    perror("shmat");
    exit(1);
  }

  // Copiar los elementos del array la memoria compartida
  for (i = 0; i < c; i++) {
    shmptr->arr[i] = arr[i];
  }

  free(arr); // Liberar array temporal

  // Inicializar el arbol a strings vacios
  for (i = 0; i < MAX_ITEMS; i++) {
    strcpy(shmptr->arbol[i], "");
  }

  printf("===Mapeos===\n");
  printf("Proceso #%d ", getpid()); // Imprimir el proceso padre
  printf("%s\n", array_to_string(shmptr->arr, 0, c - 1));

  // Guardar en el arbol el array inicial
  char tmp[100];
  sprintf(tmp, "#%d: %s\n", getpid(), array_to_string(shmptr->arr, 0, c - 1));
  strcpy(shmptr->arbol[0], tmp);
  // strcpy(shmptr->arbol[0], array_to_string(shmptr->arr, 0, c - 1));

  // mergesort especificando hasta que profundidad usar procesos
  // mergesort(shm, inicio, fin, profundidad, indice para agregar al arbol))
  mergeSort(shmptr, 0, c - 1, max_depth, 0);

  printf("\n===Procesamiento===\n");
  printf("%s", shmptr->ordenados);
  printf("Proceso #%d lista ordenada: %s\n", getpid(),
         array_to_string(shmptr->arr, 0, c - 1));

  printf("\n===Arbol===\n");
  imprimirArbol(shmptr->arbol, contar_no_vacios(shmptr->arbol), 0, 0);

  // Desasignar la memoria compartida
  if (shmdt(shmptr) == -1) {
    perror("shmdt");
    exit(1);
  }

  // Eliminar la memoria compartida
  if (shmctl(shmid, IPC_RMID, NULL) == -1) {
    perror("shmctl");
    exit(1);
  }

  return 0;
}
