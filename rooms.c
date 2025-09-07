#include <stdio.h>
#include <string.h>
#include "rooms.h"

void all_room_list(void){
    
    FILE *file = fopen("/Users/v/Desktop/hotel/hotel/data/rooms.txt", "r");
    
    if (file == NULL) {
        printf("File Not Found!\n");
        return;
    }
    ROOMS room[23];
    
    int room_counts = 0;
    
    while (fscanf(file,
                  "%d %s %d %d",
                  &room[room_counts].room_number,
                  room[room_counts].room_type,
                  &room[room_counts].price,
                  &room[room_counts].availability) != EOF) {
        room_counts++;
    }
    
    fclose(file);
    
    printf("Rooms:\n");
    printf("Room Number  | Room Type    | Price        | Availability\n");
    printf("---------------------------------------------------------\n");
    
    for (int i = 0; i < room_counts; i++) {
        printf("%12d | %12s | %12d | %12d\n",
               room[i].room_number,
               room[i].room_type,
               room[i].price,
               room[i].availability);
    }

}

void available_room_list(void){
    
    FILE *file = fopen("/Users/v/Desktop/hotel/hotel/data/rooms.txt", "r");
    
    if (file == NULL) {
        printf("File Not Found!\n");
        return;
    }
    ROOMS room[23];
    
    int room_counts = 0;
    
    while (fscanf(file,
                  "%d %s %d %d",
                  &room[room_counts].room_number,
                  room[room_counts].room_type,
                  &room[room_counts].price,
                  &room[room_counts].availability) != EOF) {
        room_counts++;
    }
    // condition to filter unavailable room
    fclose(file);
    
    printf("Rooms:\n");
    printf("Room Number  | Room Type    | Price        \n");
    printf("-------------------------------------------\n");
    
    for (int i = 0; i < room_counts; i++) {
        if (room[i].availability == 0) continue;
        printf("%12d | %12s | %12d\n",
               room[i].room_number,
               room[i].room_type,
               room[i].price);
    }

}
