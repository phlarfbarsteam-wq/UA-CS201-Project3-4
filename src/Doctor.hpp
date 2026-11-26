/*
 *   One simulation tick = one hour. Tick 0 is midnight. Shift hours are
 *   clock hours 0-23. The end hour is exclusive, and an end hour that is
 *   smaller than the start hour means the shift crosses midnight
 *   (e.g. 22 -> 6 is 10 PM until 6 AM). Ticks past 23 wrap around, so tick 34
 *   is hour 10 of the next day.
 *
 * DOCTOR STATES
 *   free: on shift and not treating anyone.
 *   busy: holds a patient until `busyUntil`.
 *   done: busy, and the tick has reached `busyUntil`; the hospital should
 *         call finish() to discharge the patient and free the doctor.
 *   A shift ending does not interrupt a treatment in progress; the doctor
 *   finishes that patient, but won't start a new one while off shift.
 */
#pragma once

#include <memory>
#include <string>

#include "Patient.hpp"

class Doctor {
public:
    // Skill runs from MIN_SKILL (slowest) to MAX_SKILL (fastest).
    static constexpr int MIN_SKILL = 1;
    static constexpr int MAX_SKILL = 10;
    // A treatment takes BASE_TREATMENT_TICKS / skill ticks (rounded up), so
    // skill 10 takes 2 ticks and skill 2 takes 6. Tune this one number to make
    // the whole hospital faster or slower.
    static constexpr int BASE_TREATMENT_TICKS = 12;
    static constexpr int HOURS_PER_DAY = 24;

private:
    std::string name;
    int age;
    std::string dob;                  // date of birth, "YYYY-MM-DD"
    int skill;                        // MIN_SKILL..MAX_SKILL
    int shiftStart;                   // 0-23
    int shiftEnd;                     // 0-24, exclusive
    std::shared_ptr<Patient> current; // null when not treating anyone
    int busyUntil;                    // tick the current treatment ends; 0 if idle

public:
    /*
     * Create a doctor who starts out free.
     * Input:  name, age, date of birth ("YYYY-MM-DD"), skill, and the shift
     *         start and end hours.
     * Output: a Doctor. Out-of-range values are repaired. Skill is clamped to
     *         1-10. Shift hours are wrapped into 0-23. Validate and warn about
     *         bad rows in the loader, since the repair is silent.
     */
    Doctor(std::string name, int age, std::string dob, int skill,
           int shiftStart, int shiftEnd);

    // Output: the doctor's name
    std::string getName() const;

    // Output: the doctor's age
    int getAge() const;

    // Output: the doctor's date of birth, "YYYY-MM-DD".
    std::string getDob() const;

    // Output: the doctor's skill (1-10, higher is faster).
    int getSkill() const;

    // Output: the hour (0-23) the shift starts.
    int getShiftStart() const;

    // Output: the hour (0-24) the shift ends, exclusive.
    int getShiftEnd() const;

    // Output: the patient being treated, or nullptr if the doctor is idle.
    std::shared_ptr<Patient> getCurrent() const;

    // Output: the tick the current treatment ends, or 0 if the doctor is idle.
    int getBusyUntil() const;

    /*
     * Check whether the doctor's shift covers a given time.
     * Input:  tick, the simulation time (any value; it wraps every 24).
     * Output: true if the hour is within the shift. A shift whose start equals
     *         its end is empty, so it is never on shift.
     */
    bool isOnShift(int tick) const;

    /*
     * Check whether the doctor can start a new patient right now.
     * Input:  tick, the simulation time.
     * Output: true if the doctor is on shift AND not holding a patient.
     */
    bool isFree(int tick) const;

    /*
     * Check whether the doctor's current treatment has run its course.
     * Input:  tick, the simulation time.
     * Output: true if there is a patient and tick has reached busyUntil. The
     *         hospital should then call finish().
     */
    bool isTreatmentDone(int tick) const;

    /*
     * Report how long this doctor takes to treat one patient.
     * Output: ticks per treatment: BASE_TREATMENT_TICKS / skill, rounded up,
     *         so it is always at least 1.
     */
    int treatmentDuration() const;

    /*
     * Start treating a patient.
     * Input:  the patient, and tick, the simulation time they start.
     * Output: true if treatment began (busyUntil = tick + treatmentDuration()).
     *         false, with nothing changed, if the patient pointer is null or
     *         the doctor is not free (off shift or already busy).
     */
    bool treat(std::shared_ptr<Patient> p, int tick);

    /*
     * End the current treatment and make the doctor idle.
     * Output: the patient who was being treated (so the hospital can discharge
     *         them), or nullptr if the doctor was already idle. The return
     *         value may be ignored.
     */
    std::shared_ptr<Patient> finish();
};