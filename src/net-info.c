#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#define MAX_NAME_WIDTH 10

// Represents a network
struct Network {
    char * connected;
    char * name;
    char * strength;
};

// A singly linked list of networks
typedef struct List {
    struct Network network;
    struct List * next;
} List;

// TODO: Need to get the signal strength of networks that are not connected
// Returns the signal strength of a network based on the received and transmitted values
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

// Formats a network's information into a string
char * format_networks(struct Network n) {
    // The first three characters are reserved for the connection status
    int i = 1;
    char formatted[128] = "";

    // If the network is not connected, display a space
    if(n.connected == NULL) {
        n.connected = " ";
        strcpy(formatted, n.connected);
    } else {
        n.connected = "";
        i = 3;
        strcpy(formatted, n.connected);
    }

    if(n.strength == NULL) {
        n.strength = get_signal_strength(0, 0);
    }

    // If the network name is longer than 10 characters, truncate it
    int counter = 0;
    for(; i < 40; i++) {
        // If the network is connected, properly format the spacing
        if(i == 3 && strcmp(n.connected, "") == 0) {
            formatted[i] = ' ';
            formatted[i + 1] = ' ';
            formatted[i + 2] = ' ';

            i += 2;
            continue;
        }

        // For each character in the network name, add it to the formatted string if
        // it is alphanumeric and the name index is within 10
        if(i > 3 && i < 16 && counter < 11 && isalnum(n.name[counter])) {
            formatted[i] = n.name[counter];
            counter++;

        } else formatted[i] = ' ';
    }

    // If the network is connected, properly format the spacing
    if(strcmp(n.connected, "") == 0) {
        formatted[i] = ' ';
        formatted[i + 1] = ' ';
    }
    strcat(formatted, n.strength);

    // Allocate memory for the formatted string and return it
    char * formatted_ptr = malloc(strlen(formatted) + 1);
    strcpy(formatted_ptr, formatted);
    return formatted_ptr; 
}

// Corrects the /tmp/available.network file by removing blank lines
int correct_file() {
    FILE * originalFile = fopen("/tmp/available.network", "r");
    FILE * newFile = fopen("/tmp/temp.txt", "w");

    // Check if the files were opened successfully
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

// Returns the network that the device is connected to
struct Network get_connected_network() {
    // Initialize the network struct
    struct Network n = {
        .connected = NULL,
        .name = NULL,
        .strength = NULL
    };

    // Open the connected.network file
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

    // Read the file line by line
    while ((read = getline(&line, &len, file)) != -1) {
        if (n.name == NULL) {
            n.connected = "";
            n.name = malloc(strlen(line) + 1);
            // Trim newline character from the end of the line
            strncpy(n.name, line, strlen(line) - 1);
        } else if (rx == -1) {
            rx = atoi(line);
        } else if (tx == -1) {
            tx = atoi(line);
        } else {
            // If the file has more than 3 lines, calculate the signal strength and return
            n.strength = get_signal_strength(rx, tx);
            fclose(file);

            return n;
        }
    }

    // If the file has less than 3 lines, calculate the signal strength and return
    n.strength = get_signal_strength(rx, tx);
    fclose(file);

    return n;
}

// Returns a list of available networks
struct List get_networks() {
    // Initialize the head and current nodes
    struct List * head = malloc(sizeof(struct List));
    struct List * current = head;

    // Open the available.network file
    FILE * file = fopen("/tmp/available.network", "r");
    if (file == NULL) {
        fclose(file);
        return * head;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * name;

    // Read the file line by line
    while ((read = getline(&line, &len, file)) != -1) {
        // Initialize the network struct for this position in the list
        struct Network n = {
            .connected = NULL,
            .name = NULL,
            .strength = NULL
        };

        // Trim newline character from the end of the line
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        // Allocate memory for the network name and copy the line into it
        n.name = malloc(strlen(line));
        strncpy(n.name, line, strlen(line));

        // Set the current node's network to the network struct and allocate memory for the next node
        current->network = n;
        current->next = malloc(sizeof(struct List));
        current = current->next;
    }

    fclose(file);

    return * head;
}

// Traverses the list of networks and prints them to the file
void traverse(struct List * head, FILE * file) {
    struct List * current = head;
    while (current->next != NULL) {
        fprintf(file, "%s\n", format_networks(current->network));
        current = current->next;
    }
}

// Sets up the /tmp/connected.network and /tmp/available.network files
void setup_files() {
    // Create and populate the /tmp/connected.network file with appropriate information
    system("iwctl station wlan0 show | grep 'Connected network' | awk '{print $3}' > /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Rx' | awk '{print $2}' >> /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Tx' | awk '{print $2}' >> /tmp/connected.network");

    // Create and populate the /tmp/available.network file with appropriate information
    system("iwctl station wlan0 get-networks | grep -e psk -e open -e 8021x | awk '{print $1}' > /tmp/available.network");

    correct_file();
}

// Returns 1 if the network is connected, 0 otherwise
int is_connected(struct Network n) {
    return (n.connected == NULL) ? 1 : 0;
}

void daemonize() {
    printf("daemonizing...\n");

    // Fork the process
    pid_t pid, sid;
    pid = fork();

    // Check if the fork was successful
    if (pid < 0) {
        printf("daemonizing failed! pid: %d\n", pid);
        exit(EXIT_FAILURE);
    }

    // If the fork was successful, exit the parent process
    if (pid > 0) {
        if ((chdir("/")) < 0) {
            printf("daemonizing failed! dir not changed.\n");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    } else exit(EXIT_FAILURE);
}

// Records the network information to a file, for use in the daemon and listing all networks.
void record() {
    // Set up the files and get the network information
    setup_files();
    struct Network conn_network = get_connected_network();
    struct List avail_networks = get_networks();
    int connected = is_connected(conn_network);

    // Open the file to write to
    FILE * file = fopen("/tmp/networks.txt", "w");
    if(file == NULL) {
        printf("Error opening /tmp/networks.txt!\n");
        fclose(file);
        return;
    }

    conn_network = get_connected_network();
    avail_networks = get_networks();
    connected = is_connected(conn_network);

    // Write the network information to the file
    fprintf(file, "%s\n", (connected == 0) ? format_networks(conn_network) : "");
    traverse(&avail_networks, file);

    fclose(file);
}

int main(int argc, char * argv[]) {
    // Set up the files and get the network information
    setup_files();
    struct Network conn_network = get_connected_network();
    struct List avail_networks = get_networks();
    int connected = is_connected(conn_network);

    // Open the file to write to
    FILE * file = fopen("/tmp/networks.txt", "w");
    if(file == NULL) {
        printf("Error opening /tmp/networks.txt!\n");
    }

    // Check the command line arguments and execute the appropriate command
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
