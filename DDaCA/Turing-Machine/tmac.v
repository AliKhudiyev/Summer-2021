`include "memory.v"
`include "ram.v"
`include "register.v"
`include "adder.v"
`include "modules/mux.v"

module tmac #(
    parameter DATA_WIDTH = 32,
    parameter MEM_SIZE = 14,
    parameter INPT_MEM_SIZE = 14
) (
    input clk,
    input rst
);
    // reg clk = 1'b1;                                             // clock
    wire [DATA_WIDTH/2-1:0] inst;                               // instruction or Turing tuple
    wire sym;                                                   // symbol (0 or 1)
    wire new_sym;                                               // a symbol to be written (0 or 1)
    reg we = 0;                                                 // Write Enable
    reg [MEM_SIZE-1:0] one = 1, minus_one = 14'b1;
    wire flag_of;                                               // over-flow bit: unused
    wire [MEM_SIZE-1:0] inst_addr, inpt_addr;
    wire [MEM_SIZE-1:0] next_inpt_addr, mux_out;

    // always #5 clk = !clk;
    // always @(*) begin
    //     not(clk, clk);   #5;
    // end

    memory #(DATA_WIDTH, MEM_SIZE) inst_mem({ inst_addr, sym }, 32'b0, clk, we, inst);
    ram #(1, INPT_MEM_SIZE) inpt_mem(inpt_addr, inst[14], clk, we, sym);
    register sreg(inst[DATA_WIDTH/2-1:DATA_WIDTH/2-MEM_SIZE], clk, rst, inst_addr);
    register preg(next_inpt_addr, clk, rst, inpt_addr);
    adder tape_shifter(mux_out, inpt_addr, 1'b0, next_inpt_addr, flag_of);
    mux selector(one, minus_one, inst[0], mux_out);
endmodule