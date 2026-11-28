#include "Simulation.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>

bool PatientOrder::operator()(const std::shared_ptr<Patient>& a,
                              const std::shared_ptr<Patient>& b) const {
    if (a->getCondition() != b->getCondition())
        return a->getCondition() < b->getCondition(); // Return lower priority first
    return a->getArrivalTick() > b->getArrivalTick(); // Return later arrival first
}

Simulation::Simulation() : tick(0), patients(3), doctors(3) {}

void Simulation::loadData(std::string patientCsv, std::string doctorCsv) {
    std::ifstream p_in(patientCsv);
    std::string line;
    std::getline(p_in, line); // skip header
    while (std::getline(p_in, line)) {
        std::stringstream ss(line);
        std::vector<std::string> f;
        std::string field;
        while (std::getline(ss, field, ',')) f.push_back(field);
        auto patient = std::make_shared<Patient>(
            f[0], std::stoi(f[1]), f[2], std::stoi(f[3]), std::stoi(f[4]), std::stoi(f[5]));
        pending.push_back(patient);
        patients.insert(patient->getDob(), patient);
        // pending owns the patient; the name index stores a non-owning pointer.
        patientsByName[patient->getName()].push_back(patient.get());
    }

    std::ifstream d_in(doctorCsv);
    std::getline(d_in, line); // skip header
    while (std::getline(d_in, line)) {
        std::stringstream ss(line);
        std::vector<std::string> f;
        std::string field;
        while (std::getline(ss, field, ',')) f.push_back(field);
        doctorList.push_back(std::make_shared<Doctor>(
            f[0], std::stoi(f[1]), f[2], std::stoi(f[3]), std::stoi(f[4]), std::stoi(f[5])));
    }

    refreshDoctorIndex();
}

void Simulation::nextTick() {
    for (const auto& d : doctorList) {
        if (d->isTreatmentDone(tick)) {
            auto finishedPatient = d->finish();
            finishedPatient->heal();
            // Discharged patients should no longer be returned by DOB lookup.
            patients.erase(finishedPatient->getDob());
            removePatientFromNameIndex(finishedPatient.get());
        }
    }

    bool changed = false;
    for (const auto& p : waiting) changed |= p->escalate(); // true if condition changed
    
    for (const auto& p : pending) {
        if (p->getArrivalTick() == tick) {
            waiting.push_back(p); // patient arrives
            changed = true;
        }
    }

    if (changed) std::make_heap(waiting.begin(), waiting.end(), PatientOrder()); // rebuild only if changed

    for (const auto& d : doctorList) {
        if (waiting.empty()) break;
        if (d->isFree(tick)) {
            std::pop_heap(waiting.begin(), waiting.end(), PatientOrder()); // top moves to back
            d->treat(waiting.back(), tick);
            waiting.pop_back();
        }
    }

    ++tick;
    // Keep doctor lookups aligned with the tick shown to the user.
    refreshDoctorIndex();
}

void Simulation::refreshDoctorIndex() {
    for (const auto& d : doctorList) {
        if (d->isOnShift(tick)) {
            doctors.insert(d->getDob(), d);
        } else {
            // The roster remains intact; only off-shift doctors leave the index.
            doctors.erase(d->getDob());
        }
    }
}

void Simulation::removePatientFromNameIndex(const Patient* patient) {
    // Drop the entry when its last same-name patient is discharged.
    auto entry = patientsByName.find(patient->getName());
    if (entry == patientsByName.end()) {
        return;
    }

    auto& matches = entry->second;
    matches.erase(std::remove(matches.begin(), matches.end(), patient),
                  matches.end());
    if (matches.empty()) {
        patientsByName.erase(entry);
    }
}

std::shared_ptr<Patient> Simulation::findPatient(std::string dob) {
    auto* p = patients.search(dob); // pointer to stored shared_ptr (of patient)
    return p ? *p : nullptr; // return shared_ptr if found, else nullptr
}

std::vector<Patient*> Simulation::findPatientsByName(
    const std::string& name) const {
    // Return every active exact-name match in one hash lookup.
    const auto entry = patientsByName.find(name);
    return entry == patientsByName.end() ? std::vector<Patient*>{}
                                         : entry->second;
}

std::shared_ptr<Doctor> Simulation::findDoctor(std::string dob) {
    auto* d = doctors.search(dob); // pointer to stored shared_ptr (of doctor)
    return d ? *d : nullptr; // return shared_ptr if found, else nullptr
}

std::vector<std::shared_ptr<Patient>> Simulation::getWaiting() {
    auto out = waiting;
    std::sort_heap(out.begin(), out.end(), PatientOrder()); // lowest priority first
    std::reverse(out.begin(), out.end()); // highest priority first
    return out;
}

std::vector<std::shared_ptr<Doctor>> Simulation::getDoctors() {
    return doctorList;
}

std::vector<std::vector<std::vector<std::string>>>
Simulation::getPatientTreeLevels() const {
    return patients.levelKeys();
}

int Simulation::getTick() const {
    return tick;
}
