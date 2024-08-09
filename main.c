#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CADEIRAS 5  // Número de cadeiras de espera
#define NUM_CLIENTES 10 // Número total de clientes a serem criados

// Semáforos e mutex
sem_t customers;  // Semáforo para contar clientes esperando atendimento
sem_t barbers;    // Semáforo para indicar que o barbeiro está disponível
pthread_mutex_t mutex;  // Mutex para proteção da seção crítica
pthread_mutex_t id_mutex; // Mutex para proteção do client_id

int waiting = 0;  // Número de clientes esperando
int client_id = 0; // Identificador global para clientes
int haircut_id = 0; // Identificador global para cortes de cabelo
int done = 0; // Número de clientes atendidos
int total_clients = NUM_CLIENTES; // Total de clientes a serem atendidos

// Função do barbeiro
void* barber(void* arg) {
    while (1) {
        sem_wait(&customers);  // Barbeiro espera se não houver clientes
        
        pthread_mutex_lock(&mutex);
        
        // Um cliente está sendo atendido
        waiting--;
        
        // Libera o semáforo para indicar que o barbeiro está pronto para atender um cliente
        sem_post(&barbers);

        // Gera um novo número de corte de cabelo
        int current_haircut_id;
        pthread_mutex_lock(&id_mutex);
        current_haircut_id = haircut_id++;
        pthread_mutex_unlock(&id_mutex);

        // Seção crítica - cortando cabelo
        pthread_mutex_unlock(&mutex);
        
        printf("Corte de cabelo #%d está sendo realizado.\n", current_haircut_id);
        sleep(2); // Simula o tempo de corte de cabelo

        printf("Corte de cabelo #%d foi finalizado.\n", current_haircut_id);

        // Incrementa o número de clientes atendidos
        pthread_mutex_lock(&id_mutex);
        done++;
        pthread_mutex_unlock(&id_mutex);
        
        // Se todos os clientes foram atendidos, o barbeiro pode parar
        if (done >= total_clients) {
            break;
        }
    }
    return NULL;
}

// Função do cliente
void* customer(void* arg) {
    int my_id;
    
    pthread_mutex_lock(&id_mutex);
    my_id = client_id++;
    pthread_mutex_unlock(&id_mutex);
    
    pthread_mutex_lock(&mutex);
    
    // Verifica se há uma cadeira de espera disponível
    if (waiting < NUM_CADEIRAS) {
        waiting++;
        printf("Cliente %d está esperando.\n", my_id);
        
        // Acorda o barbeiro se ele estiver dormindo
        sem_post(&customers);

        // Espera o barbeiro ficar disponível
        pthread_mutex_unlock(&mutex);
        sem_wait(&barbers);

        // Cliente sendo atendido
        printf("Cliente %d está sendo atendido.\n", my_id);
    } else {
        // Não há cadeiras disponíveis
        printf("Não há cadeiras disponíveis. Cliente %d está saindo.\n", my_id);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t barber_thread;
    pthread_t customer_threads[NUM_CLIENTES];

    // Inicializa os semáforos e mutex
    sem_init(&customers, 0, 0);
    sem_init(&barbers, 0, 1);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&id_mutex, NULL);

    // Cria o thread do barbeiro
    pthread_create(&barber_thread, NULL, barber, NULL);

    // Cria os threads dos clientes
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_create(&customer_threads[i], NULL, customer, NULL);
        sleep(1);  // Simula a chegada de clientes em momentos diferentes
    }

    // Espera os threads dos clientes terminarem
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Espera o barbeiro terminar
    pthread_join(barber_thread, NULL);

    // Destroi os semáforos e mutex
    sem_destroy(&customers);
    sem_destroy(&barbers);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&id_mutex);

    return 0;
}
