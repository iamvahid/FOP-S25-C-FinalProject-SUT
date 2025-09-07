#include <stdio.h>
#include "permission.h"
#include <string.h>
#include "admin.h"

void clear_input_buffer(void) {
    // clearing the input buffer by reading and discarding characters until a newline is encountered.
    // if admin enters a wrong password, program will require the user to hit enter button once.
    // clearing buffer is mandatory to clear the buffer if one wants to use fgets and include white spaces in their password.
    // else the scanf would be adequate to get only characters as password and skip the clearing buffer process.
    while (getchar() != '\n' && getchar() != EOF);
}

int password_check(void){
    // reading the saved password in the admin_pass.txt
    char saved_password[33];
    char filename[] = "/Users/v/Desktop/hotel/hotel/data/password.txt";
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("File path: %s\n", filename);
        printf("Password File Not Found!\n");
        return 0;
    }
    
    fgets(saved_password, sizeof(saved_password), file);
    saved_password[strcspn(saved_password, "\n")] = '\0';
    
    clear_input_buffer();
    
    fclose(file);
    
    //printf("P : %s\n", saved_password);
    
    // asking the admin to white their password in mind
    char entry_password[33];
    
    printf("Enter Password : ");
    fflush(stdout);
    fgets(entry_password, sizeof(entry_password), stdin); //also includes white_spaces but add \n and remove it manually
    entry_password[strcspn(entry_password, "\n")] = '\0';
//    scanf("%s", entry_password);

    // checking if the two passwords are identical
    if (strcmp(saved_password, entry_password) == 0) return 1;
    else return 0;
}

void change_password(void){
    
    char filename[] = "/Users/v/Desktop/hotel/hotel/data/password.txt";
    FILE *file = fopen(filename, "w+");
    
    if (file == NULL) {
        printf("Password File Not Found!\n");
        return;
    }
    
    clear_input_buffer();

    printf("Please Enter New Password\n"
           "Remember that your password must contain at least 8 and at most 32 characters : ");
    char new_password[33];
    fgets(new_password, sizeof(new_password), stdin);
    
    size_t length = strlen(new_password);
    // removing \n new line operator which fgets will add automatically
    if (new_password[length - 1] == '\n'){
        new_password[length - 1] = '\0';
    }

    fputs(new_password, file);
    printf("You have Successfully changed your password!\n");
    
    fclose(file);
    
    admin_panel();
    
}
