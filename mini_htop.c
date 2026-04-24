#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/sysinfo.h>


float cpu_usage() {
    static unsigned long prev_idle = 0, prev_total = 0;

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Failed to open /proc/stat");
        return 0;
    }

  
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal; // Variables to hold CPU time values read from /proc/stat

    fscanf(fp, "%s %lu %lu %lu %lu %lu %lu %lu %lu",
           cpu, &user, &nice, &system, &idle,
           &iowait, &irq, &softirq, &steal);

    fclose(fp);
        
    char cpu_name[5];
    unsigned long idle_total = idle + iowait;
    unsigned long total = user + nice + system + idle + iowait + irq + softirq + steal;

    unsigned long delta_total = total - prev_total; // Calculate the change in total CPU time since the last measurement
    unsigned long delta_idle = idle_total - prev_idle; // Calculate the change in idle CPU time since the last measurement

    prev_total = total; // Update previous total CPU time for the next measurement for the next measurement
    prev_idle = idle_total; // Update previous idle CPU time for the next measurement

    if (delta_total == 0)
        return 0;

    return (double)(delta_total - delta_idle) * 100.0 / delta_total; // Calculate  CPU usage percentage based on the change in total and idle CPU time
}


float mem_usage() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Failed to open /proc/meminfo");
        return 0;
    }

    char label[128]; 
    unsigned long total = 0, available = 0;

    while (fscanf(fp, "%s %lu", label, &total) != EOF) {
        if (strcmp(label, "MemTotal:") == 0) {
            continue;
        }
        if (strcmp(label, "MemAvailable:") == 0) {
            available = total;
        }
    }

    fclose(fp);

    if (available == 0)
        return 0;

    return (double)(total - available) * 100.0 / total;
}


void list_processes() {
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("Failed to open /proc");
        return;
    }

    struct dirent *entry; // Pointer to directory entry,  struct used to read entries in /proc

    printf("\nPID\tName\n");
    printf("----------------------\n");

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {// Check if it's a directory and starts with a digit to make sure it's a process directory
            char path [256];
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name); //write the path to the comm file of the process, which contains the process name

            FILE *fp = fopen(path, "r");// Open the comm file to read the process name
            if (fp) { 
                char name[256];
                fgets(name, sizeof(name), fp); 
                name[strcspn(name, "\n")] = 0; 

                printf("%s\t%s\n", entry->d_name, name); 

                fclose(fp);
            }
        }
    }

    closedir(proc);
}
void print_bar(float percent) {
    int len = 20;
    int filled = (percent / 100.0) * len; // Calculate how many characters to fill based on percentage

    printf("[");
    for (int i = 0; i < len; i++) {
        if (i < filled) printf("#"); // Print filled part
        else printf("-"); // Print empty part
    }
    printf("]");
}

long uptime() {
    struct sysinfo s; // Structure to hold system information
    sysinfo(&s);// Get system information, including uptime
    return s.uptime;// Return uptime in seconds
}

int number_processes() {
    DIR *proc = opendir("/proc"); // Open /proc directory

    struct dirent *e;//pointer to directory entry
    int count = 0;

    while ((e = readdir(proc)) != NULL) { // Read each entry in /proc
        if (isdigit(e->d_name[0]))// If the entry name starts with a digit, it's a process directory-> 3lshan lw esm msh number hyb2a msh process
            count++;
    }

    closedir(proc);
    return count;
}

int main() {
    char ch;

    while (1) {
    system("clear");

    printf("=========== MINI HTOP ===========\n\n");

    float cpu = cpu_usage();
    float mem = mem_usage();

    printf("CPU ");
    print_bar(cpu);
    printf(" %.2f%%\n", cpu);

    printf("MEM ");
    print_bar(mem);
    printf(" %.2f%%\n\n", mem);

    printf("PROC: %d\n", number_processes());
    printf("UPTIME: %ld sec\n\n", uptime());

    list_processes();

    printf("\nPress 'q' then Enter to quit\n");

    sleep(1);

    char ch;
    if (scanf(" %c", &ch) == 1) {
        if (ch == 'q') break;
    }
}

    return 0;
}  

