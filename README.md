# UA CS 201 Project 3/4: Hospital Simulation

## Build and run

```bash
cmake -S . -B build
cmake --build build
./build/hospital
```

Requires CMake 3.16+ and SFML 3.

## Layout

- `src/` source code
- `data/` patient and doctor CSV files
- `assets/` fonts
- `docs/` project instructions and rubrics
- `uml/` class diagram

## To-Do

- [ ] Implement `BTree.hpp` (`splitChild`, `insertNonFull`, `insert`, `search`)
- [ ] Implement `Patient` and `Doctor`
- [ ] Implement `Simulation` (CSV loading, `nextTick`, priority queue, lookups)
- [ ] Implement `App` (SFML window, Next Tick button, DOB lookup) and `main`
- [ ] Tag the final commit `SUBMISSION-PROJECT-3`, `PROJECT-3`, or some derivative
