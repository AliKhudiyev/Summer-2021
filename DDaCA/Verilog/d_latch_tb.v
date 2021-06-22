module d_latch_tb ();
    reg en, d;
    wire q;

    // instantiate device under test
    d_latch dut(en, d, q);

    // apply inputs one at a time
    initial begin
        $dumpfile("d_latch.vcd");
        $dumpvars(0, d_latch_tb);

        en = 0; d = 0;  #10; // q = x
        d = 1;          #10; // q = x
        en = 1; d = 0;  #10; // q = 0
        d = 1;          #10; // q = 1
        en = 0; d = 0;  #10; // q = 1
        d = 1;          #10; // q = 1
        d = 0;          #10; // q = 1
        en = 1;         #10; // q = 0
        d = 1;          #10; // q = 1
        en = 0;         #10; // q = 1
        d = 0;          #10; // q = 1
        $finish;
    end

    initial
     $monitor("At time %t, value = %h (%0d)",
              $time, q, q);
endmodule