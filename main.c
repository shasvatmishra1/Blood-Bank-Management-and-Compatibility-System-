#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_NAME 50

typedef struct {
    char name[MAX_NAME];
    char blood_type[4];
    char last_donation_date[11];
    int available_units;
} Donor;

typedef struct {
    char name[MAX_NAME];
    char blood_type[4];
    int required_units;
    int urgent;
    int fulfilled;
    char date[11];
    char time[6];
} Recipient;

const char *groups[] = {
    "A+", "A-", "B+", "B-",
    "AB+", "AB-", "O+", "O-"
};

int validGroup(const char *group) {
    for (int i = 0; i < 8; i++) {
        if (strcmp(group, groups[i]) == 0)
            return 1;
    }
    return 0;
}

void getDateTime(char date[], char timeText[]) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (t != NULL) {
        strftime(date, 11, "%Y-%m-%d", t);
        strftime(timeText, 6, "%H:%M", t);
    } else {
        strcpy(date, "Unknown");
        strcpy(timeText, "--:--");
    }
}

/* Simplified red-blood-cell ABO/Rh compatibility */
int compatible(const char *donor, const char *recipient) {
    if (strcmp(donor, "O-") == 0)
        return 1;

    if (strcmp(donor, "O+") == 0)
        return recipient[strlen(recipient) - 1] == '+';

    if (strcmp(donor, "A-") == 0)
        return strcmp(recipient, "A-") == 0 ||
               strcmp(recipient, "A+") == 0 ||
               strcmp(recipient, "AB-") == 0 ||
               strcmp(recipient, "AB+") == 0;

    if (strcmp(donor, "A+") == 0)
        return strcmp(recipient, "A+") == 0 ||
               strcmp(recipient, "AB+") == 0;

    if (strcmp(donor, "B-") == 0)
        return strcmp(recipient, "B-") == 0 ||
               strcmp(recipient, "B+") == 0 ||
               strcmp(recipient, "AB-") == 0 ||
               strcmp(recipient, "AB+") == 0;

    if (strcmp(donor, "B+") == 0)
        return strcmp(recipient, "B+") == 0 ||
               strcmp(recipient, "AB+") == 0;

    if (strcmp(donor, "AB-") == 0)
        return strcmp(recipient, "AB-") == 0 ||
               strcmp(recipient, "AB+") == 0;

    if (strcmp(donor, "AB+") == 0)
        return strcmp(recipient, "AB+") == 0;

    return 0;
}

void showGroups(void) {
    printf("\nAll 8 Blood Groups:\n");
    for (int i = 0; i < 8; i++)
        printf("%s  ", groups[i]);
    printf("\n");
}

void showDonors(Donor *donors, int count) {
    printf("\n========== DONOR RECORDS ==========\n");

    for (int i = 0; i < count; i++) {
        printf("\nName: %s", donors[i].name);
        printf("\nBlood Group: %s", donors[i].blood_type);
        printf("\nLast Donation: %s",
               donors[i].last_donation_date);
        printf("\nAvailable Units: %d\n",
               donors[i].available_units);
    }

    if (count == 0)
        printf("No donors available.\n");
}

void addDonor(Donor **donors, int *count) {
    Donor *temp = realloc(
        *donors, (*count + 1) * sizeof(Donor));

    if (temp == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    *donors = temp;
    Donor *d = &(*donors)[*count];

    printf("Enter donor name: ");
    scanf(" %49[^\n]", d->name);

    do {
        printf("Enter blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ");
        scanf("%3s", d->blood_type);

        if (!validGroup(d->blood_type))
            printf("Invalid blood group. Try again.\n");

    } while (!validGroup(d->blood_type));

    printf("Enter last donation date (YYYY-MM-DD): ");
    scanf("%10s", d->last_donation_date);

    printf("Enter available sample units: ");
    scanf("%d", &d->available_units);

    if (d->available_units < 0) {
        printf("Units cannot be negative. Donor not added.\n");
        return;
    }

    (*count)++;
    printf("Donor added successfully!\n");
}

void processRequest(
    Donor *donors, int donorCount,
    Recipient **recipients, int *requestCount) {

    Recipient r;

    printf("Enter recipient name: ");
    scanf(" %49[^\n]", r.name);

    do {
        printf("Enter recipient blood group: ");
        scanf("%3s", r.blood_type);

        if (!validGroup(r.blood_type))
            printf("Invalid blood group. Try again.\n");

    } while (!validGroup(r.blood_type));

    printf("Enter required units: ");
    scanf("%d", &r.required_units);

    if (r.required_units <= 0) {
        printf("Required units must be greater than zero.\n");
        return;
    }

    printf("Urgent request? (1 = Yes, 0 = No): ");
    scanf("%d", &r.urgent);

    if (r.urgent != 0 && r.urgent != 1) {
        printf("Enter 1 for urgent or 0 for standard.\n");
        return;
    }

    getDateTime(r.date, r.time);

    int remaining = r.required_units;

    printf("\nCompatible donors:\n");

    for (int i = 0; i < donorCount; i++) {
        if (compatible(donors[i].blood_type, r.blood_type)
            && donors[i].available_units > 0) {
            printf("%s | Group: %s | Units: %d\n",
                donors[i].name,
                donors[i].blood_type,
                donors[i].available_units);
        }
    }

    /* Allocate units, preferring exact blood-group matches. */
    for (int pass = 0; pass < 2 && remaining > 0; pass++) {
        for (int i = 0; i < donorCount && remaining > 0; i++) {

            if (donors[i].available_units <= 0 ||
                !compatible(donors[i].blood_type, r.blood_type))
                continue;

            int exact = strcmp(
                donors[i].blood_type, r.blood_type) == 0;

            if ((pass == 0 && !exact) ||
                (pass == 1 && exact))
                continue;

            int allocated = donors[i].available_units;

            if (allocated > remaining)
                allocated = remaining;

            donors[i].available_units -= allocated;
            remaining -= allocated;

            printf("Allocated %d unit(s) from %s.\n",
                   allocated, donors[i].name);
        }
    }

    r.fulfilled = (remaining == 0);

    Recipient *temp = realloc(
        *recipients,
        (*requestCount + 1) * sizeof(Recipient));

    if (temp == NULL) {
        printf("Memory allocation failed while saving request.\n");
        return;
    }

    *recipients = temp;
    (*recipients)[*requestCount] = r;
    (*requestCount)++;

    printf("\nRequest Date: %s", r.date);
    printf("\nRequest Time: %s", r.time);

    if (r.fulfilled)
        printf("\nStatus: FULFILLED\n");
    else
        printf("\nStatus: PENDING. Units still needed: %d\n",
               remaining);

    printf("Priority: %s\n",
           r.urgent ? "URGENT" : "STANDARD");
}

void showRequests(Recipient *r, int count) {
    printf("\n========== RECIPIENT REQUESTS ==========\n");

    if (count == 0) {
        printf("No requests recorded.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf("\nName: %s", r[i].name);
        printf("\nBlood Group: %s", r[i].blood_type);
        printf("\nRequired Units: %d", r[i].required_units);
        printf("\nPriority: %s",
               r[i].urgent ? "URGENT" : "STANDARD");
        printf("\nDate: %s", r[i].date);
        printf("\nTime: %s", r[i].time);
        printf("\nStatus: %s\n",
               r[i].fulfilled ? "FULFILLED" : "PENDING");
    }
}

void summary(Donor *d, int donorCount,
             Recipient *r, int requestCount) {
    int totals[8] = {0};
    int fulfilled = 0, pending = 0;
    char date[11], timeText[6];

    for (int i = 0; i < donorCount; i++) {
        for (int j = 0; j < 8; j++) {
            if (strcmp(d[i].blood_type, groups[j]) == 0) {
                totals[j] += d[i].available_units;
                break;
            }
        }
    }

    for (int i = 0; i < requestCount; i++) {
        if (r[i].fulfilled)
            fulfilled++;
        else
            pending++;
    }

    getDateTime(date, timeText);

    printf("\n========== DAILY SUMMARY ==========\n");
    printf("Date: %s\nTime: %s\n", date, timeText);

    for (int i = 0; i < 8; i++)
        printf("%s : %d units\n", groups[i], totals[i]);

    printf("Total donors: %d\n", donorCount);
    printf("Total requests: %d\n", requestCount);
    printf("Fulfilled requests: %d\n", fulfilled);
    printf("Pending requests: %d\n", pending);
}

int main(void) {
    int donorCount = 10;
    int requestCount = 0;

    Donor *donors = malloc(donorCount * sizeof(Donor));
    Recipient *recipients = NULL;

    if (donors == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    const char *names[] = {
        "Himanshu", "Shasvat", "Sachin", "Ishan",
        "Harshit", "Nikhil", "Aditi", "Vanya",
        "Astha", "Laxmi"
    };

    const char *blood[] = {
        "O-", "A+", "B+", "AB+", "O+",
        "A-", "B-", "AB-", "A+", "O+"
    };

    const char *dates[] = {
        "2026-10-10", "2026-10-12", "2026-10-13",
        "2026-10-14", "2026-10-15", "2026-10-16",
        "2026-10-17", "2026-10-18", "2026-10-19",
        "2026-10-20"
    };

    int units[] = {3, 2, 2, 1, 3, 2, 1, 2, 2, 1};

    for (int i = 0; i < donorCount; i++) {
        strcpy(donors[i].name, names[i]);
        strcpy(donors[i].blood_type, blood[i]);
        strcpy(donors[i].last_donation_date, dates[i]);
        donors[i].available_units = units[i];
    }

    int choice;

    do {
        printf("\n========================================\n");
        printf(" BLOOD BANK MANAGEMENT SYSTEM\n");
        printf("========================================\n");

        char date[11], timeText[6];
        getDateTime(date, timeText);
        printf("Date: %s | Time: %s\n", date, timeText);

        printf("\n1. View all donors\n");
        printf("2. Add new donor\n");
        printf("3. Process blood request\n");
        printf("4. View recipient requests\n");
        printf("5. Daily summary\n");
        printf("6. Show all blood groups\n");
        printf("0. Exit\n");

        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Restart the menu choice.\n");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }

        switch (choice) {
            case 1:
                showDonors(donors, donorCount);
                break;

            case 2:
                addDonor(&donors, &donorCount);
                break;

            case 3:
                processRequest(donors, donorCount,
                               &recipients, &requestCount);
                break;

            case 4:
                showRequests(recipients, requestCount);
                break;

            case 5:
                summary(donors, donorCount,
                        recipients, requestCount);
                break;

            case 6:
                showGroups();
                break;

            case 0:
                printf("Thank you for using the system!\n");
                break;

            default:
                printf("Invalid choice. Select 0 to 6.\n");
        }

    } while (choice != 0);

    free(donors);
    free(recipients);

    return 0;
}
