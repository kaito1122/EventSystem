#ifndef EVENT_HPP
#define EVENT_HPP

#include <iostream>
#include <string>
#include <chrono>
#include <deque>
#include <map>
#include <algorithm>
#include "ticket.hpp"

using namespace std;
using namespace std::chrono;

enum MeetingStyle {
    Meeting,
    Lecture,
    Wedding,
    DanceRoom
};

class Event {
    string event_name;
    string creator_username; // Username of the event creator
    time_point<system_clock> start_time;
    time_point<system_clock> end_time;
    double price_per_hour;
    bool confirmed;
    bool pubpriv;          // true for public, false for private
    bool open_to_non;      // true for open, false for closed to non-residents
    MeetingStyle meeting_style;
    deque<Ticket> tickets;
    deque<User*> waitlist;
    double cost_to_attend;

public:
    Event(const string& name, const string& creator, const time_point<system_clock>& start, const time_point<system_clock>& end, double price, bool public_private, bool open_non_residents, MeetingStyle style, double cost_to_attend)
        : event_name(name), creator_username(creator), start_time(start), end_time(end), price_per_hour(price), confirmed(false), pubpriv(public_private), open_to_non(open_non_residents), meeting_style(style), cost_to_attend(cost_to_attend) {
            deque<Ticket> new_tickets;
            if(pubpriv) { // initializing 25 5ickets
                for (int i = 0; i < 25; i++) {
                    Ticket t(name, cost_to_attend);
                    new_tickets.push_back(t);
                }
            }
            tickets = new_tickets;
            deque<User*> new_waitlist;
            waitlist = new_waitlist;
        }

    //calculates price for event
    double calculate_total_cost() const {
        // Calculate the duration in hours
        auto duration = duration_cast<hours>(end_time - start_time).count();
        
        // Calculate the total cost
        return price_per_hour * duration;
    }

    // gets waitlist
    deque<User*> get_waitlist() const{
        return waitlist;
    }

    // Accessor methods for all fields
    string get_name() const {
        return event_name;
    }

    string get_creator_username() const {
        return creator_username;
    }

    time_point<system_clock> get_start_time() const {
        return start_time;
    }

    time_point<system_clock> get_end_time() const {
        return end_time;
    }

    double get_price_per_hour() const {
        return price_per_hour;
    }

    bool is_confirmed() const {
        return confirmed;
    }

    bool is_public() const {
        return pubpriv;
    }

    bool is_open_to_non() const {
        return open_to_non;
    }

    MeetingStyle get_meeting_style() const {
        return meeting_style;
    }

    void confirm() {
        confirmed = true;
    }

    void cancel() {
        confirmed = false;
    }

    double get_cost_to_attend() const {
        return cost_to_attend;
    }

    deque<Ticket> get_tickets() const {
        return tickets;
    }

    // checks if there are tickets still available
    bool has_tickets() {
        for (auto& ticket : tickets) {
            if (!ticket.is_purchased()) {
                cout << "There are tickets still available.\n";
                return true;
            }
        }
        cout << "No more tickets.\n";
        return false;
    }

    // adds user to the waitlist
    void join_waitlist(User* user) {
        cout << "User added to the waitlist.\n";
        waitlist.push_back(user);
    }

    //purchase ticket logic
    bool purchase_ticket(User* user) {
        if (user->get_bank_balance() < cost_to_attend) {
            cout << "User does not have enough money in bank account.\n";
            return false;
        }
        user->set_bank_balance(user->get_bank_balance() - cost_to_attend);
        Ticket t = tickets.front();
        tickets.pop_front();
        Ticket new_ticket(t.get_event_name(), t.get_cost(), user->get_user_name());
        user->add_ticket(new_ticket);
        tickets.push_back(new_ticket);
        return true;
    }

    // seraches through tickets for a users 
    bool find_users_ticket(string user_name) {
        for (auto& ticket : tickets) {
            if (ticket.get_owner() == user_name) {
                cout << "found the ticket\n"; 
                return true;
            }
        }
        return false;
    }

  // cancells a users ticket and checks waitlist
  void cancel_users_ticket(const string& user_name) {
    bool ticketFound = false;

    // Iterate to find the user's ticket
    for (auto it = tickets.begin(); it != tickets.end(); ) {
        if (it->get_owner() == user_name && it->is_purchased()) {
            cout << "Found the ticket. Cancelling and checking waitlist.\n";
            it->set_purchased(false); // Mark the ticket as not purchased
            it->set_owner("");       // Clear the holder name

            ticketFound = true;

            // Continue to check the waitlist
            while (!waitlist.empty()) {
                User* nextUser = waitlist.front();
                waitlist.pop_front(); // Remove the user from the waitlist

                if (nextUser->get_bank_balance() >= cost_to_attend) {
                    // If the waitlisted user can afford the ticket, process the purchase
                    nextUser->set_bank_balance(nextUser->get_bank_balance() - cost_to_attend);
                    it->set_owner(nextUser->get_user_name());
                    it->set_purchased(true);

                    nextUser->add_ticket(*it);  // Add the ticket to the next user's list of tickets
                    cout << "Ticket transferred to waitlisted user: " << nextUser->get_user_name() << endl;
                    return;  // Exit after successfully transferring the ticket
                } else {
                    cout << "User on waitlist cannot afford the ticket. Skipping: " << nextUser->get_user_name() << endl;
                }
            }

            // If no suitable user is found in the waitlist, add the ticket back as available
            cout << "No suitable user found in waitlist. Ticket remains available.\n";
            tickets.erase(it);
            Ticket t(event_name, cost_to_attend);
            tickets.push_front(t); // pushes to front since that's where blank tickets belong.
            break;  // Exit the loop after handling the ticket
        } else {
            ++it;  // Only increment if no ticket was found, to avoid skipping elements
        }
    }

    if (!ticketFound) {
        cout << "No ticket to cancel found for user: " << user_name << endl;
    }
}
 
 //loads tickets form save
    void load_ticket(const Ticket& new_ticket) {
        tickets.pop_front();
        tickets.push_back(new_ticket);
    }

    //cancels all tickets and refunds everyone
    void cancel_all_tickets(map<string, User> users) {
        cout << "cancelling all tickets\n";
        for (auto& ticket : tickets) {
            User temp = users[ticket.get_owner()];
            temp.cancel_ticket(event_name);
        }
    }

};

#endif // EVENT_HPP

