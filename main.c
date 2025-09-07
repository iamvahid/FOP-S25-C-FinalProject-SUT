#include <stdio.h>
#include "admin.h"
#include "customer.h"
#include "permission.h"

int main(void){
    
    printf("Welcome!\n"
           "Login as:\n"
           "1. Customer\n"
           "2. Admin\n");
    
    short int user_choice;
    
    do {
        printf("Choice : ");
        
        scanf("%hi", &user_choice);
        
        if (user_choice == 1){
            customer_panel();
        }
        else if (user_choice == 2){
            while (1) {
                    if (password_check() == 1) {
                        printf("\nAccess Granted!\n");
                        admin_panel();
                        break;
                    } else {
                        printf("Access Denied! Please hit ENTER button to try again.\n");
                    }
                }
        }
        else {
            printf("Invalid Entry!\nTry Again!\n");
        }
        
    } while (user_choice != 1 && user_choice != 2);
    
    return 0;
}

