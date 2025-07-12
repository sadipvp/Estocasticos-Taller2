// Librerias estandar C++
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Generador de numeros aleatorios
#include "lcgrand.cpp"

// Constantes
#define OCUPADO 1 // Indicador de servidor ocupado
#define LIBRE 0   // Indicador de servidor libre

// Variables de estado del sistema
int num_servidores;         // Número de servidores
int estado_servidor;        // Indica si el servidor está vacío u ocupado
int clientes_perdidos;      // Número de clientes perdidos
float tiempo_ultimo_evento; // Indica el tiempo en que sucedió el último evento
int sig_tipo_evento;        // Indica el tipo del evento más próximo

// Variables de tiempo
float tiempo_simulacion;    // Tiempo actual (reloj) de la simulacion
float tiempo_sig_evento[3]; // Array que almacena el tiempo en que ocurrirá el proximo evento de cada tipo

// Contadores y acumuladores estadisticos
int num_clientes_atendidos; // Número de clientes atendidos (es decir, los que ya salieron o están en el servidor)
float area_estado_servidor; // Área bajo la función de la ocupación del servidor en el tiempo

// Tipos de eventos
// 1 = Llegada de un cliente
// 2 = Salida de un cliente
int num_eventos; // Cantidad de eventos

// Variables de entrada
float media_entre_llegadas; // Media del tiempo entre llegadas de los clientes
float media_atencion;       // Media del tiempo de atención de un cliente
int num_clientes;           // Número total de clientes que se deben atender

// Archivos de entrada y salida
FILE *parametros, *resultados;

// Declaracion de funciones
void inicializar(void);
void control_tiempo(void);
void llegada(void);
void salida(void);
void reportes(void);
void actualizar_prom_tiempo(void);
float expon(float mean);

// Funcion Principal
int main(void)
{
    // Abrir los archivos de entrada y salida
    parametros = fopen("params.txt", "r");
    resultados = fopen("resultsB.txt", "w");

    // Especifica el numero de eventos para la funcion control_tiempo.
    num_eventos = 2;

    // Lee y asigna las variables de entrada
    fscanf(parametros, "%f %f %d %d", &media_entre_llegadas, &media_atencion, &num_clientes, &num_servidores);

    // Escribe en el archivo de salida los encabezados del reporte y los parametros iniciales
    fprintf(resultados, "Sistema de Colas M/M/%d/%d\n", num_servidores, num_servidores);
    fprintf(resultados, "---------------------------------------------\n");
    fprintf(resultados, "Tiempo promedio de llegada:               %.3f minutos\n", media_entre_llegadas);
    fprintf(resultados, "Tiempo promedio de atencion:              %.3f minutos\n", media_atencion);
    fprintf(resultados, "Numero de clientes:                       %d\n\n", num_clientes);

    // Inicializa la simulacion.
    inicializar();

    // Corre la simulacion mientras no se atienda el numero de clientes especificado.
    while (num_clientes_atendidos < num_clientes)
    {
        // Determinar el siguiente evento
        control_tiempo();
        // Actualiza los acumuladores estadisticos de tiempo
        actualizar_prom_tiempo();
        // Invoca la funcion del siguiente evento
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

    // Generar reporte y finalizar la simulacion.
    reportes();
    fclose(parametros);
    fclose(resultados);
    return 0;
}

// Funcion de inicializacion.
void inicializar(void)
{
    // Inicializa el reloj de la simulacion.
    tiempo_simulacion = 0.0;

    // Inicializa las variables de estado
    estado_servidor = LIBRE;
    clientes_perdidos = 0;
    tiempo_ultimo_evento = 0.0;

    // Inicializa los contadores estadisticos.
    num_clientes_atendidos = 0;
    area_estado_servidor = 0.0;

    // Inicializamos la lista de eventos. Solo tenemos la primera llegada programada.
    tiempo_sig_evento[1] = tiempo_simulacion + expon(media_entre_llegadas);
    tiempo_sig_evento[2] = 1.0e+30;
}

// Control del tiempo de la simulacion
void control_tiempo(void)
{
    // Inicializa el tiempo del siguiente evento como un número muy grande
    float min_tiempo_sig_evento = 1.0e+29;
    sig_tipo_evento = 0;

    //  Determina el tipo de evento del siguiente evento que debe ocurrir
    for (int i = 1; i <= num_eventos; ++i)
    {
        // Revisa todos los eventos agendados y selecciona el que ocurra primero
        if (tiempo_sig_evento[i] < min_tiempo_sig_evento)
        {
            min_tiempo_sig_evento = tiempo_sig_evento[i];
            sig_tipo_evento = i;
        }
    }

    // Si la lista de eventos está vacia, se detiene la simulacion.
    if (sig_tipo_evento == 0)
    {
        fprintf(resultados, "\nLa lista de eventos está vacia. Terminando la simulacion.\n\n");
        fprintf(resultados, "Tiempo de terminacion de la simulacion: %8.3f minutos", tiempo_simulacion);
        exit(1);
    }

    // Si la lista de eventos no está vacia, se mueve el reloj al tiempo del siguiente evento.
    tiempo_simulacion = min_tiempo_sig_evento;
}

// Actualiza los acumuladores de area para las estadisticas de tiempo.
void actualizar_prom_tiempo(void)
{
    // Calcula el intervalo de tiempo que ha pasado desde el último evento
    float time_since_last_event = tiempo_simulacion - tiempo_ultimo_evento;
    // Actualiza el area bajo la funcion de ocupacion del servidor a lo largo del tiempo
    area_estado_servidor += estado_servidor * time_since_last_event;
    // Iguala el tiempo del ultimo evento al tiempo actual
    tiempo_ultimo_evento = tiempo_simulacion;
}

// Funcion de llegada
void llegada(void)
{
    // Se programa la siguiente llegada
    tiempo_sig_evento[1] = tiempo_simulacion + expon(media_entre_llegadas);

    // Revisa si el servidor está OCUPADO.
    if (estado_servidor == OCUPADO)
    {
        // Incrementa el número de clientes perdidos
        ++clientes_perdidos;
    }
    else
    {
        // Incrementa el número de clientes atendidos y pasa el servidor a ocupado
        ++num_clientes_atendidos;
        estado_servidor = OCUPADO;

        // Programa la salida del cliente que está siendo atendido
        tiempo_sig_evento[2] = tiempo_simulacion + expon(media_atencion);
    }
}

// Funcion de salida
void salida(void)
{
    // Pasa el servidor a LIBRE y no se tienen salidas programadas
    estado_servidor = LIBRE;
    tiempo_sig_evento[2] = 1.0e+30;
}

// Retorna una variable aleatoria exponencial con media "media"
float expon(float media)
{
    return -media * log(lcgrand(1));
}

// Calcula e imprime las medidas deseadas de desempeño
void reportes(void)
{
    fprintf(resultados, "Clientes perdidos:                        %d\n", clientes_perdidos);
    fprintf(resultados, "Clientes atendidos:                       %d\n", num_clientes_atendidos);
    fprintf(resultados, "Probabilidad de pérdida (B de Erlang):    %.3f\n", (float)clientes_perdidos / (clientes_perdidos + num_clientes_atendidos));
    fprintf(resultados, "Uso del servidor:                         %.3f\n", area_estado_servidor / tiempo_simulacion);
    fprintf(resultados, "Tiempo de terminacion de la simulacion:   %.3f minutos\n", tiempo_simulacion);
}