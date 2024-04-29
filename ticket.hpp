#ifndef TICKET_TICKET_HPP
#define TICKET_TICKET_HPP

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

class Ticket {
    string eventName;
    double cost; // could use float double if we want
    string owner_name;
    bool been_purchased;
    
public:
    Ticket(const string& name, double price) : eventName(name), cost(price) {
        been_purchased = false;
    }

    Ticket(string name, double price, string owner){
        been_purchased = true;
        eventName = name;
        cost = price;
        owner_name = owner;
    }

    // Standard getters and setters
    string get_event_name() const {
        return eventName;
    }

    double get_cost() const {
        return cost;
    }
    
    void set_cost(double new_cost) {
        cost = new_cost;
    }

    string get_owner() const {
        return owner_name;
    }

    void set_owner(const string& newOwner) {
        owner_name = newOwner;
    }

    bool is_purchased() const {
        return been_purchased;
    }
    
    void set_purchased(bool status) {
        been_purchased = status;
    }
};

#endif // TICKET_TICKET_HPP

 
