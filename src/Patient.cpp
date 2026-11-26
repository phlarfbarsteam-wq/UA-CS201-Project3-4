#include "Patient.hpp"

#include <algorithm>
#include <utility>

Patient::Patient(std::string name, int age, std::string dob, int condition, int arrivalTick, int deadline) :
    name(std::move(name)), // Store name
    age(age), // Store age
    dob(std::move(dob)), // Store DOB
    condition(std::clamp(condition, 0, 10)), // store condition
    arrivalTick(arrivalTick), // Store arrival tick
    deadline(deadline) {} // Store deadline

std::string Patient::getName() const {
    return name;
}
int Patient::getAge() const {
    return age;
}
std::string Patient::getDob() const {
    return dob;
}
int Patient::getCondition() const {
    return condition;
}
int Patient::getArrivalTick() const {
    return arrivalTick;
}
int Patient::getDeadline() const {
    return deadline;
}

void Patient::escalate() {
    //implementation
}

void Patient::heal() {
    //implementation
}
