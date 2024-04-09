#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void signalHandler(int signum) {
  switch (signum) {
  case SIGINT:
    printf("Señal SIGINT recibida.\n");
    break;
  case SIGALRM:
    printf("Señal SIGALRM recibida.\n");
    break;
  case SIGUSR1:
    printf("Señal SIGUSR1 recibida.\n");
    break;
  case SIGUSR2:
    printf("Señal SIGUSR2 recibida.\n");
    break;
  default:
    printf("Señal %d recibida.\n", signum);
  }
}

int main() {
  // Manejador de señales
  struct sigaction sa;
  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  // Manejando las señales
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGALRM, &sa, NULL);
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGUSR2, &sa, NULL);

  printf("Esperando señales en %d...\n", getpid());

  while (1) {
    sleep(1);
  }

  return EXIT_SUCCESS;
}
