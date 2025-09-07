#include <stdio.h>
#include "admin.h"
#include "permission.h"
#include "main.h"
#include "reservations.h"
#include "rooms.h"

void clear_screen(void){
    printf("\033[2J\033[H");
    fflush(stdout);
}

void admin_panel(void){
    
    // clear_screen();
    
    printf("Welcome Dear Manager!\n"
           "Menu :\n"
           "1.View Reservations\n"
           "2.Change Password\n"
           "0.EXIT\n");
    
    short int admin_choice;

    do {
        printf("\nYour Choice : ");
        
        scanf("%hi", &admin_choice);
        
        switch (admin_choice) {
            case 1:
                reservaions_management();
                break;
            case 2:
                change_password();
                break;
            case 0: break;
            default: printf("Invalid Entry, Please Try Again!\n");
        }
    } while (admin_choice != 1 && admin_choice != 2 && admin_choice != 0);
    
    printf("\n");
    main();
}
//add change password : DONE
