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
    struct List * prev;
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
    if(n.name == NULL) {
        return NULL;
    }

    if(n.strength == NULL) {
        n.strength = "󰤭";
    }

    if(n.connected == NULL) {
        n.connected = " ";
    }

    int len = 15 - strlen(n.name);
    char formatted[30] = ""; 

    strcat(formatted, n.connected);
    strcat(formatted, "   ");
    strcat(formatted, n.name);

    if(len < 3) {
        // Truncate the name (4 characters for connection status and spacing, 15 for the name truncated to 12)
        while(strlen(formatted) > 16) {
            formatted[strlen(formatted) - 1] = '\0';
        }
    }

    for (int i = 0; i < len; i++) {
        strcat(formatted, " ");
    }

    strcat(formatted, n.strength);
    char * formatted_ptr = malloc(strlen(formatted) + 1);
    strcpy(formatted_ptr, formatted);

    return formatted_ptr;
}

struct Network get_connected_network() {
    system("iwctl station wlan0 show | grep 'Connected network' | awk '{print $3}' > /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Rx' | awk '{print $2}' >> /tmp/connected.network");
    system("iwctl station wlan0 show | grep 'Tx' | awk '{print $2}' >> /tmp/connected.network");

    struct Network n = {
        .connected = NULL,
        .name = NULL,
        .strength = NULL
    };

    FILE * file = fopen("/tmp/connected.network", "r");
    if (file == NULL) {
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

    fclose(file);

    n.strength = get_signal_strength(rx, tx);

    return n;
}

struct List get_networks() {
    struct List * head = malloc(sizeof(struct List));
    struct List * current = head;
    struct List * prev = NULL;

    system("iwctl station wlan0 get-networks | grep psk | awk '{print $1}' > /tmp/available.network");
    FILE * file = fopen("/tmp/available.network", "r");

    if (file == NULL) {
        return * prev;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * name;

    while ((read = getline(&line, &len, file)) != -1) {
        if(strcmp(line, "\n") == 0) {
            continue;
        }

        struct Network n = {
            .connected = NULL,
            .name = NULL,
            .strength = NULL
        };

        n.name = malloc(strlen(line) + 1);
        strncpy(n.name, line, strlen(line) - 1);

        current->network = n;
        current->next = malloc(sizeof(struct List));
        current->prev = prev;

        prev = current;
        current = current->next;
    }

    fclose(file);

    return * head;
}

void print(struct List * head) {
    struct List * current = head;
    while (current->next != NULL) {
        printf("%s\n", format_networks(current->network));
        current = current->next;
    }
}

int main() {
    struct Network conn_network = get_connected_network();
    printf("%s\n", format_networks(conn_network));
    struct List avail_networks = get_networks();
    print(&avail_networks);
    return 0;
}
