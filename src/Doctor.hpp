#pragma once
#include <memory>
#include <string>

#include "Patient.hpp"

class Doctor {
    std::string name;
    int age;
    std::string dob;
    int skill;
    int shiftStart;
    int shiftEnd;
    std::shared_ptr<Patient> current;
    int busyUntil;

public:
    Doctor(std::string name, int age, std::string dob, int skill,
           int shiftStart, int shiftEnd);

    std::string getName() const;
    int getAge() const;
    std::string getDob() const;
    int getSkill() const;
    int getShiftStart() const;
    int getShiftEnd() const;
    std::shared_ptr<Patient> getCurrent() const;
    int getBusyUntil() const;

    bool isOnShift(int tick) const;
    bool isFree(int tick) const;
    void treat(std::shared_ptr<Patient> p, int tick);
    void finish();
};
