#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return "󰤟";
}

char * format_networks(struct Network n) {
    if(n.connected == NULL) {
        n.connected = " ";
    }

    if(n.strength == NULL) {
        n.strength = "󰤭";
    }

    int len = 15 - strlen(n.name);
    char name [12] = "";
    if(len <= 3) {
        strncpy(name, n.name, 11);
        len = 4;
    } else {
        strcpy(name, n.name);
    }

    char formatted[128] = ""; 

    strcat(formatted, n.connected);
    strcat(formatted, "   ");
    strcat(formatted, name);

    for (int i = 0; i < len; i++) {
        strcat(formatted, " ");
    }

    strcat(formatted, n.strength);
    char * formatted_ptr = malloc(strlen(formatted) + 1);
    strcpy(formatted_ptr, formatted);

    return formatted_ptr;
}

int correct_file() {
    FILE *originalFile = fopen("/tmp/available.network", "r");
    FILE *newFile = fopen("/tmp/temp.txt", "w");

    if (originalFile == NULL || newFile == NULL) {
        return 1;
    }

    // Skip the first line
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), originalFile) == NULL) {
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
        n.strength = "󰤯";
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
            n.connected = "";
            n.name = malloc(strlen(line) + 1);
            strncpy(n.name, line, strlen(line) - 1);
        } else if (rx == -1) {
            rx = atoi(line);
        } else if (tx == -1) {
            tx = atoi(line);
        } else {
            return n;
        }
    }

    n.strength = get_signal_strength(rx, tx);

    return n;
}

struct List get_networks() {
    struct List * head = malloc(sizeof(struct List));
    struct List * current = head;
    struct List * prev = NULL;

    FILE * file = fopen("/tmp/available.network", "r");

    if (file == NULL) {
        return * prev;
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

        n.name = malloc(strlen(line) + 1);
        strncpy(n.name, line, strlen(line) - 1);

        current->network = n;
        current->next = malloc(sizeof(struct List));

        current = current->next;
    }

    return * head;
}

void print(struct List * head) {
    struct List * current = head;
    while (current->next != NULL) {
        printf("%s\n", format_networks(current->network));
        current = current->next;
    }
}

void setup_files() {
    // Create and populate the /tmp/connected.network file with appropriate information
    system("iwctl station wlan0 show | grep 'Connected network' | awk '{print $3}' > /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Rx' | awk '{print $2}' >> /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Tx' | awk '{print $2}' >> /tmp/connected.network");

    // Create and populate the /tmp/available.network file with appropriate information
    system("iwctl station wlan0 get-networks | grep psk | awk '{print $1}' > /tmp/available.network");

    correct_file();
}

int is_connected(struct Network n) {
    if(n.name == NULL) {
        return 1;
    }

    return 0;
}

int main(int argc, char * argv[]) {
    setup_files();
    struct Network conn_network = get_connected_network();
    struct List avail_networks = get_networks();
    int connected = is_connected(conn_network);

    if(argc > 1) {
        if(strcmp(argv[1], "--connected") == 0 || strcmp(argv[1], "-c") == 0) {
            printf("%s\n", (connected == 0) ? format_networks(conn_network) : "");
        } else if(strcmp(argv[1], "--connected-strength") == 0 || strcmp(argv[1], "-cs") == 0) {
            printf("%s\n", conn_network.strength);
        } else if(strcmp(argv[1], "--available") == 0 || strcmp(argv[1], "-a") == 0) {
            print(&avail_networks);
        } else if(strcmp(argv[1], "--all") == 0 || strcmp(argv[1], "-l") == 0) {
            printf("%s\n", (connected == 0) ? format_networks(conn_network) : "");

            print(&avail_networks);
        } else if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Usage: get-networks [OPTION]\n");
            printf("Prints the connected network and available networks.\n\n");
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
        printf("%s\n", (connected == 0) ? format_networks(conn_network) : "");

        print(&avail_networks);
    }

    return 0;
}
