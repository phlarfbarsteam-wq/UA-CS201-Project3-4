#pragma once
#include <memory>
#include <string>
#include <vector>

#include "BTree.hpp"
#include "Doctor.hpp"
#include "Patient.hpp"

struct PatientOrder {
    bool operator()(const std::shared_ptr<Patient>& a,
                    const std::shared_ptr<Patient>& b) const;
};

class Simulation {
    int tick;
    BTree<std::shared_ptr<Patient>> patients;
    BTree<std::shared_ptr<Doctor>> doctors;
    std::vector<std::shared_ptr<Doctor>> doctorList;
    std::vector<std::shared_ptr<Patient>> pending;
    std::vector<std::shared_ptr<Patient>> waiting;
public:
    Simulation();

    void loadData(std::string patientCsv, std::string doctorCsv);
    void nextTick();

    std::shared_ptr<Patient> findPatient(std::string dob);
    std::shared_ptr<Doctor> findDoctor(std::string dob);

    std::vector<std::shared_ptr<Patient>> getWaiting();
    std::vector<std::shared_ptr<Doctor>> getDoctors();
    int getTick() const;
};
