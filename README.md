# TicketSystem

SJTU CS1951 Homework

**requirement**: [homework_requirement](./docs/README.md)  
**command**: [management_system](./docs/management_system.md)

## How to run


Compile

```shell
cmake . && make
```

Run

```shell
./code
```


## Structure


source code files structure:

```plaintext
src
├── include
│   ├── BPlusTree.hpp
│   ├── exceptions.hpp
│   ├── File.hpp
│   ├── Hashmap.hpp
│   ├── Map.hpp
│   ├── Stack.hpp
│   ├── String.hpp
│   ├── utility.hpp
│   └── Vector.hpp
├── main.cpp
└── src
    ├── TicketSystem.hpp
    ├── Train.hpp
    ├── TrainSystem.hpp
    ├── User.hpp
    ├── UserSystem.hpp
    └── utils.hpp
```

`utils.hpp` Define some utility functions and some type alias.
`Train.hpp` Define some classes related to train.
`User.hpp` Define some classes related to user.

`UserSystem.hpp` implement the UserSystem, `TrainSystem.hpp` implement the TrainSystem. They are the two main parts. `TicketSystem.hpp` is the main class to manage the two systems.


