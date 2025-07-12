#include <stdio.h>
#include <stdlib.h>
#include "lcgrand.cpp"

#define LIMITE_COLA 1200
#define OCUPADO 1
#define LIBRE 0

// Parámetros de la simulación
int N = 5;               // Capacidad del sistema (N)
int m = 5;               // Número de servidores
float p = 0.8;           // Probabilidad de llegada por slot
float s = 0.4;           // Probabilidad de servicio por slot
int num_slots = 1000000; // Número de slots a simular

// Variables de estado
int n = 0;           // Número de clientes en el sistema (incluyendo en servicio y en cola)
int en_servicio = 0; // Clientes siendo atendidos (<= m)
int total_llegadas = 0, total_salidas = 0, total_bloqueos = 0;
int histograma[100] = {0}; // Para P_n
FILE *resultados;

// Prototipos
void inicializar();
void llegada();
void salida();
void reportes();

int main()
{
    resultados = fopen("resultE.txt", "w");
    if (!resultados)
    {
        printf("No se pudo abrir result.txt\n");
        return 1;
    }

    inicializar();

    // Simulación por slots (tiempo discreto)
    for (int slot = 0; slot < num_slots; ++slot)
    {
        // Guarda el estado para estimar la función de densidad
        if (n < 100)
            histograma[n]++;

        // Llegada
        if (lcgrand(1) < p)
            llegada();

        // Salida de todos los servidores ocupados
        int salidas_este_slot = 0;
        for (int i = 0; i < en_servicio; ++i)
        {
            if (lcgrand(2) < s)
                salidas_este_slot++;
        }
        for (int j = 0; j < salidas_este_slot; ++j)
            salida();
    }

    reportes();
    fclose(resultados);
    return 0;
}

void inicializar()
{
    n = 0;
    en_servicio = 0;
    total_llegadas = 0;
    total_salidas = 0;
    total_bloqueos = 0;
    for (int i = 0; i < 100; i++)
        histograma[i] = 0;
}

void llegada()
{
    total_llegadas++;
    if (n < N)
    {
        n++;
        if (en_servicio < m)
            en_servicio++;
    }
    else
    {
        total_bloqueos++;
    }
}

void salida()
{
    if (en_servicio > 0 && n > 0)
    {
        en_servicio--;
        n--;
        total_salidas++;
        // Si hay clientes esperando en cola, uno entra a servicio
        if (n >= en_servicio + 1 && en_servicio < m)
            en_servicio++;
    }
}

void reportes()
{
    fprintf(resultados, "Modelo Geom/Geom/%d/%d (Bernoulli llegada p=%.2f, servicio s=%.2f)\n", m, N, p, s);
    fprintf(resultados, "Slots simulados: %d\n", num_slots);
    fprintf(resultados, "Total llegadas: %d\n", total_llegadas);
    fprintf(resultados, "Total salidas: %d\n", total_salidas);
    fprintf(resultados, "Total bloqueos: %d\n", total_bloqueos);
    fprintf(resultados, "Probabilidad de bloqueo estimada: %.4f\n", (float)total_bloqueos / total_llegadas);
    fprintf(resultados, "\nFunción de densidad estimada (P_n):\n");
    for (int i = 0; i <= N; i++)
        fprintf(resultados, "P_%d = %.4f\n", i, (float)histograma[i] / num_slots);
}
