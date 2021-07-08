`include "adder.v"

module full_adder #(
    parameter DATA_WIDTH = 16
) (
    input [DATA_WIDTH-1:0] a, b,
    input cin,
    output [DATA_WIDTH-1:0] q,
    output cout
);
    wire tmp_cout = cin;
    generate
        genvar i;
        
        for (i = 0; i < DATA_WIDTH; i = i + 1) begin
            adder add_bit(a[i], b[i], tmp_cout, q[i], tmp_cout);
        end
    endgenerate

endmodule