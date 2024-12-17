#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h>
 //constantlar 
#define MAX_ITEMS 5
#define MAX_DESC 256
#define MAX_ROOMS 4
#define MAX_CMD 50
#define MAX_SAVES 10 
// Structs for Game Entities
typedef struct Item {
    char name[MAX_DESC];
    char description[MAX_DESC];
    int health_bonus; // Oyuncunun saðlýðýný artýrýr
    int strength_bonus; // Oyuncunun gücünü artýrýr
} Item;

typedef struct Creature {
    char name[MAX_DESC];
    int health;
    int attack;
    char weakness[MAX_DESC];
} Creature;

typedef struct Room {
    char name[MAX_DESC];
    char description[MAX_DESC];
    Creature* creature;
    Item* item;
    struct Room* connections[4]; // [0] = north, [1] = east, [2] = south, [3] = west
} Room;

typedef struct Player {
    char name[MAX_DESC];
    int health;
    int strength;
    Item inventory[MAX_ITEMS];
    int inventory_count;
    Room* current_room;
} Player;

// Directions enum for readability
typedef enum { NORTH = 0, EAST, SOUTH, WEST } Directions;


// Game saving and loading
typedef struct Save {
    char filepath[MAX_DESC]; // Kaydedilen oyunun dosya yolu
} Save;


// Function prototypes
void show_menu();
void initialize_game(Player* player, Room rooms[], Save saves[], int* save_count);
void describe_room(Room* room);
void handle_command(Player* player, Room rooms[], Save saves[], int* save_count, char* command); void move_player(Player* player, int direction);
void look(Player* player);
void pickup_item(Player* player);
void show_inventory(Player* player);
void attack_creature(Player* player);
void save_game(Player* player, Room rooms[], const char* filepath);
void load_game(Player* player, Room rooms[], const char* filepath);
void free_resources(Room rooms[], int num_rooms);
void list_saved_games(Save saves[], int save_count);
void apply_item_effect(Player* player, Item* item);
void move_player(Player* player, int direction);
// Main Function
int main() {
    Player player;
    Room rooms[MAX_ROOMS];
    Save saves[MAX_SAVES];
    int save_count = 0;
    char command[MAX_CMD];

    int menu_choice;
    bool game_running = true;


    while (game_running) {
        show_menu();
        if (scanf_s("%d", &menu_choice) != 1) {
            printf("Input error!\n");
            continue; // Hatalý giriþte döngüyü devam ettir
        }
        getchar(); // Yeni satýr karakterini al

        switch (menu_choice) {
        case 1:
            initialize_game(&player, rooms, saves, &save_count);
            printf("WELCOME TO THE DUNGEON GAME!\n");
            describe_room(player.current_room);

            while (player.health > 0) {
                printf("\n> ");
                fgets(command, MAX_CMD, stdin);
                command[strcspn(command, "\n")] = 0; // Yeni satýrý sil

                if (strcmp(command, "exit") == 0) {
                    printf("Thanks for playing!\n");
                    game_running = false;
                    break;
                }
                handle_command(&player, rooms, saves, &save_count, command);
            }

            free_resources(rooms, MAX_ROOMS);
            break;

        case 2:
            load_game(&player, rooms, "savefile.txt");
            printf("Game loaded successfully!\n");
            describe_room(player.current_room);
            break;

        case 3:
            printf("Instructions:\n");
            printf("- Use 'move <direction>' to move between rooms (north, south, east, west).\n");
            printf("- Use 'look' to look around the room.\n");
            printf("- Use 'pickup' to pick up items in the room.\n");
            printf("- Use 'inventory' to view your inventory.\n");
            printf("- Use 'attack' to fight creatures.\n");
            printf("- Use 'save' to save your progress.\n");
            printf("- Use 'exit' to quit the game.\n");
            break;

        case 4:
            printf("Exiting the game. Goodbye!\n");
            game_running = false;
            break;

        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    }
    return 0;
}

// Display the game menu
void show_menu() {
    printf("\n==== Dungeon Adventure Menu ==== \n");
    printf("1. Start New Game\n");
    printf("2. Load Game\n");
    printf("3. Help\n");
    printf("4. Exit\n");
    printf("Choose an option: ");
}

// initialize the game setup
// Zindanda yer alacak her oda için özellikler
void initialize_game(Player* player, Room rooms[], Save saves[], int* save_count) {
    // Player setup
    strcpy_s(player->name, "Hero");
    player->health = 100;
    player->strength = 10;
    player->inventory_count = 0;

    // Ateþ Odasý
    strcpy_s(rooms[0].name, "Fire Room");
    strcpy_s(rooms[0].description, "A blazing room with walls of fire. The heat is intense.");
    rooms[0].creature = (Creature*)malloc(sizeof(Creature));
    strcpy_s(rooms[0].creature->name, "Fire Elemental");
    rooms[0].creature->health = 80;
    rooms[0].creature->attack = 20;
    strcpy_s(rooms[0].creature->weakness, "Water");
    rooms[0].item = NULL;

    // Hava Odasý
    strcpy_s(rooms[1].name, "Air Room");
    strcpy_s(rooms[1].description, "The air is swirling around you with intense winds.");
    rooms[1].creature = (Creature*)malloc(sizeof(Creature));
    strcpy_s(rooms[1].creature->name, "Air Elemental");
    rooms[1].creature->health = 70;
    rooms[1].creature->attack = 15;
    strcpy_s(rooms[1].creature->weakness, "Earth");
    rooms[1].item = NULL;

    // Su Odasý
    strcpy_s(rooms[2].name, "Water Room");
    strcpy_s(rooms[2].description, "A cool room with water flowing from the walls.");
    rooms[2].creature = (Creature*)malloc(sizeof(Creature));
    strcpy_s(rooms[2].creature->name, "Water Elemental");
    rooms[2].creature->health = 90;
    rooms[2].creature->attack = 18;
    strcpy_s(rooms[2].creature->weakness, "Fire");
    rooms[2].item = NULL;

    // Toprak Odasý
    strcpy_s(rooms[3].name, "Earth Room");
    strcpy_s(rooms[3].description, "The room feels heavy with the scent of soil and stone.");
    rooms[3].creature = (Creature*)malloc(sizeof(Creature));
    strcpy_s(rooms[3].creature->name, "Earth Elemental");
    rooms[3].creature->health = 100;
    rooms[3].creature->attack = 25;
    strcpy_s(rooms[3].creature->weakness, "Air");
    rooms[3].item = NULL;

    // Odalarýn Baðlantýlarý
    rooms[0].connections[NORTH] = &rooms[1]; // Ateþ Odasý -> Hava Odasý
    rooms[1].connections[NORTH] = &rooms[2]; // Hava Odasý -> Su Odasý
    rooms[2].connections[NORTH] = &rooms[3]; // Su Odasý -> Toprak Odasý
    rooms[3].connections[NORTH] = NULL;      // Toprak Odasý'ndan çýkýþ yok, oyun bitiyor

    // Baþlangýç odasý
    player->current_room = &rooms[0];
}


// Free dynamically allocated resources
void free_resources(Room rooms[], int num_rooms) {
    for (int i = 0; i < num_rooms; i++) {
        if (rooms[i].creature) {
            free(rooms[i].creature);
            rooms[i].creature = NULL;
        }
        if (rooms[i].item) {
            free(rooms[i].item);
            rooms[i].item = NULL;
        }
    }
}

// Show available saved games
void list_saved_games(Save saves[], int save_count) {
    printf("Saved games:\n");
    for (int i = 0; i < save_count; i++) {
        printf("%d. %s\n", i + 1, saves[i].filepath);
    }
}
// Describe the current room
void describe_room(Room* room) {
    printf("\nYou are in %s. %s\n", room->name, room->description);
    if (room->creature) {
        printf("A %s is lurking here with %d health!\n", room->creature->name, room->creature->health);
    }
    else {
        printf("The room is clear, no enemies here!\n");
    }
}

// Handle player commands
void handle_command(Player* player, Room rooms[], Save saves[], int* save_count, char* command) {
    if (strncmp(command, "move", 4) == 0) {
        if (strstr(command, "north")) move_player(player, NORTH);
        else if (strstr(command, "south")) move_player(player, SOUTH);
        else if (strstr(command, "east")) move_player(player, EAST);
        else if (strstr(command, "west")) move_player(player, WEST);
        else printf("Invalid direction!\n");
    }
    else if (strcmp(command, "look") == 0) {
        look(player);
    }
    else if (strcmp(command, "inventory") == 0) {
        show_inventory(player);
    }
    else if (strncmp(command, "pickup", 6) == 0) {
        pickup_item(player);
    }
    else if (strcmp(command, "attack") == 0) {
        attack_creature(player);
    }
    else if (strncmp(command, "save", 4) == 0) {
        save_game(player, rooms, "savefile.txt");
    }
    else if (strncmp(command, "load", 4) == 0) {
        load_game(player, rooms, "savefile.txt");
    }
    else if (strcmp(command, "list") == 0) {
        list_saved_games(saves, *save_count);
    }
    else {
        printf("Unknown command!\n");
    }
}

// Command implementations
void move_player(Player* player, int direction) {
    if (player->current_room->connections[direction]) {
        player->current_room = player->current_room->connections[direction];
        describe_room(player->current_room);
    }
    else {
        printf("You can't move in that direction!\n");
    }
}

void look(Player* player) {
    describe_room(player->current_room);
}

void pickup_item(Player* player) {
    if (player->current_room->item) {
        if (player->inventory_count < MAX_ITEMS) {
            player->inventory[player->inventory_count++] = *player->current_room->item;
            printf("You picked up %s!\n", player->current_room->item->name);
            apply_item_effect(player, player->current_room->item);
            free(player->current_room->item);
            player->current_room->item = NULL;
        }
        else {
            printf("Your inventory is full!\n");
        }
    }
    else {
        printf("There are no items here.\n");
    }
}

void show_inventory(Player* player) {
    printf("Your inventory:\n");
    for (int i = 0; i < player->inventory_count; i++) {
        printf("%d. %s\n", i + 1, player->inventory[i].name);
    }
}

void apply_item_effect(Player* player, Item* item) {
    player->health += item->health_bonus;
    player->strength += item->strength_bonus;
    printf("Item effects applied: Health +%d, Strength +%d\n", item->health_bonus, item->strength_bonus);
}

void attack_creature(Player* player) {
    Creature* creature = player->current_room->creature;
    if (creature) {
        // Yaratýðý saldýrarak zayýflatýyoruz
        creature->health -= player->strength;
        printf("You attacked the %s! It has %d health left.\n", creature->name, creature->health);

        // Eðer yaratýk öldüyse
        if (creature->health <= 0) {
            printf("You defeated the %s!\n", creature->name);
            free(creature);
            player->current_room->creature = NULL;

            // Eðer odadaki yaratýk öldü ise bir sonraki odaya geç
            if (player->current_room->connections[NORTH]) {
                player->current_room = player->current_room->connections[NORTH];
                describe_room(player->current_room);
            }
            else {
                printf("You've defeated the Earth Elemental! You've completed the dungeon!\n");
                player->health = 0; // Oyunun bitmesi için oyuncuyu öldürüyoruz
            }
        }
        else {
            // Yaratýðýn saldýrýsý
            player->health -= creature->attack;
            printf("The %s attacked you! You have %d health left.\n", creature->name, player->health);
        }
    }
    else {
        printf("There's nothing to attack here!\n");
    }
}

// Save game state to a file
void save_game(Player* player, Room rooms[], const char* filepath) {
    FILE* file;
    errno_t err = fopen_s(&file, filepath, "w"); // fopen_s kullanýmý
    if (err != 0 || file == NULL) {
        printf("Failed to save game!\n");
        return;
    }

    // Oyuncu bilgilerini dosyaya yaz
    fprintf(file, "%s %d %d %d\n", player->name, player->health, player->strength, player->inventory_count);

    // Envanter bilgilerini dosyaya yaz
    for (int i = 0; i < player->inventory_count; i++) {
        fprintf(file, "%s %s\n", player->inventory[i].name, player->inventory[i].description);
    }

    fclose(file);
    printf("Game saved!\n");
}

// Load game state from a file
void load_game(Player* player, Room rooms[], const char* filepath) {
    FILE* file;
    errno_t err = fopen_s(&file, filepath, "r"); // fopen_s kullanýmý
    if (err != 0 || file == NULL) {
        printf("Failed to load game!\n");
        return;
    }

    // Oyuncu bilgilerini dosyadan oku
    fscanf_s(file, "%s %d %d %d\n", player->name, (unsigned int)sizeof(player->name),
        &player->health, &player->strength, &player->inventory_count);

    // Envanter bilgilerini dosyadan oku
    for (int i = 0; i < player->inventory_count; i++) {
        fscanf_s(file, "%s %s\n", player->inventory[i].name, (unsigned int)sizeof(player->inventory[i].name),
            player->inventory[i].description, (unsigned int)sizeof(player->inventory[i].description));
    }

    fclose(file);
    printf("Game loaded!\n");
}
