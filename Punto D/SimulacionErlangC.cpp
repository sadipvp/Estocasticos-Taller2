#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcgrand.cpp" // Generador pseudoaleatorio

#define OCUPADO 1
#define LIBRE 0

// Parámetros de entrada
int num_servidores;         // Número de servidores (c)
int num_clientes;           // Número de clientes a simular
float media_entre_llegadas; // Media de tiempo entre llegadas
float media_atencion;       // Media de tiempo de atención

// Estado del sistema
int servidores_ocupados = 0;
int clientes_en_cola = 0;
int num_clientes_atendidos = 0;
int num_clientes_que_esperaron = 0;
int sig_tipo_evento = 0;

float tiempo_ultimo_evento = 0.0;
float tiempo_simulacion = 0.0;
float area_estado_servidor = 0.0;
float area_num_en_cola = 0.0;
float total_espera_en_cola = 0.0;

int num_eventos = 2;
float tiempo_sig_evento[3]; // [0] no se usa

FILE *parametros, *resultados;

// Prototipos
void inicializar(void);
void control_tiempo(void);
void llegada(void);
void salida(void);
void actualizar_prom_tiempo(void);
void reportes(void);
float expon(float media);

int main(void)
{
    parametros = fopen("params.txt", "r");
    resultados = fopen("results.txt", "w");

    fscanf(parametros, "%f %f %d %d", &media_entre_llegadas, &media_atencion, &num_clientes, &num_servidores);

    fprintf(resultados, "Sistema de Colas M/M/%d\n", num_servidores);
    fprintf(resultados, "---------------------------------------------\n");
    fprintf(resultados, "Tiempo promedio de llegada:        %.3f minutos\n", media_entre_llegadas);
    fprintf(resultados, "Tiempo promedio de atencion:       %.3f minutos\n", media_atencion);
    fprintf(resultados, "Numero de clientes:                %d\n\n", num_clientes);

    inicializar();

    while (num_clientes_atendidos < num_clientes)
    {
        control_tiempo();
        actualizar_prom_tiempo();
        switch (sig_tipo_evento)
        {
        case 1:
            llegada();
            break;
        case 2:
            salida();
            break;
        }
    }

    reportes();
    fclose(parametros);
    fclose(resultados);
    return 0;
}

void inicializar(void)
{
    tiempo_simulacion = 0.0;
    tiempo_ultimo_evento = 0.0;
    servidores_ocupados = 0;
    clientes_en_cola = 0;
    num_clientes_atendidos = 0;
    num_clientes_que_esperaron = 0;
    area_estado_servidor = 0.0;
    area_num_en_cola = 0.0;
    total_espera_en_cola = 0.0;

    tiempo_sig_evento[1] = tiempo_simulacion + expon(media_entre_llegadas); // Llegada
    tiempo_sig_evento[2] = 1.0e+30;                                         // Salida (no programada aún)
}

void control_tiempo(void)
{
    float min_tiempo_sig_evento = 1.0e+29;
    sig_tipo_evento = 0;
    for (int i = 1; i <= num_eventos; ++i)
    {
        if (tiempo_sig_evento[i] < min_tiempo_sig_evento)
        {
            min_tiempo_sig_evento = tiempo_sig_evento[i];
            sig_tipo_evento = i;
        }
    }
    if (sig_tipo_evento == 0)
    {
        fprintf(resultados, "\nNo hay eventos futuros. Fin de la simulacion en %.3f minutos\n", tiempo_simulacion);
        exit(1);
    }
    tiempo_simulacion = min_tiempo_sig_evento;
}

void actualizar_prom_tiempo(void)
{
    float time_since_last_event = tiempo_simulacion - tiempo_ultimo_evento;
    area_num_en_cola += clientes_en_cola * time_since_last_event;
    area_estado_servidor += servidores_ocupados * time_since_last_event;
    tiempo_ultimo_evento = tiempo_simulacion;
}

void llegada(void)
{
    tiempo_sig_evento[1] = tiempo_simulacion + expon(media_entre_llegadas);

    if (servidores_ocupados < num_servidores)
    {
        // Hay un servidor libre, atiende de una vez
        servidores_ocupados++;
        num_clientes_atendidos++;
        // Programa salida de este cliente
        if (servidores_ocupados <= num_servidores)
            tiempo_sig_evento[2] = tiempo_simulacion + expon(media_atencion);
    }
    else
    {
        // Todos ocupados, cliente a la cola
        clientes_en_cola++;
        num_clientes_que_esperaron++;
        // No programa salida ahora, saldrá cuando se libere un servidor
        // Se puede guardar el tiempo de llegada a la cola si quieres calcular espera promedio
        // Por simplicidad, lo puedes sumar como total_espera_en_cola += tiempo_simulacion;
    }
}

void salida(void)
{
    if (clientes_en_cola > 0)
    {
        // Sale uno de la cola, entra al servidor
        clientes_en_cola--;
        num_clientes_atendidos++;
        // Calcular espera en cola
        // Suponiendo llegadas homogéneas, puedes aproximar la espera como la diferencia de tiempo
        // Para simulación fiel deberías guardar el tiempo de llegada de cada cliente a la cola
        // Aquí solo como ejemplo:
        // total_espera_en_cola += (tiempo_simulacion - tiempo_llegada_cliente_a_cola);
        // (para cálculo rápido, puedes no calcularlo por cliente si no te lo piden)
        // Programa salida de este cliente
        tiempo_sig_evento[2] = tiempo_simulacion + expon(media_atencion);
    }
    else
    {
        servidores_ocupados--;
        if (servidores_ocupados > 0)
        {
            tiempo_sig_evento[2] = tiempo_simulacion + expon(media_atencion);
        }
        else
        {
            tiempo_sig_evento[2] = 1.0e+30; // No hay más clientes en el sistema
        }
    }
}

float expon(float media)
{
    return -media * log(lcgrand(1));
}

void reportes(void)
{
    fprintf(resultados, "\nClientes que esperaron en cola:    %d\n", num_clientes_que_esperaron);
    fprintf(resultados, "Clientes atendidos:                %d\n", num_clientes_atendidos);
    fprintf(resultados, "Probabilidad de espera (Erlang C): %.3f\n",
            (float)num_clientes_que_esperaron / num_clientes_atendidos);
    fprintf(resultados, "Longitud promedio de la cola:      %.3f\n", area_num_en_cola / tiempo_simulacion);
    fprintf(resultados, "Uso promedio de servidores:        %.3f\n", area_estado_servidor / tiempo_simulacion);
    fprintf(resultados, "Tiempo de terminacion:             %.3f minutos\n", tiempo_simulacion);
}
