
// 1 tick = 1 hour

#include "Doctor.hpp"

#include <algorithm>
#include <utility>

namespace {

/*
 * Wrap any tick or hour onto the 24-hour clock.
 * Input:  any integer (negative values are allowed).
 * Output: the matching hour from 0 to 23. The extra "+ 24" step is needed
 *         because % can return a negative number.
 */
int wrapHour(int value) {
    const int hours = Doctor::HOURS_PER_DAY;
    return ((value % hours) + hours) % hours;
}

}  // namespace

Doctor::Doctor(std::string name, int age, std::string dob, int skill,
               int shiftStart, int shiftEnd)
    : name(std::move(name)),
      age(age),
      dob(std::move(dob)),
      // Keep skill inside the legal range so treatmentDuration() never divides
      // by zero and never returns a nonsense value.
      skill(std::clamp(skill, MIN_SKILL, MAX_SKILL)),
      shiftStart(wrapHour(shiftStart)),
      // An end of exactly 24 means "until midnight", so it must not wrap to 0
      // (that would turn a late shift into an empty one).
      shiftEnd(shiftEnd == HOURS_PER_DAY ? HOURS_PER_DAY : wrapHour(shiftEnd)),
      current(nullptr),
      busyUntil(0) {}

std::string Doctor::getName() const {
    return name;
}

int Doctor::getAge() const {
    return age;
}

std::string Doctor::getDob() const {
    return dob;
}

int Doctor::getSkill() const {
    return skill;
}

int Doctor::getShiftStart() const {
    return shiftStart;
}

int Doctor::getShiftEnd() const {
    return shiftEnd;
}

std::shared_ptr<Patient> Doctor::getCurrent() const {
    return current;
}

int Doctor::getBusyUntil() const {
    return busyUntil;
}

bool Doctor::isOnShift(int tick) const {
    // Ticks count forever, but shifts repeat daily, so convert to a clock hour.
    const int hour = wrapHour(tick);

    // Zero-length shift: the doctor never works.
    if (shiftStart == shiftEnd) {
        return false;
    }

    // Normal daytime shift, e.g. 8 -> 16: the hour must fall between the two.
    if (shiftStart < shiftEnd) {
        return hour >= shiftStart && hour < shiftEnd;
    }

    // Shift crosses midnight, e.g. 22 -> 6: on duty late in the evening OR
    // early in the morning (a single "between" test cannot express that).
    return hour >= shiftStart || hour < shiftEnd;
}

bool Doctor::isFree(int tick) const {
    // Holding a patient (even a finished one that has not been released with
    // finish() yet) counts as busy, so a patient can never be overwritten.
    return current == nullptr && isOnShift(tick);
}

bool Doctor::isTreatmentDone(int tick) const {
    return current != nullptr && tick >= busyUntil;
}

int Doctor::treatmentDuration() const {
    // Integer ceiling of BASE / skill: adding (skill - 1) before dividing
    // rounds up. Skill is always >= 1, so this is at least 1 tick.
    return (BASE_TREATMENT_TICKS + skill - 1) / skill;
}

bool Doctor::treat(std::shared_ptr<Patient> p, int tick) {
    // Refuse instead of crashing: no patient, off shift, or already busy.
    if (p == nullptr || !isFree(tick)) {
        return false;
    }

    current = std::move(p);
    busyUntil = tick + treatmentDuration();
    return true;
}

std::shared_ptr<Patient> Doctor::finish() {
    // Hand the patient back to the caller before forgetting them, so the
    // hospital can discharge that patient.
    std::shared_ptr<Patient> finished = std::move(current);
    current = nullptr;  // a moved-from shared_ptr is empty, but be explicit
    busyUntil = 0;
    return finished;
}