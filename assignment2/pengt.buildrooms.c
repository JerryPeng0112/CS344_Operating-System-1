#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define NUM_ROOMS 7         // Define the number of rooms
#define NUM_ROOM_NAMES 10   // Define the number of room names to choose from
#define MIN_CONNECTION 3    // Define the minimum connections for each room

struct Room {
        char* name;
        struct Room** connections;
        int num_connections;
        int cap_connections;
        int type;
        /* 1 for START_ROOM
           2 for MID_ROOM
           3 for END_ROOM */
};

struct Room* set_rooms();
void set_name(struct Room*);
char** generate_names();
void free_generated_names(char**);
void set_connections(struct Room*);
void init_room(struct Room*);
void add_connections(struct Room*, int, int);
void expand_connections(struct Room*, int);
void set_types(struct Room*);
void generate_rooms();
void free_rooms(struct Room*);

/*--------------------------------------------------------------
description: Helper function to print all the room info
parameter: Room*: arrayof room
return value: none
--------------------------------------------------------------*/
void print_rooms(struct Room* rooms) {
    int i, j;
    for(i = 0; i < 7; i++) {
        printf("Room Name: %s\n", rooms[i].name);
        for(j = 0; j < rooms[i].num_connections;j++) {
            printf("%s\n", rooms[i].connections[j]->name);
        }
        printf("Type: %d\n\n", rooms[i].type);
    }
}

void main(){
    srand(time(NULL));
    struct Room* rooms = set_rooms();
    generate_rooms(rooms);
    free_rooms(rooms);
}

/*--------------------------------------------------------------
description: Setup all the room info
parameter: none
return value: Room*: array of rooms
--------------------------------------------------------------*/
struct Room* set_rooms(){
    struct Room* rooms = malloc(sizeof(struct Room) * NUM_ROOMS);
    set_name(rooms);
    int i;
    set_connections(rooms);
    set_types(rooms);
    return rooms;
}

/*--------------------------------------------------------------
description: Set the names for the rooms
parameter: Room*: array of Rooms
return value: none
--------------------------------------------------------------*/
void set_name(struct Room* rooms_ptr){
    // add: condition to add.
    int i = 0, random = 0, size = 0, add = 1;
    int* nums = malloc(sizeof(int) * NUM_ROOMS);
    for(i = 0; i < NUM_ROOMS; i++) {
        nums[i] = -1;
    }
    // Generate the names
    char** room_names = generate_names();
    // initialize all the room names
    while(size < NUM_ROOMS) {
        random = rand() % NUM_ROOM_NAMES;
        i = 0;
        add = 1;
        for(i = 0; i < size; i++) {
            // If name is repeated, re-assign the room names
            if(nums[i] == random) {
                add = 0;
            }
        }
        if(add) {
            // If the room name is not repeated, add the room name
            nums[size] = random;
            rooms_ptr[size].name = malloc(strlen(room_names[random]) + 1);
            strcpy(rooms_ptr[size].name, room_names[random]);
            size++;
        }
    }
    free(nums);
    free_generated_names(room_names);
}

/*--------------------------------------------------------------
description: Generate all the names for the room (hard coded)
parameter: none
return value: char**: array of strings of all the names
--------------------------------------------------------------*/
char** generate_names(){
    // Number of room names 10
    char** room_names = malloc(sizeof(char*) * 10);
    // allocating spaces for the names
    room_names[0] = malloc(sizeof(char) * 12);
    room_names[1] = malloc(7);
    room_names[2] = malloc(9);
    room_names[3] = malloc(9);
    room_names[4] = malloc(8);
    room_names[5] = malloc(9);
    room_names[6] = malloc(8);
    room_names[7] = malloc(7);
    room_names[8] = malloc(13);
    room_names[9] = malloc(15);
    strcpy(room_names[0], "Living room");
    strcpy(room_names[1], "Attic");
    strcpy(room_names[2], "Basement");
    strcpy(room_names[3], "Ballroom");
    strcpy(room_names[4], "Kitchen");
    strcpy(room_names[5], "Bathroom");
    strcpy(room_names[6], "Bedroom");
    strcpy(room_names[7], "Garage");
    strcpy(room_names[8], "Meeting room");
    strcpy(room_names[9], "Second bedroom");
    return room_names;
}

/*--------------------------------------------------------------
description: To free all the hard-coded generated names
parameter: char**: array of strings
return value: none
--------------------------------------------------------------*/
void free_generated_names(char** room_names){
    int i;
    for(i = 0; i < NUM_ROOM_NAMES; i++) {
        free(room_names[i]);
    }
    free(room_names);
}

/*--------------------------------------------------------------
description: Set up the connections randomly between rooms
parameter: Room*: array of rooms
return value: none
--------------------------------------------------------------*/
void set_connections(struct Room* rooms){
    int i = 0;
    // initialize the rooms
    init_room(rooms);
    for(i = 0; i < NUM_ROOMS; i++) {
        // For each room, assign room connections unless max connecitons reached
        while(rooms[i].num_connections < NUM_ROOMS - 1) {
            int j, rand_room, connection_exist = 0;
            // Generate a random room name, and exit loop only when the
            // name is not existed in the current connections
            do {
                connection_exist = 0;
                rand_room = rand() % NUM_ROOMS;
                for(j = 0; j < rooms[i].num_connections; j++) {
                    if(strcmp(rooms[i].connections[j]->name, rooms[rand_room].name) == 0) {
                        connection_exist = 1;
                    }
                }
            } while(connection_exist);
            // If the room generated is not itself
            if(i != rand_room){
                add_connections(rooms, i, rand_room);
            }
            // If the room has more than minimum connection, exit loop
            if(rooms[i].num_connections >= MIN_CONNECTION) break;
        }
    }
}

/*--------------------------------------------------------------
description: initialize the Room arrays
parameter: Room*: array of rooms
return value: none
--------------------------------------------------------------*/
void init_room(struct Room* rooms){
    int i, j;
    for(i = 0; i < NUM_ROOMS; i++) {
        rooms[i].num_connections = 0;
        rooms[i].cap_connections = MIN_CONNECTION;
        // Initiate initial connections array with size of minimum connection
        rooms[i].connections = malloc(sizeof(struct Room*) * MIN_CONNECTION);
        for(j = 0; j < MIN_CONNECTION; j++){
            rooms[i].connections[j] = NULL;
        }
    }
}

/*--------------------------------------------------------------
description: Add the connections to the rooms randomly
parameter: Room*: array of rooms, int: first room, int: second room
return value: none
--------------------------------------------------------------*/
void add_connections(struct Room* rooms, int fir, int sec){
    // If any of the room connections array reach max cap, expand
    if(rooms[fir].num_connections == rooms[fir].cap_connections)
        expand_connections(rooms, fir);
    if(rooms[sec].num_connections == rooms[sec].cap_connections)
        expand_connections(rooms, sec);
    // Connect both of the room
    rooms[fir].connections[rooms[fir].num_connections] = &(rooms[sec]);
    rooms[sec].connections[rooms[sec].num_connections] = &(rooms[fir]);
    rooms[sec].num_connections++;
    rooms[fir].num_connections++;
}

/*--------------------------------------------------------------
description: Expand the connections variable (array)
parameter: Room*: array of rooms, int: index of the room
return value: none
--------------------------------------------------------------*/
void expand_connections(struct Room* rooms, int idx){
    int i;
    struct Room** temp = malloc(sizeof(struct Room*) * rooms[idx].cap_connections * 2);
    for(i = 0; i < rooms[idx].cap_connections; i++) {
        temp[i] = rooms[idx].connections[i];
    }
    free(rooms[idx].connections);
    rooms[idx].connections = temp;
    rooms[idx].cap_connections *= 2;
}

void set_types(struct Room* rooms){
    int i = 0, random = 0, end_assign = 0;
    // Setup all rooms to MID_ROOM
    for(i = 0; i < NUM_ROOMS; i++) {
        rooms[i].type = 2;
    }
    while(!end_assign) {
        // Randomly find anyroom that's mid room and set it to Start
        random = rand() % NUM_ROOMS;
        if(rooms[random].type == 2) {
            rooms[random].type = 1;
            end_assign = 1;
        }
    }
    end_assign = 0;
    while(!end_assign) {
        // Randomly find anyroom that's mid room and set it to End
        random = rand() % NUM_ROOMS;
        if(rooms[random].type == 2) {
            rooms[random].type = 3;
            end_assign = 1;
        }
    }
}

/*--------------------------------------------------------------
description: Generate the room files
parameter: Room*: array of rooms
return value: none
--------------------------------------------------------------*/
void generate_rooms(struct Room* rooms){
    pid_t pid = getpid();
    char buffer [30], buffer2[50];
    // Appending strings to make directory name with process ID
    sprintf(buffer, "pengt.rooms.%d", pid);
    mkdir(buffer, 0755);
    FILE *fp;
    char room_type [12];
    int i, j;
    for(i = 0; i < NUM_ROOMS; i++) {
        // Reuse buffer variable for room names
        memset(buffer2, '\0', sizeof(buffer));
        sprintf(buffer2, "./%s/%s", buffer, rooms[i].name);
        fp = fopen(buffer2, "w");
        // Write room infos into the files
        fprintf(fp, "ROOM NAME: %s\n", rooms[i].name);
        for(j = 0; j < rooms[i].num_connections; j++) {
            fprintf(fp, "CONNECTION %d: %s\n", j + 1, rooms[i].connections[j]->name);
        }
        switch(rooms[i].type) {
            case 1: strcpy(room_type, "START_ROOM");
            break;
            case 2: strcpy(room_type, "MID_ROOM");
            break;
            case 3: strcpy(room_type, "END_ROOM");
            break;
        }
        fprintf(fp, "ROOM TYPE: %s\n", room_type);
        fclose(fp);
    }
}

/*--------------------------------------------------------------
description: Free all the room variables
parameter: Room*: array of rooms
return value: none
--------------------------------------------------------------*/
void free_rooms(struct Room* rooms){
    int i;
    for(i = 0; i < NUM_ROOMS; i++) {
        free(rooms[i].name);
        free(rooms[i].connections);
    }
    free(rooms);
}
