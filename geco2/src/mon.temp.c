#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


extern volatile bool keep_running;
extern int peak_usage;
extern int avg_usage;
extern unsigned long mem_total, mem_free;

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

////////////////////

void get_memory_usage(unsigned long* total, unsigned long* free) {
    FILE* file = fopen("/proc/meminfo", "r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (sscanf(buffer, "MemTotal: %lu kB", total) == 1 ||
            sscanf(buffer, "MemFree: %lu kB", free) == 1) {
            // Do nothing, just parsing
        }
    }

    fclose(file);
}
    

////////////////////



void get_process_info(const char* process_name) {
    char cmd[128];
    char buffer[128];
    FILE* fp;
    char pid[16];

    // Step 1: Find the PID of the process
    
    //snprintf(cmd, sizeof(cmd), "pgrep -f \"%s\"", process_name);      // works but if process name mismatches still gives no error (pgrep or popen err m.p.)
    //snprintf(cmd, sizeof(cmd), "ps -eo pid,comm | grep -w \"%s\" | awk '{print $1}' | head -n 1", process_name);
    //snprintf(cmd, sizeof(cmd), "ps aux | grep -w \"%s\" | grep -v grep | awk '{print $2}' | head -n 1", process_name);      //works flawlessly
    //snprintf(cmd, sizeof(cmd), "ps -A | grep -w \"%s\" | awk '{print $1}' | head -n 1", process_name);
    //snprintf(cmd, sizeof(cmd), "pgrep -x \"%s\"", process_name);
    //snprintf(cmd, sizeof(cmd), "ps -e | grep -w \"%s\" | awk '{print $1}' | head -n 1", process_name);
    //snprintf(cmd, sizeof(cmd), "pgrep -u $(whoami) -x \"%s\"", process_name);
    //snprintf(cmd, sizeof(cmd), "ps -eo pid,args | grep -w \"%s\" | grep -v grep | awk '{print $1}' | head -n 1", process_name); //works flawlessly
    //snprintf(cmd, sizeof(cmd), "ps -C \"%s\" -o pid= | head -n 1", process_name);
    //snprintf(cmd, sizeof(cmd), "ps -eo pid,comm | grep -i -w \"%s\" | awk '{print $1}' | head -n 1", process_name);
    //snprintf(cmd, sizeof(cmd), "pgrep -f -u $(whoami) \"%s\"", process_name);  //same error with wrong process name

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
        //////// MEMORY USAGE /////////////////////
        get_memory_usage(&mem_total, &mem_free);
        ///////////////////////////////////////////

        // Sleep for a while before checking again
        sleep(1);
    }
    setUsage();
}

void* monitor_cpu_usage(void* arg) {
    iniArr();

    const char* process_name = "./GeCo2"; // The name of the current executable
    get_process_info(process_name);
    return NULL;
}
