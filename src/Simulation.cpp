#include "Simulation.hpp"
#include <fstream>
#include <sstream>
#include <string>

bool PatientOrder::operator()(const std::shared_ptr<Patient>& a,
                              const std::shared_ptr<Patient>& b) const {
    //implementation
}

Simulation::Simulation() : tick(0), patients(3), doctors(3) {
    //implementation
}

void Simulation::loadData(std::string patientCsv, std::string doctorCsv) {
    std::ifstream in(patientCsv);
    std::string line;
    std::getline(in, line); // skip header
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::vector<std::string> f;
        std::string field;
        while (std::getline(ss, field, ',')) f.push_back(field);
        pending.push_back(std::make_shared<Patient>(
            f[0], std::stoi(f[1]), f[2], std::stoi(f[3]), std::stoi(f[4]), std::stoi(f[5])));
    }
}

void Simulation::nextTick() {
    ++tick; // advance tick count
}

std::shared_ptr<Patient> Simulation::findPatient(std::string dob) {
    //implementation
}

std::shared_ptr<Doctor> Simulation::findDoctor(std::string dob) {
    //implementation
}

std::vector<std::shared_ptr<Patient>> Simulation::getWaiting() {
    return pending; // temporary placeholder
}

std::vector<std::shared_ptr<Doctor>> Simulation::getDoctors() {
    //implementation
}

int Simulation::getTick() const {
    return tick;
}
