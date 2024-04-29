#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "ticket.hpp"

using namespace std;

enum USER_TYPE {
    CITY = 0,
    RESIDENT = 1,
    NON_RESIDENT = 2
};

class User {
    string name;
    double bank_balance;
    USER_TYPE user_type;
    vector<Ticket> tickets_owned;

public:
    User() {}
    User(const string& name, int balance, USER_TYPE type) : name(name), bank_balance(balance), user_type(type) {
        vector<Ticket> tickets;
    }

    //Standard getter and setters
    string get_user_name() const {
        return name;
    }

    void set_user_name(const string& username) {
        name = username;
    }

    double get_bank_balance() const {
        return bank_balance;
    }

    void set_bank_balance(double balance) {
        bank_balance = balance;
    }

    USER_TYPE get_user_type() const {
        return user_type;
    }

    void set_user_type(USER_TYPE type) {
        user_type = type;
    }

    vector<Ticket> get_tickets() {
        return tickets_owned;
    }

    void add_ticket(Ticket ticket) {
        tickets_owned.push_back(ticket);
    }

    void get_payment(double amount) {
        bank_balance += amount;
    }
    
    //cancel ticket logic
    void cancel_ticket(const string& event_name) {
        for (auto it = tickets_owned.begin(); it != tickets_owned.end(); ) {
            if (it->get_owner() == name) {
                cout << "found the ticket\n";
                tickets_owned.erase(it);
                return; 
            } else {
                ++it;
            }
        }
    }

    //logic to print all tickets you own for an event, inclduing duplicates
    void print_tickets() {
        for (auto& ticket : tickets_owned) {
            cout << "Ticket for event named: " << ticket.get_event_name() << " cost $" << ticket.get_cost() << endl;
        }
    }

};

#endif // USER_HPP

