#include <stdio.h>
#include "customer.h"
#include "main.h"
#include "reservations.h"

void customer_panel(void){
    
    printf("\nWelcome Dear Costumer\n");
    
    printf("1. Reserve a room\n"
           "2. View your reservations\n"
           "0. EXIT\n");
    
    int user_choice;
    
    do {
        printf("Your Choice : ");
        
        scanf("%d", &user_choice);
        
        switch (user_choice) {
            case 1:
                reserve_a_room();
                break;
            case 2:
                view_reservaions_user();
                //ask for id number or name or phone to check the reservations
                break;
            case 0: break;
            default: printf("Invalid Entry, Please Try Again!\n");
        }
    } while (user_choice != 1 && user_choice != 2 && user_choice != 0);
    
    printf("\n");
    main();
}
