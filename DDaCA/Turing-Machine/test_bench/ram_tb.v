// `include "../ram.v"

module ram_tb ();
    parameter DATA_WIDTH = 32;
    parameter ADDR_SPACE = 14;

    reg clk = 0;
    reg [ADDR_SPACE-1:0] addr = 0;
    reg [DATA_WIDTH-1:0] d;
    wire [DATA_WIDTH-1:0] out;
    reg we = 1;

    always #5 clk = !clk;

    ram #(DATA_WIDTH, ADDR_SPACE) mem(addr, d, clk, we, out);

    initial begin
        $dumpfile("ram.vcd");
        $dumpvars(0, ram_tb);

        d = 16'b11;
        #10;
        d = 16'b1001;
        #3;
        d = 16'b1000_0000_0011_0001; we = 0;
        #1;
        d = 16'b110;
        #4;
        d = 16'b11; we = 1;
        #8;
        d = 16'b1000;
        #6;
        d = 16'hFF_FF;
        #7;
        d = 16'b0; we = 0;
        #7;
        d = 16'b11;
        #20;
        $finish;
    end

    initial
     $monitor("At time %t, q = %h",
              $time, out);
endmodule