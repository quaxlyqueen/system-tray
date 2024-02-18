#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#define MAX_NAME_WIDTH 10

struct Device {
    char * connected;
    char * name;
};

typedef struct List {
    struct Device device;
    struct List * next;
} List;

char * format_devices(struct Device d) {
    int i = 1;
    char formatted[128] = "";

    if(d.connected == NULL) {
        d.connected = " ";
        strcpy(formatted, d.connected);
    } else {
        d.connected = "";
        i = 3;
        strcpy(formatted, d.connected);
    }

    int counter = 0;
    for(; i < 40; i++) {
        if(i == 3 && strcmp(d.connected, "") == 0) {
            formatted[i] = ' ';
            formatted[i + 1] = ' ';
            formatted[i + 2] = ' ';

            i += 2;
            continue;
        }

        if(i > 3 && i < 16 && counter < 11 && isalnum(d.name[counter])) {
            formatted[i] = d.name[counter];
            counter++;

        }
    }

    if(strcmp(d.connected, "") == 0) {
        formatted[i] = ' ';
        formatted[i + 1] = ' ';
    }

    char * formatted_ptr = malloc(strlen(formatted) + 1);
    strcpy(formatted_ptr, formatted);

    return formatted_ptr; 
}

int correct_file() {
    FILE * originalFile = fopen("/tmp/available.device", "r");
    FILE * newFile = fopen("/tmp/temp.txt", "w");

    if (originalFile == NULL || newFile == NULL) {
        fclose(originalFile);
        fclose(newFile);
        return 1;
    }

    // Skip the first line
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), originalFile) == NULL) {
        fclose(originalFile);
        fclose(newFile);
        return 1;
    }

    // Copy the rest of the file
    while (fgets(buffer, sizeof(buffer), originalFile) != NULL) {
        fputs(buffer, newFile);
    }

    fclose(originalFile);
    fclose(newFile);

    // Rename the new file to overwrite the original file
    if (rename("/tmp/temp.txt", "/tmp/available.device") != 0) {
        return 1;
    }

    return 0;
}


// TODO: Need to process the /tmp/connected.device file to get the connected device
struct Device get_connected_device() {
    struct Device d = {
        .connected = NULL,
        .name = NULL,
    };

    FILE * file = fopen("/tmp/connected.device", "r");
    if (file == NULL) {
        fclose(file);
        return d;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * name;

    fclose(file);

    return d;
}

// TODO: Need to process the /tmp/available.device file to get the available devices
struct List get_devices() {
    struct List * head = malloc(sizeof(struct List));
    struct List * current = head;

    FILE * file = fopen("/tmp/available.device", "r");

    if (file == NULL) {
        fclose(file);
        return * head;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * name;

    fclose(file);

    return * head;
}

void traverse(struct List * head, FILE * file) {
    struct List * current = head;
    while (current->next != NULL) {
        fprintf(file, "%s\n", format_devices(current->device));
        current = current->next;
    }
}

// TODO: Create and populate the /tmp/connected.device file with appropriate information
// TODO: Create and populate the /tmp/available.device file with appropriate information
void setup_files() {
    correct_file();
}

int is_connected(struct Device d) {
    if(d.connected == NULL) {
        return 1;
    }

    return 0;
}

// TODO: Need to fix the daemonize function.
void daemonize() {
    printf("daemonizing...\n");
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        printf("daemonizing failed! pid: %d\n", pid);
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        if ((chdir("/")) < 0) {
            printf("daemonizing failed! dir not changed.\n");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }

    if ((chdir("/")) < 0) {
        printf("daemonizing failed! dir not changed.\n");
        exit(EXIT_FAILURE);
    }
}

void record() {
    setup_files();
    struct Device conn_device = get_connected_device();
    struct List avail_devices = get_devices();
    int connected = is_connected(conn_device);

    FILE * file = fopen("/tmp/devices.txt", "w");
    if(file == NULL) {
        printf("Error opening /tmp/devices.txt!\n");
        fclose(file);
        return;
    }

    conn_device = get_connected_device();
    avail_devices = get_devices();
    connected = is_connected(conn_device);

    fprintf(file, "%s\n", (connected == 0) ? format_devices(conn_device) : "");
    traverse(&avail_devices, file);

    fclose(file);
}

int main(int argc, char * argv[]) {
    setup_files();
    struct Device conn_device = get_connected_device();
    struct List avail_devices = get_devices();
    int connected = is_connected(conn_device);

    FILE * file = fopen("/tmp/devices.txt", "w");
    if(file == NULL) {
        printf("Error opening /tmp/devices.txt!\n");
    }

    if(argc == 2) {
        if(strcmp(argv[1], "--daemonize") == 0 || strcmp(argv[1], "-d") == 0) {
            daemonize();
            fclose(file);

            while(1) {
                record();

                sleep(30);
            }

        } else if(strcmp(argv[1], "--connected") == 0 || strcmp(argv[1], "-c") == 0) {
            fprintf(file, "%s\n", (connected == 0) ? format_devices(conn_device) : "");

        } else if(strcmp(argv[1], "--available") == 0 || strcmp(argv[1], "-a") == 0) {
            traverse(&avail_devices, file);

        } else if(strcmp(argv[1], "--all") == 0 || strcmp(argv[1], "-l") == 0) {
            fprintf(file, "%s\n", (connected == 0) ? format_devices(conn_device) : "");
            traverse(&avail_devices, file);

        } else if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Usage: get-devices [OPTION]\n");
            printf("Prints the connected device and available devices.\n\n");
            printf("Options:\n");
            printf("  -d, --daemonize               daemonize the process and update the device information every 5 seconds\n");
            printf("  -c, --connected               print the connected device\n");
            printf("  -cs, --connected-strength     print the connected device's signal strength\n");
            printf("  -a, --available               print the available devices\n");
            printf("  -l, --all                     print the connected and available devices\n");
            printf("  -h, --help                    display this help and exit\n");
            printf("  -v, --version                 output version information and exit\n");

        } else if(strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("get-devices 0.1\n");

        }
    } else {
        fprintf(file, "%s\n", (connected == 0) ? format_devices(conn_device) : "");
        traverse(&avail_devices, file);

    }

    fclose(file);
    return 0;
}
