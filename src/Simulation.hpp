#pragma once
#include <memory>
#include <string>
#include <unordered_map>
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
    std::unordered_map<std::string, std::vector<Patient*>> patientsByName;
    std::vector<std::shared_ptr<Doctor>> doctorList;
    std::vector<std::shared_ptr<Patient>> pending;
    std::vector<std::shared_ptr<Patient>> waiting;

    void refreshDoctorIndex();
    void removePatientFromNameIndex(const Patient* patient);

public:
    Simulation();

    void loadData(std::string patientCsv, std::string doctorCsv);
    void nextTick();

    std::shared_ptr<Patient> findPatient(std::string dob);
    // Exact-name matches; returned pointers remain owned by this simulation.
    std::vector<Patient*> findPatientsByName(const std::string& name) const;
    std::shared_ptr<Doctor> findDoctor(std::string dob);

    std::vector<std::shared_ptr<Patient>> getWaiting();
    std::vector<std::shared_ptr<Doctor>> getDoctors();
    // Snapshot the DOB tree as levels containing nodes containing their keys.
    std::vector<std::vector<std::vector<std::string>>> getPatientTreeLevels() const;
    int getTick() const;
};
