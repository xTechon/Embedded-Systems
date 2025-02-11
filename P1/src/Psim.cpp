// Instructions: <Opcode, Destination Reg, First Op, Second Op>
// Register File: <Reg name, Reg value>
// datamemory: <addr, value>

// tokens are memory locations

// declare transitions
// - each transition has a bool ACT flag
// - each transition will make a copy of neighboring nodes
// - each transition will have X number input "ports" and Y output "ports"
// - each "port" takes a node pointer
// - each transition may perform an action, after which the ACT flag is marked false
// - actions on node states from copies will be committed at the end of a simulation timestep

// declare nodes
// Instruction Mememory (init)
// - token format: <Opcode, Destination Reg, First Op, Second Op>
// - provides instruction tokens
// - max 16 tokens
//
// Register File (init)
// - 8 registers 0-7
// - up to 8 tokens per register
// - value can be 0-63
// 
// Data Memory (init)
// - 8 locations in memory
// - up to 8 tokens per time step
// - value can be 0-63

#include <iostream>
// argc is # of arguments including program execution
// argv is the array of strings of every argument including execution
int main(int argc, char* argv[]) {
    std::cout << "Hello World!" << std::endl;
    std::cout << argv[1] << std::endl;
    std::cout << argv[2] << std::endl;
    std::cout << argv[3] << std::endl;
    return 0;
}