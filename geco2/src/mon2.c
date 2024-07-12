#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


extern volatile bool keep_running;
extern int peak_usage;
extern int avg_usage;

int arr[60];
int i,count=0;

void iniArr(){
    for(i=0;i<60;i++){
        arr[i]=0;
    }
}

void setUsage(){
    int sum=0,max=arr[0];
    for(i=0;i<60;i++){
        sum+=arr[i];
        if(max < arr[i]){
            max = arr[i];
        }
    }
    avg_usage = sum/count;
    peak_usage = max;
}



void get_process_info(const char* process_name) {
    char cmd[128];
    char buffer[128];
    FILE* fp;
    char pid[16];

    // Step 1: Find the PID of the process
    snprintf(cmd, sizeof(cmd), "pgrep -f \"%s\"", process_name);
    //snprintf(cmd, sizeof(cmd), "ps -eo pid,args | grep -w \"%s\" | grep -v grep | awk '{print $1}' | head -n 1", process_name); //works flawlessly

    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return;
    }
    
    if (fgets(pid, sizeof(pid), fp) == NULL) {
        printf("Process not found\n");
        pclose(fp);
        return;
    }
    pclose(fp);

    // Remove newline character from pid
    pid[strcspn(pid, "\n")] = '\0';

    printf("PID of the process: %s\n", pid);

    while (keep_running && count < 60) {
        // Step 2: Find the CPU usage of the process using the PID
        snprintf(cmd, sizeof(cmd), "ps -p %s -o %%cpu", pid);

        fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed");
            return;
        }
        
        // Skip the header line
        if (fgets(buffer, sizeof(buffer), fp) == NULL) {
            printf("Failed to retrieve CPU usage\n");
            pclose(fp);
            return;
        }

        // Get the CPU usage
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            //printf("CPU usage of the process: %s%%\n", buffer);
            int cpu_usage = atoi(buffer);
            arr[count++] = cpu_usage;
        } 
        else {
            printf("Failed to retrieve CPU usage\n");
        }

        pclose(fp);

        ////////////////////////////////////////////

        // Sleep for a while before checking again
        sleep(1);
    }
    setUsage();
}

void* monitor_cpu_usage(void* arg) {
    iniArr();

    const char* process_name = "./GeDe2"; // The name of the current executable
    get_process_info(process_name);
    return NULL;
}
