#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#define MAX_NAME_WIDTH 10

struct Network {
    char * connected;
    char * name;
    char * strength;
};

typedef struct List {
    struct Network network;
    struct List * next;
} List;

char * get_signal_strength(int rx, int tx) {
    if (rx > 80 && tx > 60) {
        return "󰤨";
    } else if (rx > 50 && tx > 30) {
        return "󰤥";
    } else if (rx > 30 && tx > 10) {
        return "󰤢";
    } else if (rx > 10 && tx > 5) {
        return "󰤟";
    }
    return "󰤮";
}

char * format_networks(struct Network n) {
    int i = 1;
    char formatted[128] = "";

    if(n.connected == NULL) {
        n.connected = " ";
        strcpy(formatted, n.connected);
    } else {
        n.connected = "";
        i = 3;
        strcpy(formatted, n.connected);
    }

    if(n.strength == NULL) {
        // TODO: Need to get the signal strength of networks that are not connected
        n.strength = get_signal_strength(0, 0);
    }

    int counter = 0;
    for(; i < 25; i++) {
        if(i == 3 && strcmp(n.connected, "") == 0) {
            formatted[i] = ' ';
            formatted[i + 1] = ' ';
            formatted[i + 2] = ' ';

            i += 2;
            continue;
        }

        if(i > 3 && i < 16 && counter < 11 && isalnum(n.name[counter])) {
            formatted[i] = n.name[counter];
            counter++;

        } else formatted[i] = ' ';
    }

    if(strcmp(n.connected, "") == 0) {
        formatted[i] = ' ';
        formatted[i + 1] = ' ';
    }
    strcat(formatted, n.strength);

    char * formatted_ptr = malloc(strlen(formatted) + 1);
    strcpy(formatted_ptr, formatted);

    return formatted_ptr; 
}

int correct_file() {
    FILE * originalFile = fopen("/tmp/available.network", "r");
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
    if (rename("/tmp/temp.txt", "/tmp/available.network") != 0) {
        return 1;
    }

    return 0;
}

struct Network get_connected_network() {
    struct Network n = {
        .connected = NULL,
        .name = NULL,
        .strength = NULL
    };

    FILE * file = fopen("/tmp/connected.network", "r");
    if (file == NULL) {
        fclose(file);
        return n;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * name;
    int rx = -1;
    int tx = -1;

    while ((read = getline(&line, &len, file)) != -1) {
        if (n.name == NULL) {
            n.connected = "";
            n.name = malloc(strlen(line) + 1);
            strncpy(n.name, line, strlen(line) - 1);
        } else if (rx == -1) {
            rx = atoi(line);
        } else if (tx == -1) {
            tx = atoi(line);
        } else {
            n.strength = get_signal_strength(rx, tx);
            fclose(file);

            return n;
        }
    }

    n.strength = get_signal_strength(rx, tx);
    fclose(file);

    return n;
}

struct List get_networks() {
    struct List * head = malloc(sizeof(struct List));
    struct List * current = head;

    FILE * file = fopen("/tmp/available.network", "r");

    if (file == NULL) {
        fclose(file);
        return * head;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * name;

    while ((read = getline(&line, &len, file)) != -1) {
        struct Network n = {
            .connected = NULL,
            .name = NULL,
            .strength = NULL
        };

        // Trim newline character from the end of the line
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        n.name = malloc(strlen(line));
        strncpy(n.name, line, strlen(line));

        current->network = n;
        current->next = malloc(sizeof(struct List));

        current = current->next;
    }

    fclose(file);

    return * head;
}

void traverse(struct List * head, FILE * file) {
    struct List * current = head;
    while (current->next != NULL) {
        fprintf(file, "%s\n", format_networks(current->network));
        current = current->next;
    }
}

void setup_files() {
    // Create and populate the /tmp/connected.network file with appropriate information
    system("iwctl station wlan0 show | grep 'Connected network' | awk '{print $3}' > /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Rx' | awk '{print $2}' >> /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Tx' | awk '{print $2}' >> /tmp/connected.network");

    // Create and populate the /tmp/available.network file with appropriate information
    system("iwctl station wlan0 get-networks | grep -e psk -e open -e 8021x | awk '{print $1}' > /tmp/available.network");

    correct_file();
}

int is_connected(struct Network n) {
    if(n.connected == NULL) {
        return 1;
    }

    return 0;
}

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
    struct Network conn_network = get_connected_network();
    struct List avail_networks = get_networks();
    int connected = is_connected(conn_network);

    FILE * file = fopen("/tmp/networks.txt", "w");
    if(file == NULL) {
        printf("Error opening /tmp/networks.txt!\n");
        fclose(file);
        return;
    }

    conn_network = get_connected_network();
    avail_networks = get_networks();
    connected = is_connected(conn_network);

    fprintf(file, "%s\n", (connected == 0) ? format_networks(conn_network) : "");
    traverse(&avail_networks, file);

    fclose(file);
}

int main(int argc, char * argv[]) {
    setup_files();
    struct Network conn_network = get_connected_network();
    struct List avail_networks = get_networks();
    int connected = is_connected(conn_network);

    FILE * file = fopen("/tmp/networks.txt", "w");
    if(file == NULL) {
        printf("Error opening /tmp/networks.txt!\n");
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
            fprintf(file, "%s\n", (connected == 0) ? format_networks(conn_network) : "");

        } else if(strcmp(argv[1], "--connected-strength") == 0 || strcmp(argv[1], "-cs") == 0) {
            fprintf(file, "%s\n", (connected == 0) ? conn_network.strength : "󰤮");

        } else if(strcmp(argv[1], "--available") == 0 || strcmp(argv[1], "-a") == 0) {
            traverse(&avail_networks, file);

        } else if(strcmp(argv[1], "--all") == 0 || strcmp(argv[1], "-l") == 0) {
            fprintf(file, "%s\n", (connected == 0) ? format_networks(conn_network) : "");
            traverse(&avail_networks, file);

        } else if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Usage: get-networks [OPTION]\n");
            printf("Prints the connected network and available networks.\n\n");
            printf("Options:\n");
            printf("  -d, --daemonize               daemonize the process and update the network information every 5 seconds\n");
            printf("  -c, --connected               print the connected network\n");
            printf("  -cs, --connected-strength     print the connected network's signal strength\n");
            printf("  -a, --available               print the available networks\n");
            printf("  -l, --all                     print the connected and available networks\n");
            printf("  -h, --help                    display this help and exit\n");
            printf("  -v, --version                 output version information and exit\n");

        } else if(strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("get-networks 0.1\n");

        }
    } else {
        fprintf(file, "%s\n", (connected == 0) ? format_networks(conn_network) : "");
        traverse(&avail_networks, file);

    }

    fclose(file);
    return 0;
}
