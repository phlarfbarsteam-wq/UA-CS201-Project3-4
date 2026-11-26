#pragma once
#include <string>

class Patient {
    std::string name;
    int age;
    std::string dob;
    int condition;
    int arrivalTick;
    int deadline;

public:
    Patient(std::string name, int age, std::string dob, int condition,
            int arrivalTick, int deadline);

    std::string getName() const;
    int getAge() const;
    std::string getDob() const;
    int getCondition() const;
    int getArrivalTick() const;
    int getDeadline() const;

    bool escalate();
    void heal();
};
