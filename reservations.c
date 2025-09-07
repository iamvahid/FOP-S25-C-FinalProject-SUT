#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "reservations.h"
#include "permission.h"
#include "rooms.h"

// ------------------------------
// Helpers
// ------------------------------
static void sanitize_name(char *dst, const char *src) {
    // Replace spaces with underscores for safe single-token storage
    size_t j = 0;
    for (size_t i = 0; src[i] && j < 63; ++i) {
        dst[j++] = (src[i] == ' ' || src[i] == '\t' || src[i] == '\n') ? '_' : src[i];
    }
    dst[j] = '\0';
}
static void desanitize_name(char *s) {
    for (size_t i = 0; s[i]; ++i) if (s[i] == '_') s[i] = ' ';
}

static int parse_date(const char *s, struct tm *out) {
    // Expected format: YYYY-MM-DD
    if (!s || strlen(s) != 10) return 0;
    if (!(isdigit((unsigned char)s[0]) && isdigit((unsigned char)s[1]) && isdigit((unsigned char)s[2]) && isdigit((unsigned char)s[3]) &&
          s[4]=='-' &&
          isdigit((unsigned char)s[5]) && isdigit((unsigned char)s[6]) &&
          s[7]=='-' &&
          isdigit((unsigned char)s[8]) && isdigit((unsigned char)s[9]))) return 0;

    int y = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
    int m = (s[5]-'0')*10 + (s[6]-'0');
    int d = (s[8]-'0')*10 + (s[9]-'0');
    if (m < 1 || m > 12 || d < 1 || d > 31) return 0;

    memset(out, 0, sizeof(*out));
    out->tm_year = y - 1900;
    out->tm_mon  = m - 1;
    out->tm_mday = d;
    out->tm_isdst = -1;
    if (mktime(out) == (time_t)-1) return 0;
    return 1;
}
static int same_day(struct tm a, struct tm b) {
    return a.tm_year==b.tm_year && a.tm_mon==b.tm_mon && a.tm_mday==b.tm_mday;
}
static time_t day_start_epoch(struct tm tmin) {
    struct tm x = tmin;
    x.tm_hour = 0; x.tm_min = 0; x.tm_sec = 0;
    return mktime(&x);
}
static int ranges_overlap(struct tm start1, int nights1, struct tm start2, int nights2) {
    time_t a0 = day_start_epoch(start1);
    time_t a1 = a0 + (time_t)nights1 * 24*3600;
    time_t b0 = day_start_epoch(start2);
    time_t b1 = b0 + (time_t)nights2 * 24*3600;
    return (a0 < b1) && (b0 < a1);
}


// id name_no_spaces phone room_no checkin_YYYY-MM-DD nights price_per_night total
static int read_booking_line(const char *line,
                             long *bid, char *name, long *phone, int *rno,
                             char *date, int *bnights, int *price, long *total) {
    // name is 1 token (underscores), date is 1 token (YYYY-MM-DD)
    return sscanf(line, "%ld %63s %ld %d %15s %d %d %ld",
                  bid, name, phone, rno, date, bnights, price, total) == 8;
}

static int price_for_room(int room_no) {
    FILE *file = fopen("/Users/v/Desktop/hotel/hotel/data/rooms.txt", "r");
    if (!file) return 0;
    ROOMS room[64]; int n=0;
    while (n < 64 && fscanf(file, "%d %49s %d %d",
                            &room[n].room_number,
                            room[n].room_type,
                            &room[n].price,
                            &room[n].availability) == 4) {
        n++;
    }
    fclose(file);
    for (int i=0;i<n;i++) if (room[i].room_number==room_no) return room[i].price;
    return 0;
}

static int is_room_available_for_range(int room_no, struct tm start, int nights) {
    FILE *bk = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
    if (!bk) return 1; // no bookings yet => available

    char line[256];
    long bid;
    char name[64];
    long phone;
    int rno;
    char date[16];
    int bnights;
    int price_per_night; long total;
    while (fgets(line, sizeof line, bk)) {
        if (!read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price_per_night, &total)) continue;
        if (rno != room_no) continue;
        struct tm other_start;
        if (!parse_date(date, &other_start)) continue;
        if (ranges_overlap(start, nights, other_start, bnights)) {
            fclose(bk);
            return 0;
        }
    }
    fclose(bk);
    return 1;
}

static void available_room_list_by_date(struct tm start, int nights) {
    FILE *file = fopen("/Users/v/Desktop/hotel/hotel/data/rooms.txt", "r");
    if (file == NULL) {
        printf("File Not Found!\n");
        return;
    }
    ROOMS room[64];
    int room_counts = 0;
    while (room_counts < 64 && fscanf(file,
                  "%d %49s %d %d",
                  &room[room_counts].room_number,
                  room[room_counts].room_type,
                  &room[room_counts].price,
                  &room[room_counts].availability) == 4) {
        room_counts++;
    }
    fclose(file);

    printf("Rooms available for the selected dates:\n");
    printf("Room Number  | Room Type    | Price\n");
    printf("------------------------------------\n");
    for (int i = 0; i < room_counts; i++) {
        if (room[i].availability == 0) continue; // respect static flag
        if (!is_room_available_for_range(room[i].room_number, start, nights)) continue;
        printf("%12d | %12s | %5d\n", room[i].room_number, room[i].room_type, room[i].price);
    }
}


int check_availability(void){
    FILE *room_file = fopen("/Users/v/Desktop/hotel/hotel/data/customers.txt", "r");
    if (room_file == NULL) {
        printf("File Not Found!\n");
        return 0;
    }
    ROOMS room[23];
    int room_counts = 0;
    while (fscanf(room_file,
                  "%d %s %d %d",
                  &room[room_counts].room_number,
                  room[room_counts].room_type,
                  &room[room_counts].price,
                  &room[room_counts].availability) != EOF) {
        room_counts++;
    }
    int user_input;
    int room_available = 0;
    do {
        printf("Enter the room number you want to reserve: ");
        scanf("%d", &user_input);
        for (int i = 0; i < room_counts; i++) {
            if (user_input == room[i].room_number) {
                if (room[i].availability == 0) {
                    printf("Room is FULL, please choose another room!\n");
                    room_available = 0;
                    break;
                } else {
                    printf("Room %d is available. Proceed with reservasions.\n", user_input);
                    room_available = 1;
                    break;
                }
            }
        }
    } while (!room_available);
    fclose(room_file);
    return user_input;
}



void reserve_a_room(void){
    CUSTOMER guest;
    srand((unsigned int)time(NULL));
    guest.customer_id = rand() % 90000 + 10000;

    printf("\nWhat's your name? : ");
    clear_input_buffer();
    fgets(guest.customer_name, sizeof(guest.customer_name), stdin);
    guest.customer_name[strcspn(guest.customer_name, "\n")] = '\0';

    printf("Please provide a contact number : ");
    scanf("%ld", &guest.customer_number);

    // Dates
    char checkin_str[16];
    int nights = 1;
    printf("Enter check-in date (YYYY-MM-DD): ");
    scanf("%15s", checkin_str);
    struct tm checkin;
    while (!parse_date(checkin_str, &checkin)) {
        printf("Invalid date. Please enter again (YYYY-MM-DD): ");
        scanf("%15s", checkin_str);
    }
    printf("Number of nights: ");
    scanf("%d", &nights);
    if (nights < 1) nights = 1;

    printf("Choose a room from the list below!\n");
    available_room_list_by_date(checkin, nights);

    printf("Room Number : ");
    scanf("%d", &guest.room_no);

    // Validate chosen room for the selected dates
    if (!is_room_available_for_range(guest.room_no, checkin, nights)) {
        printf("Sorry, that room is not available for the selected dates.\n");
        return;
    }

    printf("\nRESERVATION DONE SUCCESSFULLY!\n\n");
    printf("Your reservation number : %ld\n", guest.customer_id);
    printf("Customer Name : %s\n", guest.customer_name);
    printf("Customer Number : %ld\n", guest.customer_number);
    printf("Room Numer : %d\n", guest.room_no);
    printf("Check-in : %s | Nights: %d\n", checkin_str, nights);

    // Legacy customer log
    FILE *customer_file = fopen("/Users/v/Desktop/hotel/hotel/data/customers.txt", "a");
    if (customer_file != NULL) {
        fprintf(customer_file, "%ld %s %ld %d\n",
                guest.customer_id,
                guest.customer_name,
                guest.customer_number,
                guest.room_no);
        fclose(customer_file);
    }

    // NEW: bookings.txt with sanitized name
    int price = price_for_room(guest.room_no);
    long total = (long)price * (long)nights;

    char name_sanitized[64];
    sanitize_name(name_sanitized, guest.customer_name);

    FILE *bk = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "a");
    if (bk) {
        fprintf(bk, "%ld %s %ld %d %s %d %d %ld\n",
                guest.customer_id,
                name_sanitized,
                guest.customer_number,
                guest.room_no,
                checkin_str,
                nights,
                price,
                total);
        fclose(bk);
    } else {
        printf("Warning: unable to write bookings.txt\n");
    }
}

void view_reservaions_user(void){
    printf("Please Enter Your Reservation Number : ");
    long int reservation_number;
    scanf("%ld", &reservation_number);

    int found = 0;

    FILE *bk = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
    if (bk) {
        char line[256];
        long bid; char name[64]; long phone; int rno; char date[16]; int bnights;
        int price; long total;

        while (fgets(line, sizeof line, bk)) {
            if (!read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &total)) continue;
            if (bid == reservation_number) {
                desanitize_name(name);
                printf("\nReservation Found!\n");
                printf("Name: %s\n", name);
                printf("number: %ld\n", phone);
                printf("Room Number: %d\n", rno);
                printf("Check-in: %s | Nights: %d\n", date, bnights);
                found = 1;
                break;
            }
        }
        fclose(bk);
    }

    if (!found) {
        printf("Reservation Number Not Found.\n");
        return;
    }

    clear_input_buffer();
    char choice;
    printf("Do you wish to keep your reservation? (y/n): ");
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {
        printf("You decided to keep your reservations.\n");
        return;
    }

    // Cancel by rewriting bookings.txt
    FILE *in = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
    FILE *out = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.tmp", "w");
    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        printf("Error processing cancellation.\n");
        return;
    }

    char line[256];
    long bid; char name[64]; long phone; int rno; char date[16]; int bnights; int price; long total;
    int removed = 0;

    while (fgets(line, sizeof line, in)) {
        if (read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &total) && bid == reservation_number) {
            removed = 1; // skip this line
            continue;
        }
        fputs(line, out); // keep original formatting
    }
    fclose(in);
    fclose(out);

    if (removed) {
        remove("/Users/v/Desktop/hotel/hotel/data/bookings.txt");
        if (rename("/Users/v/Desktop/hotel/hotel/data/bookings.tmp",
                   "/Users/v/Desktop/hotel/hotel/data/bookings.txt") != 0) {
            perror("rename");
            printf("Warning: temp rename failed. You may need to delete the .tmp manually.\n");
        } else {
            printf("Your reservation has been cancelled.\n");
        }
    } else {
        remove("/Users/v/Desktop/hotel/hotel/data/bookings.tmp");
        printf("Could not cancel reservation (not found).\n");
    }
}


// Admin tools called from admin panel DONE!
static void list_all_reservations(void) {
    FILE *bk = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
    if (!bk) { printf("No reservations found.\n"); return; }

    printf("All Reservations:\n");
    printf("ID     Name                 Phone        Room  Check-in   Nights  Price  Total\n");
    printf("-----------------------------------------------------------------------------\n");

    char line[256];
    long bid; char name[64]; long phone; int rno; char date[16]; int bnights; int price; long total;
    int any = 0;

    while (fgets(line, sizeof line, bk)) {
        if (!read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &total)) continue;
        desanitize_name(name);
        printf("%-6ld %-20s %-12ld %-5d %-10s %-6d %-6d %-6ld\n",
               bid, name, phone, rno, date, bnights, price, total);
        any = 1;
    }
    if (!any) printf("(empty)\n");
    fclose(bk);
}

static void search_reservation(void) {
    printf("Search by: 1) Reservation ID  2) Customer name\nChoice: ");
    int c=0; scanf("%d", &c);
    if (c==1) {
        long q; printf("Enter reservation ID: "); scanf("%ld", &q);
        FILE *bk = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
        if (!bk) { printf("No reservations.\n"); return; }
        char line[256];
        long bid; char name[64]; long phone; int rno; char date[16]; int bnights; int price; long total;
        int found=0;
        while (fgets(line, sizeof line, bk)) {
            if (!read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &total)) continue;
            if (bid==q) {
                desanitize_name(name);
                printf("Found: %ld %s %ld Room:%d %s x%d Total:%ld\n",
                       bid, name, phone, rno, date, bnights, total);
                found=1; break;
            }
        }
        if (!found) printf("Not found.\n");
        fclose(bk);
    } else if (c==2) {
        char q[64]; printf("Enter customer name (exact): "); scanf("%63s", q);
        // also accept input with spaces replaced by underscores
        char q_sanitized[64]; sanitize_name(q_sanitized, q);

        FILE *bk = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
        if (!bk) { printf("No reservations.\n"); return; }
        char line[256];
        long bid; char name[64]; long phone; int rno; char date[16]; int bnights; int price; long total;
        int any=0;
        while (fgets(line, sizeof line, bk)) {
            if (!read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &total)) continue;
            if (strcmp(name, q_sanitized)==0) {
                desanitize_name(name);
                printf("%ld %s %ld Room:%d %s x%d Total:%ld\n", bid, name, phone, rno, date, bnights, total);
                any=1;
            }
        }
        if (!any) printf("No matches.\n");
        fclose(bk);
    } else {
        printf("Invalid choice.\n");
    }
}

static void cancel_reservation_admin(void) {
    long id; printf("Enter reservation ID to cancel: "); scanf("%ld", &id);

    FILE *in = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
    FILE *out = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.tmp", "w");
    if (!in || !out) { if (in) fclose(in); if (out) fclose(out); printf("No reservations.\n"); return; }

    char line[256];
    long bid; char name[64]; long phone; int rno; char date[16]; int bnights; int price; long total;
    int removed=0;

    while (fgets(line, sizeof line, in)) {
        if (read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &total) && bid==id) {
            removed=1; continue; // skip this booking
        }
        fputs(line, out);
    }
    fclose(in); fclose(out);

    if (removed) {
        remove("/Users/v/Desktop/hotel/hotel/data/bookings.txt");
        if (rename("/Users/v/Desktop/hotel/hotel/data/bookings.tmp",
                   "/Users/v/Desktop/hotel/hotel/data/bookings.txt") != 0) {
            perror("rename");
            printf("Warning: temp rename failed. You may need to delete the .tmp manually.\n");
        } else {
            printf("Reservation cancelled.\n");
        }
    } else {
        remove("/Users/v/Desktop/hotel/hotel/data/bookings.tmp");
        printf("Reservation not found.\n");
    }
}

static void revenue_report(void) {
    printf("Revenue report: 1) Day  2) Last 7 days (rolling)\nChoice: ");
    int c=0; scanf("%d", &c);

    FILE *bk = fopen("/Users/v/Desktop/hotel/hotel/data/bookings.txt", "r");
    if (!bk) { printf("No bookings.\n"); return; }

    long total=0;
    char line[256];
    long bid; char name[64]; long phone; int rno; char date[16]; int bnights; int price; long t;

    if (c==1) {
        char day_str[16]; printf("Enter date (YYYY-MM-DD): "); scanf("%15s", day_str);
        struct tm d; if (!parse_date(day_str, &d)) { printf("Invalid date.\n"); fclose(bk); return; }

        while (fgets(line, sizeof line, bk)) {
            if (!read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &t)) continue;
            struct tm s; if (!parse_date(date, &s)) continue;
            for (int i=0;i<bnights;i++) {
                time_t e = day_start_epoch(s) + i*24*3600;
                struct tm night = *localtime(&e);
                if (same_day(night, d)) total += price;
            }
        }
        printf("Revenue for %s: %ld\n", day_str, total);
    } else if (c==2) {
        time_t now = time(NULL);
        struct tm today = *localtime(&now);
        time_t today0 = day_start_epoch(today);

        while (fgets(line, sizeof line, bk)) {
            if (!read_booking_line(line, &bid, name, &phone, &rno, date, &bnights, &price, &t)) continue;
            struct tm s; if (!parse_date(date, &s)) continue;
            for (int i=0;i<bnights;i++) {
                time_t e = day_start_epoch(s) + i*24*3600;
                if (e >= today0 - 6*24*3600 && e <= today0) total += price;
            }
        }
        printf("Revenue for last 7 days (including today): %ld\n", total);
    } else {
        printf("Invalid choice.\n");
    }
    fclose(bk);
}

void reservaions_management(void){
    printf("Your Reservations\n");
    int choice=0;
    do {
        printf("\nAdmin Reservations Menu:\n");
        printf("1. List all\n");
        printf("2. Search\n");
        printf("3. Cancel a reservation\n");
        printf("4. Revenue report\n");
        printf("0. Back\n");
        printf("Choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: list_all_reservations(); break;
            case 2: search_reservation(); break;
            case 3: cancel_reservation_admin(); break;
            case 4: revenue_report(); break;
            case 0: default: break;
        }
    } while (choice != 0);
}
