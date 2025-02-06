#include <sysc/communication/sc_port.h>
#include <sysc/communication/sc_signal_ifs.h>
#include <sysc/communication/sc_signal_ports.h>
#include <sysc/datatypes/bit/sc_bit.h>
#include <sysc/kernel/sc_module.h>
#include <systemc>

using namespace sc_core;

SC_MODULE(ONE_TO_ONE) {
  // declare ports
  sc_inout<sc_dt::sc_bit> in1;
  sc_out<sc_dt::sc_bit> out1;

  SC_CTOR(ONE_TO_ONE) {
    // declare concurrency
    SC_THREAD(transition);
    sensitive << in1;
  }

  // declare 1 and 0 bits
  sc_dt::sc_bit flag;
  sc_dt::sc_bit reset;
  void transition() {
    flag  = 1;
    reset = 0;
    while (true) {
      if (in1->read() == 1) {
        // reset input
        in1->write(reset);
        // write to output
        out1->write(flag);
      }
      wait();
    }
  };
};

SC_MODULE(TWO_TO_ONE) {
  // declare ports
  sc_inout<sc_dt::sc_bit> in1;
  sc_inout<sc_dt::sc_bit> in2;
  sc_out<sc_dt::sc_bit> out1;

  SC_CTOR(TWO_TO_ONE) {
    // declare concurrency
    SC_THREAD(transition);
    sensitive << in1 << in2;
  }

  // declare 1 and 0 bits
  sc_dt::sc_bit flag;
  sc_dt::sc_bit reset;
  void transition() {
    flag  = 1;
    reset = 0;
    while (true) {
      if (in1->read() == 1 && in2->read() == 1) {
        // reset inputs
        in1->write(reset);
        in2->write(reset);
        // write to outputs
        out1->write(flag);
      }
      wait();
    }
  }
};

SC_MODULE(TWO_TO_TWO) {
  // declare ports
  sc_inout<sc_dt::sc_bit> in1;
  sc_inout<sc_dt::sc_bit> in2;
  sc_out<sc_dt::sc_bit> out1;
  sc_out<sc_dt::sc_bit> out2;

  SC_CTOR(TWO_TO_TWO) {
    // declare concurrency
    SC_THREAD(transition);
    sensitive << in1 << in2;
  }

  // declare 1 and 0 bits
  sc_dt::sc_bit flag;
  sc_dt::sc_bit reset;
  void transition() {
    flag  = 1;
    reset = 0;
    while (true) {
      if (in1->read() == 1 && in2->read() == 1) {
        // reset inputs
        in1->write(reset);
        in2->write(reset);
        // write to outputs
        out1->write(flag);
        out2->write(flag);
      }
      wait();
    }
  }
};

SC_MODULE(TWO_TO_THREE) {
  // declare ports
  sc_inout<sc_dt::sc_bit> in1;
  sc_inout<sc_dt::sc_bit> in2;
  sc_out<sc_dt::sc_bit> out1;
  sc_out<sc_dt::sc_bit> out2;
  sc_out<sc_dt::sc_bit> out3;
  SC_CTOR(TWO_TO_THREE) {
    SC_THREAD(transition);
    sensitive << in1 << in2;
    // dont_initialize();
  }

  // declare 0 and 1 bits
  sc_dt::sc_bit flag;
  sc_dt::sc_bit reset;

  void transition() {
    flag = 0;
    while (true) {
      if (in1->read() == 1 && in2->read() == 1) {
        flag = 1;
        // reset inputs
        in1->write(reset);
        in2->write(reset);
        // write to outputs
        out1->write(flag);
        out2->write(flag);
        out3->write(flag);
      }
      wait();
    }
  }
};

int sc_main(int, char*[]) { 
  
  // delcare nodes from graph
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> A;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> B;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> C;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> D;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> E;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> F;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> G;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> H;
  sc_signal<sc_dt::sc_bit, SC_MANY_WRITERS> SO;
  
  // declare transitions from graph
  ONE_TO_ONE trans1("trans1");
  TWO_TO_ONE trans3("trans3");
  TWO_TO_ONE trans4("trans4");
  TWO_TO_THREE trans5("trans5");
  TWO_TO_TWO trans6("trans6");
  TWO_TO_ONE trans7("trans7");
  ONE_TO_ONE trans8("trans8");
  TWO_TO_TWO trans9("trans9");
  TWO_TO_THREE trans10("trans10");
  TWO_TO_TWO trans11("trans11");
  TWO_TO_TWO trans12("trans12");
  
  // connect nodes and transitions
  // --- Transition 1 ---
  trans1.in1(SO);
  trans1.out1(A);
  
  // --- Transition 3 ---
  trans3.in1(A);
  trans3.in2(B);
  trans3.out1(C);
  
  // -- Transition 4 ---
  trans4.in1(A);
  trans4.in2(B);
  trans4.out1(D);
  
  // --- Transition 5 ---
  return 0; 
  
  }