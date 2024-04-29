#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "user.hpp"
#include "facility.hpp"
#include <limits>

using namespace std;

class System {
    map<string, User> users;
    Facility facility;

public:
    System() {
        load_users_from_file("users.csv");
        load_events("events_data.csv");
        load_waitlists();
    }

    ~System() {
        save_users_to_file("users.csv");
        save_events("events_data.csv"); // Save events when the facility is destroyed
        save_waitlists();
    }

    // allow the user to login
    User* login_user(const string& username) {
        // Check if the user exists in the map
        auto it = users.find(username);
        if (it != users.end()) {
            return &it->second; // Return a pointer to the User
        }
        return nullptr; // Return nullptr if user does not exist
    }

    // create a new user
    void create_user(const string& username, double balance, USER_TYPE userType) {
        // Check if user already exists
        if (users.find(username) == users.end()) {
            // Create and insert new user if not found
            users.emplace(piecewise_construct,
                          forward_as_tuple(username),
                          forward_as_tuple(username, balance, userType));
        }
    }

    // print the schedule for the user to see, x days in advance
    void print_schedule() {
        int days;
        cout << "How many days of the schedule would you like to see (up to 14 days)? ";
        cin >> days;

        // Check for proper input
        if(cin.fail()) {
            cin.clear(); // Clear error state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore wrong input
            cout << "Invalid input. Please enter a number.\n";
        } else {
            facility.print_schedule(days);
        }
    }

    // get all the info needed for making a reservation
    void process_reservation(User* currentUser) {
        string event_name, date_str;
        int start_hour, duration, style_choice;
        double cost_to_attend;
        bool pubpriv, open_to_non;
        cout << "Enter event name: ";
        getline(cin, event_name);  // Use getline to read strings to handle spaces and new lines
        cout << "What date do you want the event (MM-DD-YYYY)? ";
        getline(cin, date_str);
        cout << "Starting time, hour in military time (e.g., 20 for 8 PM): ";
        cin >> start_hour;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the buffer
        cout << "Length in hours (integer only): ";
        cin >> duration;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Is the event public or private (1 for public, 0 for private): ";
        cin >> pubpriv;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Is the event open to non-residents (1 for open, 0 for closed): ";
        cin >> open_to_non;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Choose meeting style (1 for Meeting, 2 for Lecture, 3 for Wedding, 4 for Dance Room): ";
        cin >> style_choice;
        cout << "If this event is public how much would you like to charge for a ticket? If private, enter 0.\n";
        cin >> cost_to_attend;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear any remaining data


        MeetingStyle meeting_style;
        switch (style_choice) {
            case 1:
                meeting_style = Meeting;
                break;
            case 2:
                meeting_style = Lecture;
                break;
            case 3:
                meeting_style = Wedding;
                break;
            case 4:
                meeting_style = DanceRoom;
                break;
            default:
                cout << "Invalid meeting style selected. Defaulting to Meeting." << endl;
                meeting_style = Meeting;
        }

        // Set price based on user type
        double price_per_hour = 10; // default for residents
        if (currentUser->get_user_type() == NON_RESIDENT) {
            price_per_hour = 15;
        } else if (currentUser->get_user_type() == CITY) {
            price_per_hour = 5;
            if (meeting_style == Wedding) {
                cout << "City events cannot be reserved with the Wedding style." << endl;
                return; // Exit case if city tries to book a wedding
            }
        }

        istringstream date_stream(date_str);
        tm date_tm = {};
        date_stream >> get_time(&date_tm, "%m-%d-%Y");
        system_clock::time_point event_date = system_clock::from_time_t(mktime(&date_tm));
        system_clock::time_point start_time = event_date + hours(start_hour);
        system_clock::time_point end_time = start_time + hours(duration);

        if (make_reservation(event_name, currentUser->get_user_name(), start_time, end_time, price_per_hour, pubpriv, open_to_non, meeting_style, cost_to_attend, currentUser, users)) {
            cout << "Reservation created successfully.\n";
        } else {
            cout << "Failed to create reservation.\n";
        }
    }

    // make the reservation
    bool make_reservation(const string& event_name, const string& username, const time_point<system_clock>& start_time, const time_point<system_clock>& end_time, double price_per_hour, bool pubpriv, bool open_to_non, MeetingStyle style, double cost_to_attend, User* user, map<string, User> users) {
        if (users.find(username) != users.end()) {
            return facility.make_reservation(event_name, username, start_time, end_time, price_per_hour, pubpriv, open_to_non, style, cost_to_attend, user, users);
        }
        return false;
    }
    
    void display_events_by_organizer(const string& organizer_username) {
        facility.display_events_by_organizer(organizer_username);
    }

    // process payment for a reservation
    void process_payement(User* currentUser) {
        string event_name;
        cout << "Enter the name of the event you wish to pay for: ";
        getline(cin, event_name);   
 
        double total_cost = facility.get_event_cost(event_name);
        if (total_cost == -1) {
            cout << "Event not found or already confirmed.\n";
            return;
        }

        cout << "Your current budget is: $" << currentUser->get_bank_balance() << endl;
        cout << "Total cost of the event: $" << total_cost << endl;

        if (currentUser->get_bank_balance() < total_cost) {
            cout << "Insufficient funds to cover the cost of the event.\n";
            return;
        }

        char userConfirmation;
        cout << "Do you wish to proceed with the payment? (Y/N): ";
        cin >> userConfirmation;

        if (toupper(userConfirmation) == 'Y') {
            if (facility.process_payment(event_name, currentUser, total_cost)) {
                cout << "Payment successful and event confirmed." << endl;
            } else {
                cout << "Payment failed." << endl;
            }
        } else {
            cout << "Payment cancelled." << endl;
        }
    }

    // get which event the user wants to buy a ticket for
    void buy_ticket(User* currentUser) {
        cout << "Buying a ticket! These are all of the available events:" << endl;
        facility.display_available_events(currentUser);
        string event_name;
        cout << "Enter the event name in which you want to attend: \n";
        cout << "If the event is sold out you will automatically be added to the waitlist.\n";
        getline(cin, event_name);
        if (!facility.check_availability(event_name, currentUser)) {
            return;
        }
        if (facility.buy_ticket(event_name, currentUser)) {
            facility.pay_organizer(event_name, currentUser, users);
            cout << "Ticket purchase successful!\n";
        } else {
            cout << "Was not able to purchase ticket\n";
        }
        cout << "bye\n";
    }

    // what event the user wants to cancel their ticket for
    void cancel_ticket(User* currentUser) {
        cout << "Cancelling a ticket.\n";
        string event_name;
        cout << "Enter the name of the event you want to cancel your ticket for.\n";
        cin >> event_name;
        if (facility.find_ticket(event_name, currentUser)) {
            cout << "Cancelling your ticket\n"; 
            facility.cancel_ticket(event_name, currentUser);
            currentUser->cancel_ticket(event_name);
            return;
        }
        cout << "It does not look like you have a ticket to this event\n";
    }

    // print the tickets that the user has
    void print_tickets(User* currentUser) {
        currentUser->print_tickets();
    }

    // cancel an event if the user is the organizer
    void cancel_event(User* currentUser){
        cout << "Cancelling a hosting event!" << endl;
        string event_name;
        cout<<"Enter the event name in which you host and want to cancel: "<<endl;
        getline(cin, event_name);
        if(facility.cancel_event(event_name, currentUser, users)){
            cout << "Cancellation successful\n";
        } else {
            cout << "Cancellation unsuccessful\n";
        }
        cout << "bye\n";
    }


private:
//csv style loading events and tickets
    void load_events(const string& data_file) {
        ifstream file(data_file);
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name, creator, start_str, end_str, style_str, pubpriv_str, open_str, confirmed_str;
            int price_per_hour, cost_to_attend;
            getline(ss, name, ',');
            getline(ss, creator, ',');
            getline(ss, start_str, ',');
            getline(ss, end_str, ',');
            ss >> price_per_hour; ss.ignore();
            getline(ss, pubpriv_str, ',');
            getline(ss, open_str, ',');
            getline(ss, style_str, ',');
            getline(ss, confirmed_str, ',');
            ss >> cost_to_attend; ss.ignore();

            time_point<system_clock> start = system_clock::from_time_t(stoll(start_str));
            time_point<system_clock> end = system_clock::from_time_t(stoll(end_str));
            bool pubpriv = pubpriv_str == "1";
            bool open_to_non = open_str == "1";
            MeetingStyle style = static_cast<MeetingStyle>(stoi(style_str));
            bool confirmed = confirmed_str == "1";

            Event loaded_event(name, creator, start, end, price_per_hour, pubpriv, open_to_non, style, cost_to_attend);
            if (confirmed) {
                loaded_event.confirm();
            }

            while (ss.good()) {
                string ticket_price_str, ticket_holder, ticket_purchased_str;
                getline(ss, ticket_price_str, ',');
                getline(ss, ticket_holder, ',');
                getline(ss, ticket_purchased_str, ',');
                if (!ticket_price_str.empty() && !ticket_holder.empty() && !ticket_purchased_str.empty()) {
                    int ticket_price = stoi(ticket_price_str);
                    bool ticket_purchased = stoi(ticket_purchased_str) == 1;
                    Ticket ticket(name, ticket_price, ticket_holder);
                    ticket.set_purchased(ticket_purchased);
                    loaded_event.load_ticket(ticket);
                    // Assign ticket to user if the user exists
                        auto user_it = users.find(ticket_holder);
                        if (user_it != users.end()) {
                            user_it->second.add_ticket(ticket); // Assume addTicket takes a pointer
                        } else {
                            // Optionally handle or log if no such user exists
                            cout << "No user found for username: " << ticket_holder << endl;
                        }
                }
            }

            facility.add_event(loaded_event);
        }
        file.close();
    }

    void save_events(const string& data_file) {
        ofstream file(data_file);
        for (const Event& event : facility.get_events()) {
            file << event.get_name() << ","
                << event.get_creator_username() << ","
                << duration_cast<seconds>(event.get_start_time().time_since_epoch()).count() << ","
                << duration_cast<seconds>(event.get_end_time().time_since_epoch()).count() << ","
                << event.get_price_per_hour() << ","
                << event.is_public() << ","
                << event.is_open_to_non() << ","
                << static_cast<int>(event.get_meeting_style()) << ","
                << event.is_confirmed() << ","
                << event.get_cost_to_attend();

            // Serialize tickets
            for (const Ticket& ticket : event.get_tickets()) {
                file << "," << ticket.get_cost() << ","
                    << ticket.get_owner() << ","
                    << ticket.is_purchased();
            }
            file << "\n";
        }
        file.close();
    }

//csv style loading and saving users
    void load_users_from_file(const string& filename) {
        ifstream file(filename);
        string line;
        while (getline(file, line)) {
            stringstream linestream(line);
            string name;
            int balance;
            int type;
            getline(linestream, name, ',');
            linestream >> balance >> type;
            users[name] = User(name, balance, static_cast<USER_TYPE>(type));
        }
    }

    void save_users_to_file(const string& filename) {
        ofstream file(filename);
        for (const auto& pair : users) {
            const User& user = pair.second;
            file << user.get_user_name() << ',' << user.get_bank_balance() << ',' << static_cast<int>(user.get_user_type()) << '\n';
        }
    }

//csv style saving and loading waitlists in a waitlist directory.
void load_waitlists() {
        auto& events = facility.get_events();
        for (auto& event : events) {
            string filename = "waitlist/waitlist_" + event.get_name() + ".csv";
            ifstream file(filename);
            if (!file.is_open()) {
                cerr << "Failed to open waitlist file: " << filename << endl;
                continue;
            }
            string username;
            while (getline(file, username)) {
                User* user = login_user(username);
                if (user) {
                    event.join_waitlist(user);
                }
            }
            file.close();
        }
    }

void save_waitlists() {
        const auto& events = facility.get_events();
        for (const auto& event : events) {
            string filename = "waitlist/waitlist_" + event.get_name() + ".csv";
            ofstream file(filename);
            if (!file.is_open()) {
                cerr << "Failed to open waitlist file for writing: " << filename << endl;
                continue;
            }
            const auto& waitlist = event.get_waitlist();
            for (const auto* user : waitlist) {
                if (user) {
                    file << user->get_user_name() << '\n';
                }
            }
            file.close();
        }
    }


};

#endif // SYSTEM_HPP

