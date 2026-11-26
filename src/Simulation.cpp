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

Simulation::Simulation() : tick(0), patients(3), doctors(3) {
    //implementation
}

void Simulation::loadData(std::string patientCsv, std::string doctorCsv) {
    std::ifstream p_in(patientCsv);
    std::string line;
    std::getline(p_in, line); // skip header
    while (std::getline(p_in, line)) {
        std::stringstream ss(line);
        std::vector<std::string> f;
        std::string field;
        while (std::getline(ss, field, ',')) f.push_back(field);
        pending.push_back(std::make_shared<Patient>(
            f[0], std::stoi(f[1]), f[2], std::stoi(f[3]), std::stoi(f[4]), std::stoi(f[5])));
    }

    for (const auto& p : pending) patients.insert(p->getDob(), p); // add to patients map
    
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

    for (const auto& d : doctorList) doctors.insert(d->getDob(), d); // add to doctors map
}

void Simulation::nextTick() {
    bool changed = false;
    for (const auto& p : waiting) changed |= p->escalate(); // true if condition changed
    
    for (const auto& p : pending) {
        if (p->getArrivalTick() == tick) {
            waiting.push_back(p); // patient arrives
            changed = true;
        }
    }

    if (changed) std::make_heap(waiting.begin(), waiting.end(), PatientOrder()); // rebuild only if changed

    ++tick;
}

std::shared_ptr<Patient> Simulation::findPatient(std::string dob) {
    auto* p = patients.search(dob); // pointer to stored shared_ptr (of patient)
    return p ? *p : nullptr; // return shared_ptr if found, else nullptr
}

std::shared_ptr<Doctor> Simulation::findDoctor(std::string dob) {
    //implementation
}

std::vector<std::shared_ptr<Patient>> Simulation::getWaiting() {
    auto out = waiting;
    std::sort_heap(out.begin(), out.end(), PatientOrder()); // lowest priority first
    std::reverse(out.begin(), out.end()); // highest priority first
    return out;
}

std::vector<std::shared_ptr<Doctor>> Simulation::getDoctors() {
    //implementation
}

int Simulation::getTick() const {
    return tick;
}
