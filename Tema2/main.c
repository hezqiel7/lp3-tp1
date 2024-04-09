#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void enviarSignals(char *archivo) {

    pid_t processId;
    int signalNumber;
    int delaySeconds;
  
    FILE *f = fopen(archivo, "r");
    if (f == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
  
    while (fscanf(f, "%d %d %d", &processId, &signalNumber, &delaySeconds) == 3) {
        printf("Enviando señal %d a el proceso %d en %d segundos.\n", signalNumber, processId, delaySeconds);
        sleep(delaySeconds);
        kill(processId, signalNumber);
    }

    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s archivo_de_señales\n", argv[0]);
        return EXIT_FAILURE;
    }

    enviarSignals(argv[1]);

    printf("Todas las señales se enviaron.\n");

    return EXIT_SUCCESS;
}
