#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "system.hpp"
#include "user.hpp"

using namespace std;
using namespace std::chrono;

int main() {
    System system;
    string username;
    int type;
    double balance;
    bool quit = false;

    cout << "Welcome to our facility ticketing system!\n";
    cout << "\nLogin\nUsername: ";
    cin >> username;

    User* currentUser = system.login_user(username);
    // handle creating a new user
    if (!currentUser) {
        cout << "No existing user found, creating new user...\n";
        cout << "Please tell me, how much money do you have in your budget?\n";
        cin >> balance;

        cout << "Please enter whatever best describes you: 1, 2, or 3\n1) Worker for the city\n2) Resident of Newton\n3) Non-Resident of Newton\n(Default: non-resident)\n";
        cin >> type;

        USER_TYPE userType = static_cast<USER_TYPE>(type - 1);
        system.create_user(username, balance, userType);
        currentUser = system.login_user(username);
    }

    cout << "Thanks for using our system, " << currentUser->get_user_name() << "!\n";

    while (!quit) {
        // print the options available for the user
        cout << "\nOptions:\n1. View Schedule\n2. Make reservation\n3. View and confirm your events/Make a payment\n4. Buy a ticket\n5. Cancel a ticket\n6. Cancel event\n7. View my tickets\n8. Quit\n";
        int operation;
        cin >> operation;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (operation) {
            case 1:
                system.print_schedule();
                // View schedule logic
                break;
            case 2:
                // Making a reservation
                system.process_reservation(currentUser);
                break;
            case 3:
                system.display_events_by_organizer(currentUser->get_user_name());
                // Payment logic
                system.process_payement(currentUser);
                break;
            case 4:
                system.buy_ticket(currentUser);
                // Buy a ticket
                break;
            case 5:
                system.cancel_ticket(currentUser);
                // Cancel a ticket
                break;
            case 6:
                system.cancel_event(currentUser);
                // Cancel an event
                break;
            case 7:
                system.print_tickets(currentUser);
                // View tickets
                break;
            case 8:
                quit = true;
                break;
            default:
                cout << "Invalid option. Please try again.\n";
                break;
        }
        if (quit) {
            cout << "Logging out, see you next time!\n";
            break;
        }
    }

    return 0;
}

