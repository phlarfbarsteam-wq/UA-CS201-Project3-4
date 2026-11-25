#include "Simulation.hpp"

bool PatientOrder::operator()(const std::shared_ptr<Patient>& a,
                              const std::shared_ptr<Patient>& b) const {
    //implementation
}

Simulation::Simulation() : tick(0), patients(3), doctors(3) {
    //implementation
}

void Simulation::loadData(std::string patientCsv, std::string doctorCsv) {
    //implementation
}

void Simulation::nextTick() {
    //implementation
}

std::shared_ptr<Patient> Simulation::findPatient(std::string dob) {
    //implementation
}

std::shared_ptr<Doctor> Simulation::findDoctor(std::string dob) {
    //implementation
}

std::vector<std::shared_ptr<Patient>> Simulation::getWaiting() {
    //implementation
}

std::vector<std::shared_ptr<Doctor>> Simulation::getDoctors() {
    //implementation
}

int Simulation::getTick() const {
    return 0; // placeholder
}
