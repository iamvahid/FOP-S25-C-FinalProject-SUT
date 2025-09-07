#ifndef RESERVATIONS_H
#define RESERVATIONS_H

typedef struct {
    long int customer_id;
    char customer_name[100];
    long int customer_number;
    int room_no;
} CUSTOMER;

void reserve_a_room(void);
void view_reservaions_user(void);
void reservaions_management(void);

#endif //RESERVATIONS_H
