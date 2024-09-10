#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pid; //Process ID (Indice del proceso)
    int arrival_time; //Arrival time (Tiempo de llegada) = AT
    int burst_time; //Burst time (Tiempo de ejecucion) = BT
    int priority; //Priority (se usa para MLQ) = P
    int remaining_time; //Remaining burst time (se usa para MLFQ) = RT
    int start_time; //Start time of the process (Tiempo de comienzo)
    int completed_time; //Completed time (Tiempo completado) = CT
    int waiting_time; //Waiting time = WT
    int turnaround_time; //Turnaround time = TAT
} Process;

//Funcion para abrir archivo de procesos
void read_processes(char *filename, Process processes[], int *n) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: No se pudo abrir el archivo\n");
        exit(1);
    }
    //n = Contador de procesos leidos
    *n = 0;
    //Leer y almacenar datos en el arreglo mientras la funcion no retorne EOF
    while (fscanf(file, "%d %d %d %d", &processes[*n].pid, &processes[*n].arrival_time, 
                  &processes[*n].burst_time, &processes[*n].priority) != EOF) {
        processes[*n].remaining_time = processes[*n].burst_time;
        processes[*n].start_time = -1;  //-1 indica que ST aún no se ha ejecutado
        (*n)++; //Incrementar contador de linea(proceos)
    }
    fclose(file);
}

//Mejora: current_time como puntero a fcfs para asegurar que el tiempo se comparta y acumule correctamente entre las colas

//Funcion First Come First Served
void fcfs(Process queue[], int count, int *total_waiting_time, int *total_turnaround_time, int *current_time) {
    
    //Ajustar el tiempo actual si el proceso llega después del tiempo actual
    for (int i = 0; i < count; i++) {
        if (queue[i].arrival_time > *current_time) {
            *current_time = queue[i].arrival_time;
        }
        queue[i].start_time = *current_time;
        queue[i].completed_time = *current_time + queue[i].burst_time;  //CT
        queue[i].waiting_time = queue[i].start_time - queue[i].arrival_time;  //WT
        queue[i].turnaround_time = queue[i].completed_time - queue[i].arrival_time; //TAT
        //El tiempo actual se establece cuando el proceso terminó.
        *current_time = queue[i].completed_time;  
        
        //Acumular tiempos para promedios
        *total_waiting_time += queue[i].waiting_time;
        *total_turnaround_time += queue[i].turnaround_time;
        
        //Mostrar resultados
        printf("Proceso %d ejecutado desde %d hasta %d\n", queue[i].pid, queue[i].start_time, queue[i].completed_time);
    }
}

//Mejora: current_time como puntero a fcfs para asegurar que el tiempo se comparta y acumule correctamente entre las colas

//Funcion mlq basado en múltiples niveles de colas con FCFS
void mlq(Process processes[], int n) {
    //Colas de prioridad de tipo Process
    Process queue1[n], queue2[n], queue3[n];
    //Numero de procesos en cada cola de prioridad
    int count1 = 0, count2 = 0, count3 = 0;
    //Variables de tiempo
    int total_waiting_time = 0;
    int total_turnaround_time = 0;
    int current_time = 0;

    //Separar procesos en colas según su prioridad
    for (int i = 0; i < n; i++) {
        if (processes[i].priority == 1)
            queue1[count1++] = processes[i];
        else if (processes[i].priority == 2)
            queue2[count2++] = processes[i];
        else
            queue3[count3++] = processes[i];
    }
    
    //Implementar FCFS
    printf("Ejecucion del MLQ:\n");
    
    //Procesa la cola FCFS y actualiza tiempos de WT, TAT y tiempo actual (recibe queue, numero de procesos y direcciones(WT y TAT))
    fcfs(queue1, count1, &total_waiting_time, &total_turnaround_time, &current_time);
    fcfs(queue2, count2, &total_waiting_time, &total_turnaround_time, &current_time);
    fcfs(queue3, count3, &total_waiting_time, &total_turnaround_time, &current_time);

    //Calcular y mostrar promedios
    int total_processes = count1 + count2 + count3;
    printf("Tiempo promedio de WT en MLQ: %.2f\n", (float)total_waiting_time / total_processes);
    printf("Tiempo promedio de TAT en MLQ: %.2f\n", (float)total_turnaround_time / total_processes);
}



//Funcion mlfq basado en múltiples colas de prioridad con Round Robin
void mlfq(Process queue1[], Process queue2[], Process queue3[], int count1, int *count2, int *count3, int quantum[], int *total_waiting_time, int *total_turnaround_time) {
    int current_time = 0;
    int cola_index = 0;
    //Puntero que apunta a la cola de procesos queue1
    Process *current_queue = queue1;
    //Puntero a la cantidad de procesos en queue1
    int *current_count = &count1;
    //Mientras haya colas de prioridad
    while (cola_index < 3) {
        //Iterar sobre los procesos en la cola actual
        for (int i = 0; i < *current_count; i++) {
            //Ajustar el tiempo actual si el proceso llega después del tiempo actual
            if (current_queue[i].arrival_time > current_time) {
                current_time = current_queue[i].arrival_time;
            }
            int time_slice = quantum[cola_index];
            //Tiempo de quantum para la cola actual
        
            
            //Inicializar start_time solo una vez
            if (current_queue[i].start_time == -1) {  
                current_queue[i].start_time = current_time;
            }
            
            if (current_queue[i].remaining_time > time_slice) {
                //El proceso no termina en este quantum
                current_queue[i].remaining_time -= time_slice;
                current_time += time_slice;
                //Mover a la siguiente cola (castigo)
                if (cola_index == 0) {
                    queue2[(*count2)++] = current_queue[i];
                } else if (cola_index == 1) {
                    queue3[(*count3)++] = current_queue[i];
                }
            } else {
                //El proceso termina
                current_time += current_queue[i].remaining_time; //Tiempo actual
                current_queue[i].remaining_time = 0; //Tiempo restante 0
                current_queue[i].completed_time = current_time; //CT
                current_queue[i].waiting_time = current_queue[i].start_time - current_queue[i].arrival_time; //WT
                current_queue[i].turnaround_time = current_queue[i].completed_time - current_queue[i].arrival_time; //TAT

                //Acumular tiempos para promedios
                *total_waiting_time += current_queue[i].waiting_time;
                *total_turnaround_time += current_queue[i].turnaround_time;

                printf("Proceso %d ejecutado desde %d hasta %d\n", current_queue[i].pid, current_queue[i].start_time, current_queue[i].completed_time);
            }
        }
        //Cambiar a la siguiente cola
        cola_index++;
        if (cola_index == 1) {
            current_queue = queue2;
            current_count = count2;
        } else if (cola_index == 2) {
            current_queue = queue3;
            current_count = count3;
        }
    }
}

//Funcion para preparar lo necesario (iniciar colas y contadores, asignar procesos e implementar mlfq)
void init_mlfq(Process processes[], int n) {
    //Suponer que tenemos 3 colas con diferentes quantum
    int quantum[3] = {4, 8, 16};
    Process queue1[n], queue2[n], queue3[n];
    int count1 = 0, count2 = 0, count3 = 0;
    int total_waiting_time = 0, total_turnaround_time = 0;

    //Asignar procesos a la primera cola inicialmente
    for (int i = 0; i < n; i++) {
        queue1[count1++] = processes[i];
    }
    
    //Implementar MLFQ
    printf("Ejecucion con MLFQ:\n");

    //Procesa múltiples colas con MLFQ, actualiza conteos y tiempos WT y TAT (recibe queues, numero de procesos, quantum y direcciones(WT y TAT))
    mlfq(queue1, queue2, queue3, count1, &count2, &count3, quantum, &total_waiting_time, &total_turnaround_time);

    //Calcular y mostrar promedios
    int total_processes = count1 + count2 + count3;
    printf("Tiempo promedio de WT en MLFQ: %.2f\n", (float)total_waiting_time / total_processes);
    printf("Tiempo promedio de turnaround en MLFQ: %.2f\n", (float)total_turnaround_time / total_processes);
}

int main() {
    //Arreglo tipo Process para almacenar hasta 100 procesos
    Process processes[100];
    
    //n es para contar la cantidad de procesos presentes en el archivo processes.txt"
    int n;
    
    read_processes("processes.txt", processes, &n);
    
    printf("Multilevel Queue Scheduling:\n");
    mlq(processes, n);
    
    printf("\nMultilevel Feedback Queue Scheduling:\n");
    init_mlfq(processes, n);
    
    return 0;
}
