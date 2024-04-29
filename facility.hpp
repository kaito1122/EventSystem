#ifndef FACILITY_HPP
#define FACILITY_HPP

#include <algorithm>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <map>
#include "event.hpp"
#include <iomanip>

using namespace std;
using namespace std::chrono;

class Facility {
    vector<Event> events;
    double budget;  // Facility budget
public:
    Facility() : budget(0.0) {
        load_budget();
    }

    ~Facility() {
        save_budget();
        // No need to save events here as it's handled by the System
    }

    // gets events vector
    vector<Event>& get_events() {
        return events;
    }

    // print each event in the schedule
    void print_schedule(int days) {
        if (days < 1 || days > 14) {
            cout << "Please enter a number of days between 1 and 14.\n";
            return;
        }

        auto now = chrono::system_clock::now();
        auto end_time = now + chrono::hours(24 * days);

        vector<Event> filtered_events;
        for (const Event& event : events) {
            if (event.is_confirmed() && event.get_start_time() >= now && event.get_start_time() <= end_time) {
                filtered_events.push_back(event);
            }
        }

        // Sort by start time
        sort(filtered_events.begin(), filtered_events.end(),
            [](const Event& a, const Event& b) {
                return a.get_start_time() < b.get_start_time();
            });

        // Display events, grouped by day
        int current_day = -1;
        for (const Event& event : filtered_events) {
            auto event_time = event.get_start_time();
            auto days_since_epoch = chrono::duration_cast<chrono::hours>(event_time.time_since_epoch()).count() / 24;

            if (days_since_epoch != current_day) {
                current_day = days_since_epoch;
                time_t event_day_time_t = chrono::system_clock::to_time_t(event_time);
                cout << "\nDay: " << put_time(localtime(&event_day_time_t), "%Y-%m-%d") << "\n";
            }

            time_t start_time_t = chrono::system_clock::to_time_t(event_time);
            time_t end_time_t = chrono::system_clock::to_time_t(event.get_end_time());
            cout << "Event Name: " << event.get_name() << "\n"
                << "Organizer: " << event.get_creator_username() << "\n"
                << "Start Time: " << put_time(localtime(&start_time_t), "%R") << "\n"
                << "End Time: " << put_time(localtime(&end_time_t), "%R") << "\n"
                << "Ticket Cost: $" << event.get_cost_to_attend() << " per hour\n"
                << "Room Setup/Meeting Style: " << to_string(static_cast<int>(event.get_meeting_style())) << "\n"
                << "Other Details: " << (event.is_public() ? "Public" : "Private") << ", "
                << (event.is_open_to_non() ? "Open to non-residents" : "Not open to non-residents") << "\n"
                << "--------------------------\n";
        }
    }

    //...ads events
    void add_event(const Event& event) {
        events.push_back(event);
    }

    // making the reservation
    bool make_reservation(const string& event_name, const string& creator_username, const time_point<system_clock>& start_time, const time_point<system_clock>& end_time, double price_per_hour, bool pubpriv, bool open_to_non, MeetingStyle style, double cost_to_attend, User* user, map<string, User> users) {
        // Convert times to local time structure
        time_t start_timet = system_clock::to_time_t(start_time);
        time_t end_timet = system_clock::to_time_t(end_time);
        struct tm *start_tm = localtime(&start_timet);
        struct tm *end_tm = localtime(&end_timet);

        int start_hour = start_tm->tm_hour;
        int end_hour = end_tm->tm_hour;

        // Check for conflicts with existing events
        for (const Event& existing_event : events) {
            if ((start_time < existing_event.get_end_time() && end_time > existing_event.get_start_time()) ||
                start_time == existing_event.get_start_time()) {
                system_clock::time_point now = system_clock::now();
                if (duration_cast<seconds>(existing_event.get_start_time() - now).count() / (60*60*24) > 7) {
                    cout << "Over a week in advance, will override if applicable.\n";
                    if (existing_event.get_price_per_hour() == 5 || price_per_hour != 5) {
                        cout << "Override not applicable.\n";
                        return false;
                    } else {
                        cout << "Overriding current event reservation.\n";
                        cancel_event(existing_event.get_name(), user, users);
                        break;
                    }
                } else {
                    cout << "Event time conflict, cannot schedule event." << endl;
                    return false;
                }
            }
        }

        // Check operational hours
        if (start_hour < 9 || end_hour >= 21) {
            cout << "Event must start after 9 AM and finish by 9 PM." << endl;
            return false;
        }

        // If all checks pass, add the event
        Event new_event(event_name, creator_username, start_time, end_time, price_per_hour, pubpriv, open_to_non, style, cost_to_attend);
        events.push_back(new_event);
        cout << "Event successfully scheduled." << endl;
        return true;
    }


    // Method to display events organized by a specific user
    void display_events_by_organizer(const string& organizer_username) {
        bool found = false;
        cout << "Events organized by " << organizer_username << ":\n";
        for (const auto& event : events) {
            if (event.get_creator_username() == organizer_username) {
                found = true;
                // Convert time_point to time_t then to tm struct for formatting
                auto start_time_t = std::chrono::system_clock::to_time_t(event.get_start_time());
                auto end_time_t = std::chrono::system_clock::to_time_t(event.get_end_time());
                auto start_tm = *std::localtime(&start_time_t);
                auto end_tm = *std::localtime(&end_time_t);

                cout << "Event Name: " << event.get_name() << "\n"
                    << "Date: " << std::put_time(&start_tm, "%m-%d-%Y") << "\n"
                    << "Start Time: " << std::put_time(&start_tm, "%H:%M") << "\n"
                    << "Duration: " << std::chrono::duration_cast<std::chrono::hours>(event.get_end_time() - event.get_start_time()).count() << " hour(s)\n"
                    << "Meeting Style: " << static_cast<int>(event.get_meeting_style()) << "\n" // Consider translating enum to string
                    << "Public/Private: " << (event.is_public() ? "Public" : "Private") << "\n"
                    << "Open to Non-residents: " << (event.is_open_to_non() ? "Yes" : "No") << "\n"
                    << "Confirmed: " << (event.is_confirmed() ? "Yes" : "No") << "\n"
                    << "--------------------------\n";
            }
        }
        if (!found) {
            cout << "No events found for " << organizer_username << ".\n";
        }
    }

    //returns event_cost
    double get_event_cost(const string& event_name) {
        for (const auto& event : events) {
            if (event.get_name() == event_name && !event.is_confirmed()) {
                return event.calculate_total_cost() + 10; // Adding $10 service charge
            }
        }
        return -1; // Indicates the event was not found or already confirmed
    }

    //processes payment logic
    bool process_payment(const string& event_name, User* user, double amount_paid) {
        for (auto& event : events) {
            if (event.get_name() == event_name) {
                double total_cost = event.calculate_total_cost() + 10; // Including $10 service charge
                if (!event.is_confirmed() && user->get_bank_balance() >= amount_paid && amount_paid >= total_cost) {
                    user->set_bank_balance(user->get_bank_balance() - amount_paid); // Deduct the amount
                    budget+= amount_paid;
                    event.confirm(); // Confirm the event
                    return true;
                } else {
                    return false; // Payment failed due to insufficient funds or incorrect amount
                }
            }
        }
        return false; // Event not found
    }

    //checks if event tickets are allowed to be purchased
    bool check_availability(const string& event_name, User* user) {
        for (auto& event : events) {
            if (event.get_name() == event_name) {
                if (!event.is_public()) {
                    cout << "Event is not open to the public.\n";
                    return false;
                }
                if (!event.is_open_to_non()) {
                    cout << "Event is not open to non residents.\n";
                    return false;
                }
                if (!event.has_tickets()) {
                    event.join_waitlist(user);
                    return false;
                }
                return true;
            }
        }
        cout << "There is no event with the given event name! Double check the schedule and please try again! \n";
        return false;
    }

    //displays all events availabel to a specific user
    void display_available_events (User* currentUser) {
        for (const Event& event : events) {
            if (event.is_public() && !(currentUser->get_user_type() == 2 && !event.is_open_to_non()) && event.is_confirmed()) {
                    auto start_time_t = std::chrono::system_clock::to_time_t(event.get_start_time());
               
                    auto start_tm = *std::localtime(&start_time_t);
            cout << "Event name: " << event.get_name() << ", Date: " << std::put_time(&start_tm, "%m-%d-%Y")
                << ", Start Time: " << std::put_time(&start_tm, "%H:%M") << endl;
            }
        }
    }


    //buys ticket and returns true if done, 
    bool buy_ticket(const string& event_name, User* user) {
        for (auto& event : events) {
            if (event.get_name() == event_name) {
                return event.purchase_ticket(user);
            }
        }
        // should never reach here
        return false;
    }

    // pays event organizers for purchaseed tickets
    void pay_organizer(const string& event_name, User* user, map<string, User> users) {
        string username = user->get_user_name();
        for (auto& event : events) {
            if (event.get_name() == event_name) {
                if (users.find(username) != users.end()) {
                    User organizer = users[username];
                    cout << "Paid the organizer\n";
                    organizer.get_payment(event.get_cost_to_attend());
                }
            }
        }
    }
 
    // finds a ticket for a user
    bool find_ticket(const string& event_name, User* user) {
        for (auto& event : events) {
            if (event.get_name() == event_name) {
                return event.find_users_ticket(user->get_user_name());
            }
        }
        return false;
    }

    //cancels a ticket for a user
    void cancel_ticket(const string& event_name, User* user) {
        for (auto& event : events) {
            if (event.get_name() == event_name) {
                event.cancel_users_ticket(user->get_user_name());
            }
        }
    }

    // cancells event, refunds everyone
    bool cancel_event(string event_name, User* user, map<string, User> users){
        auto it = find_if(events.begin(), events.end(),
            [&](const Event& e) { return e.get_name() == event_name; });
        
        for (auto& event : events) { // refund purchased tickets if any
            if (event.get_name() == event_name) {
                event.cancel_all_tickets(users);
            }
        }
        if (it != events.end()) {
            // Calculate penalty if within 7 days from start
            system_clock::time_point now = system_clock::now();
            double penalty = 10;
            double event_cost = get_event_cost(it->get_name());
            if (duration_cast<seconds>(it->get_start_time() - now).count() / (60*60*24) < 7) {
                penalty += 0.01 * event_cost;  // Assuming rent_amount or similar can be fetched
            }     
            user->set_bank_balance(user->get_bank_balance() + event_cost - penalty);
            budget += penalty;
            events.erase(it);
            cout << "Event canceled with applicable penalties." << endl;
            return true;
        } else {
            cout << "Event not found." << endl;
        }
        return false;
    }

private:
//for saving and loading budget
void load_budget() {
        ifstream file("facility_budget.txt");
        if (file.is_open()) {
            file >> budget;
            file.close();
        }
    }

    void save_budget() const {
        ofstream file("facility_budget.txt");
        if (file.is_open()) {
            file << budget;
            file.close();
        }
    }

};

#endif // FACILITY_HPP

