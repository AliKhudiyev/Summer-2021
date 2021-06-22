module d_flip_flop2_tb ();
    reg clk = 0, d;
    wire q;

    always #5 clk = !clk;

    d_flip_flop2 dut(clk, d, q);

    initial begin
        $dumpfile("d_flip_flop2.vcd");
        $dumpvars(0, d_flip_flop2_tb);

        d = 0;
        #10;
        d = 1;
        #3;
        d = 0;
        #1;
        d = 1;
        #4;
        d = 0;
        #8;
        d = 1;
        #6;
        d = 0;
        #7;
        d = 1;
        #7;
        d = 0;
        #20;
        $finish;
    end

    initial
     $monitor("At time %t, q = %h",
              $time, q);
endmodule