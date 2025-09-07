#ifndef ROOMS_H
#define ROOMS_H

typedef struct {
    int room_number;
    char room_type[16];
    int price;
    int availability;
} ROOMS;

void all_room_list(void);
void available_room_list(void);

#endif //ROOMS_H
