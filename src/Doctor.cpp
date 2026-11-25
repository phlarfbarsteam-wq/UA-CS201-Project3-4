#include "Doctor.hpp"

Doctor::Doctor(std::string name, int age, std::string dob, int skill,
               int shiftStart, int shiftEnd) {
    //implementation
}

std::string Doctor::getName() const {
    //implementation
}
int Doctor::getAge() const {
    //implementation
}
std::string Doctor::getDob() const {
    //implementation
}
int Doctor::getSkill() const {
    //implementation
}
int Doctor::getShiftStart() const {
    //implementation
}
int Doctor::getShiftEnd() const {
    //implementation
}
std::shared_ptr<Patient> Doctor::getCurrent() const {
    //implementation
}
int Doctor::getBusyUntil() const {
    //implementation
}

bool Doctor::isOnShift(int tick) const {
    //implementation
}

bool Doctor::isFree(int tick) const {
    //implementation
}

void Doctor::treat(std::shared_ptr<Patient> p, int tick) {
    //implementation
}

void Doctor::finish() {
    //implementation
}
