// Learn with Examples, 2020, MIT license
// https://www.learnsystemc.com/
// use SC bit
// Nodes are signals/channels, Transitions are Modules with ports
#include <sysc/kernel/sc_module.h>
#include <systemc>       // include the systemC header file
using namespace sc_core; // use namespace

void hello1() { // a normal c++ function
  std::cout << "Hello world using approach 1" << std::endl;
}

struct HelloWorld : sc_module
{ // define a systemC module

  SC_CTOR(HelloWorld) { // constructor function, to be explained later
    SC_METHOD(hello2);  // register a member function to the kernel
  }

  void hello2(void) { // a function for systemC simulation kernel, void inside () can be omitted
    std::cout << "Hello world using approach 2" << std::endl;
  }
};

SC_MODULE(MODULE_A) {
  SC_CTOR(MODULE_A) {
    std::cout << name() << " constructor" << std::endl;
  }
};

SC_MODULE(MODULE_B) {
  SC_CTOR(MODULE_B) {
    SC_METHOD(func_b);
  }
  void func_b() {
    std::cout << name() << std::endl;
  }
};

// module with more arguments in the constructor
SC_MODULE(MODULE_C) {
  const int i;
  SC_HAS_PROCESS(MODULE_C);
  // explicit constructor
  MODULE_C (sc_module_name name, int i) : sc_module(name), i(i) {
    SC_METHOD(func_c); // register member function
  }
    void func_c();
};

// can declare function outside of class
void MODULE_C::func_c() {
  std::cout << name() << ", i=" << i << std::endl;
}

int sc_main(int, char*[]) {            // entry point
  hello1();                            // approach #1: manually invoke a normal function
  HelloWorld helloworld("helloworld"); // approach #2, instantiate a systemC module
  MODULE_A module_A("module_A");
  MODULE_B module_B("module_B");
  MODULE_C module_C("module_C", 1);
  sc_start();                          // let systemC simulation kernel to invoke helloworld.hello2();
  return 0;
}