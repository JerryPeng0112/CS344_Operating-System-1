#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define NUM_ROOMS 7 // Define the number of rooms

struct Room {
    char* name;                     // Name of the room
    struct Room** connections;      // The connections to other rooms
    char** string_connections;      // The name of the rooms connected
    int num_connections;            // The number of connections
    int cap_connections;            // The capaciy of connections
    int type;                       // 1 for Start, 2 for Mid, 3 for End
};

char* get_latest_dir();
char* find_latest_dir();
struct Room* init_rooms();
void create_room_from_dir(char*, struct Room*);
void get_room_info(struct Room*, char*, char*, int);
void add_info_to_room(struct Room*, char*, int);
char* extract_connections(char*);
void add_connections(char*, struct Room*, int);
void connect_rooms(struct Room*);
void adventure(struct Room*);
struct Room** expand_record(struct Room**, int);
struct Room* run_step(struct Room*, struct Room*);
void print_result(struct Room**, int);
void free_rooms(struct Room*);

int main(){
    char* path_name = get_latest_dir(); // Get the latest directory
    struct Room* rooms = init_rooms();  // Initialize the rooms variable
    create_room_from_dir(path_name, rooms); // Setup the rooms variable with files
    adventure(rooms); // Start the adventure game
    free_rooms(rooms); // Free the memory before exiting
    return 0;
}

/*--------------------------------------------------------------
description: prints time from localtime
parameter: none
return value: none
--------------------------------------------------------------*/
void print_time(){
    int i;
    FILE* file;
    time_t rawtime;
    struct tm* time_info;
    char buffer [80];
    time(&rawtime);
    time_info = localtime(&rawtime);
    // Formatting the time
    strftime(buffer, 80, "%l:%M%P, %A, %B %d, %Y\n", time_info);
    // Take out the space at the start
    for(i = 0; i < strlen(buffer); i++){
        buffer[i] = buffer[i+1];
    }
    // Open and write to the text file
    file = fopen("currentTime.txt", "w");
    fprintf(file, "%s", buffer);
    fclose(file);
    // Re-open and read the file and output to console
    file = fopen("currentTime.txt", "r");
    fgets(buffer, 80, file);
    printf("%s\n", buffer);
    fclose(file);
}

/*--------------------------------------------------------------
description: Get the latest directory from current directory
parameter: none
return value: string: name of the path appended on "./"
--------------------------------------------------------------*/
char* get_latest_dir(){
    char* latest_name;
    char* path_name;
    // Get the latest pengt.rooms.$$ directory's name
    latest_name = find_latest_dir();
    path_name = malloc(sizeof(char) * 25);
    // Joining the pathname with "./"
    sprintf(path_name, "./%s", latest_name);
    // free the original string
    free(latest_name);
    return path_name;
}

/*--------------------------------------------------------------
description: Find the latest directory using stat()
parameter: none
return value: the latest directory name
--------------------------------------------------------------*/
char* find_latest_dir(){
    DIR *dir = opendir(".");
    struct dirent* dir_entry;
    struct stat filestat;
    long int latest_time = 0;
    char* dir_name;
    char* latest_name;  // The name for latest directory to be returned
    // Read all the files from the current directory
    while(dir_entry = readdir(dir)) {
        // Only operate on directories with prefix: "peng.rooms"
        if(strncmp(dir_entry->d_name, "pengt.rooms.", 11) == 0){
            dir_name = malloc(sizeof(char) * 25);
            // Copy directory name to variable
            strcpy(dir_name, dir_entry->d_name);
            stat(dir_name, &filestat);
            // If this is the first directory checked, assign name
            if(latest_time == 0){
                latest_time = filestat.st_mtime;
                latest_name = malloc(sizeof(char) * 25);
                strcpy(latest_name, dir_name);
            }else{
                // If any directory is newer, reassign name
                if(latest_time < filestat.st_mtime){
                    latest_time = filestat.st_mtime;
                    free(latest_name);
                    latest_name = malloc(sizeof(char) * 25);
                    strcpy(latest_name, dir_name);
                }
            }
            free(dir_name);
        }
    }
    closedir(dir);
    return latest_name;
}

/*--------------------------------------------------------------
description: initiate the variables in the Room array
parameter: none
return value: return a room array of 7 rooms
--------------------------------------------------------------*/
struct Room* init_rooms(){
    int i;
    struct Room* rooms = malloc(sizeof(struct Room) * NUM_ROOMS);
    for(i = 0; i < NUM_ROOMS; i++) {
        rooms[i].num_connections = 0;
        rooms[i].cap_connections = 3;
        rooms[i].type = 2;
        // Set up the string pointers first to fetch lines from files
        rooms[i].string_connections = malloc(sizeof(char*) * rooms[i].cap_connections);
    }
    return rooms;
}

/*--------------------------------------------------------------
description: create the rooms arrays by getting values from files
parameter: string: path_name, Room*: the array of rooms
return value: none
--------------------------------------------------------------*/
void create_room_from_dir(char* path_name, struct Room* rooms){
    DIR* dir = opendir(path_name);
    struct dirent* room_entry;
    char* room_name;
    int room_count = 0;
    // Reading files from the directory
    while(room_entry = readdir(dir)) {
        // Reading any directories except "." and ".."
        if(strcmp(room_entry->d_name, ".") != 0 && strcmp(room_entry->d_name, "..") != 0){
            // Copy name to struct
            room_name = malloc(sizeof(char) * 15);
            strcpy(room_name, room_entry->d_name);
            rooms[room_count].name = room_name;
            // Putting info into rooms
            get_room_info(rooms, path_name, room_name, room_count);
            room_count++;
        }
    }
    closedir(dir);
    free(path_name);
    connect_rooms(rooms);
}

/*--------------------------------------------------------------
description: getting info from the room files
parameter: Room*: array of rooms, string: path name, string: room name,
            int: index within the room array
return value: none
--------------------------------------------------------------*/
void get_room_info(struct Room* rooms, char* path_name, char* room_name, int idx){
    int i, line_number = 0;
    char* file_path = malloc(sizeof(char) * 40);
    char* line = malloc(sizeof(char) * 50);
    // Joining strings to get the path name for room files
    sprintf(file_path, "%s/%s", path_name, room_name);
    FILE* file = fopen(file_path, "r");
    // reading each line of the file and pass into add_info_to_room function
    while(fgets(line, 50, file)) {
        add_info_to_room(rooms, line, idx);
    }
    free(file_path);
    free(line);
    fclose(file);
}

/*--------------------------------------------------------------
description: Add info into the room objects into string_connections
parameter: Room*: array of rooms, string: line from the files,
            int: index within the room array
return value: none
--------------------------------------------------------------*/
void add_info_to_room(struct Room* rooms, char* line, int idx){
    int i;
    // If the line start with "ROOM NAME", return since name is already added
    if(strncmp(line, "ROOM NAME", 9) == 0){
        return;
    }
    // If the lien start with "ROOM TYPE", only set roomtype to 1 and 3
    // Since it is initialized as 2
    else if(strncmp(line, "ROOM TYPE", 9) == 0){
        if(line[11] == 'S') rooms[idx].type = 1;
        if(line[11] == 'E') rooms[idx].type = 3;
    }
    // Add the connection to the string_connections array (string)
    else{
        // If the array is too small, expand by double
        if(rooms[idx].num_connections == rooms[idx].cap_connections){
            char** temp = malloc(sizeof(char*) * rooms[idx].cap_connections * 2);
            for(i = 0; i < rooms[idx].num_connections; i++){
                temp[i] = rooms[idx].string_connections[i];
            }
            free(rooms[idx].string_connections);
            rooms[idx].string_connections = temp;
            rooms[idx].cap_connections*=2;
        }
        // Add the string to the connection, string returned by extract_connections function
        rooms[idx].string_connections[rooms[idx].num_connections] = extract_connections(line);
        rooms[idx].num_connections++;
    }
}

/*--------------------------------------------------------------
description: Extract the room name from the connections
parameter: string: the line from files
return value: string: the name of the room connected
--------------------------------------------------------------*/
char* extract_connections(char* line){
    int i;
    char* room_connect = malloc(sizeof(char) * 15);
    // Extract all the room names after the substring "CONNECTION 1: "
    for(i = 14; i < strlen(line) - 1; i++){
        room_connect[i - 14] = line[i];
    }
    // Add Null terminated character
    room_connect[i - 14] = '\0';
    return room_connect;
}

/*--------------------------------------------------------------
description: connect the rooms for connections variable in Room struct
parameter: Room*: array of rooms
return value: none
--------------------------------------------------------------*/
void connect_rooms(struct Room* rooms){
    // Rewire all the rooms using connections variable since it is an array of
    // Room* that points to each room connected
    int i, j, k;
    for(i = 0; i < NUM_ROOMS; i++){
        rooms[i].connections = malloc(sizeof(struct Room*) * rooms[i].num_connections);
        for(j = 0; j < rooms[i].num_connections; j++){
            for(k = 0; k < NUM_ROOMS; k++){
                // If any room name is the same as the names in string_connections
                // Connect it in connections variable
                if(strcmp(rooms[i].string_connections[j], rooms[k].name) == 0){
                    rooms[i].connections[j] = &(rooms[k]);
                }
            }
        }
    }
}

/*--------------------------------------------------------------
description: The main function to start the adventure game
parameter: Room*: array of rooms
return value: none
--------------------------------------------------------------*/
void adventure(struct Room* rooms){
    int i, num_steps = 0, record_cap = 5;
    struct Room* curr_position;
    // An array of Room* to track all the steps gone through
    struct Room** step_record = malloc(sizeof(struct Room*) * record_cap);
    // Find the START_ROOM
    for(i = 0; i < NUM_ROOMS; i++){
        if(rooms[i].type == 1){
            curr_position = &(rooms[i]);
            break;
        }
    }
    // Keep looping until the user find the END_ROOM
    while(curr_position->type != 3){
        curr_position = run_step(rooms, curr_position);
        // Add the step, increment step count, only when the user makes a move
        if(curr_position != step_record[num_steps - 1]){
            if(num_steps == record_cap){
                step_record = expand_record(step_record, record_cap);
            }
            step_record[num_steps] = curr_position;
            num_steps++;
        }
    }
    // If the user reach END_ROOM, print result
    print_result(step_record, num_steps);
    free(step_record);
}

/*--------------------------------------------------------------
description: To expand the step_record array to store more user steps
parameter: Room**: Array for the step records, pointer of pointers pointing
            to each room passed by.
            int: capacity of the array
return value: Room**: new array with double the size
--------------------------------------------------------------*/
struct Room** expand_record(struct Room** step_record, int record_cap){
    int i;
    struct Room** temp = malloc(sizeof(struct Room*) * record_cap * 2);
    for(i = 0; i < record_cap; i++){
        temp[i] = step_record[i];
    }
    free(step_record);
    step_record = temp;
    record_cap *= 2;
    return step_record;
}

/*--------------------------------------------------------------
description: Run the steps of the game
parameter: Room*: array of rooms, Room*: pointer pointing to the current room
return value: return the curr_position, after moving (or not move at all)
--------------------------------------------------------------*/
struct Room* run_step(struct Room* rooms, struct Room* curr_position){
    int i, room_exist = 0;
    char* user_input = NULL;
    // variables defined to use getline()
    ssize_t read;
    size_t len = 0;
    // Printing current information
    printf("CURRENT LOCATION: %s\n", curr_position->name);
    printf("POSSIBLE CONNECTIONS: ");
    for(i = 0; i < curr_position->num_connections; i++){
        printf("%s", curr_position->connections[i]->name);
        if(i == curr_position->num_connections - 1){
            printf(".");
        }else{
            printf(", ");
        }
    }
    printf("\nWHERE TO? >");
    // Read the line fromt he user
    read = getline(&user_input, &len, stdin);
    user_input[strlen(user_input) - 1] = '\0';
    printf("\n");
    // If user input time, invoke print_time()
    if(strcmp(user_input, "time") == 0){
        print_time();
        free(user_input);
        return curr_position;
    }
    // Check rooms to go to, if found, move the curr_position variable
    for(i = 0; i < curr_position->num_connections; i++){
        if(strcmp(user_input, curr_position->connections[i]->name) == 0){
            room_exist = 1;
            curr_position = curr_position->connections[i];
            break;
        }
    }
    // Output error message for invalid input
    if(!room_exist) printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    free(user_input);
    return curr_position;
}

/*--------------------------------------------------------------
description: Print the winning message, and display info on
            how many steps taken, and the path taken
parameter: Room**: The step record, int: the number of steps taken
return value: none
--------------------------------------------------------------*/
void print_result(struct Room** step_record, int num_steps){
    int i;
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_steps);
    for(i = 0; i < num_steps; i++){
        printf("%s\n", step_record[i]->name);
    }
}

/*--------------------------------------------------------------
description: free all the dynamic memories of rooms variable
parameter: Room*: array of rooms
return value: none
--------------------------------------------------------------*/
void free_rooms(struct Room* rooms){
    int i, j;
    for(i = 0; i < NUM_ROOMS; i++){
        for(j = 0; j < rooms[i].num_connections; j++){
            free(rooms[i].string_connections[j]);
        }
        free(rooms[i].name);
        free(rooms[i].connections);
        free(rooms[i].string_connections);
    }
    free(rooms);
}
