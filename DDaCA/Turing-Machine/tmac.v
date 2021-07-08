`include "memory.v"
`include "ram.v"
`include "register.v"
`include "modules/adder.v"
`include "modules/mux.v"

module tmac #(
    parameter DATA_WIDTH = 32,
    parameter MEM_SIZE = 14;
) (
    input rst
);
    reg clk = 1'b1;                                             // clock
    wire [DATA_WIDTH-1:0] inst;                                 // instruction
    wire sym;                                                   // symbol (0 or 1)
    wire new_sym;                                               // a symbol to be written (0 or 1)
    wire [MEM_SIZE-1:0] ip = 0;                                 // Instruction Pointer (current state)
    wire [MEM_SIZE-1:0] inp = 0;                                // INput Pointer
    reg we = 0;                                                 // Write Enable
    wire [MEM_SIZE-1:0] one = 1, minus_one = { 14{1} }, tmp;
    wire flag_of;                                               // over-flow bit: unused

    register ip(ip, clk, 0, ip);
    register inp(inp, clk, 0, inp);
    memory inst_mem(ip, 0, clk, 0, inst);
    memory input_mem(inp, new_sym, clk, we, sym);
    mux plus_minus(one, minus_one, sym, tmp);
    adder movLR(tmp, inp, 0, inp, flag_of);

    // always #10 clk = !clk;
    always @(*) begin
        not(clk, clk);
        #10;
    end
endmodule