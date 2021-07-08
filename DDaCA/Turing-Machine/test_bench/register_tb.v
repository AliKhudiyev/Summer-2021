// `include "../register.v"

module register_tb ();
    parameter DATA_WIDTH = 32;

    reg clk = 0;
    reg [DATA_WIDTH-1:0] d;
    wire [DATA_WIDTH-1:0] q;

    always #5 clk = !clk;

    register #(DATA_WIDTH) dut(d, clk, 1'b0, q);

    initial begin
        $dumpfile("register.vcd");
        $dumpvars(0, register_tb);

        d = 16'b11;
        #10;
        d = 16'b1001;
        #3;
        d = 16'b1000_0000_0011_0001;
        #1;
        d = 16'b110;
        #4;
        d = 16'b11;
        #8;
        d = 16'b1000;
        #6;
        d = 16'hFF_FF;
        #7;
        d = 16'b0;
        #7;
        d = 16'b11;
        #20;
        $finish;
    end

    initial
     $monitor("At time %t, q = %h",
              $time, q);
endmodule