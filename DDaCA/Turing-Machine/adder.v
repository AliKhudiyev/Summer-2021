`include "modules/bit_adder.v"

module adder #(
    parameter DATA_WIDTH = 14
) (
    input [DATA_WIDTH-1:0] a, b,
    input cin,
    output [DATA_WIDTH-1:0] q,
    output cout
);
    generate
        genvar i;
        wire [DATA_WIDTH:0] carries;
        assign carries[0] = 1'b0;

        for (i = 0; i < DATA_WIDTH; i = i + 1) begin
            bit_adder add_1(a[i], b[i], carries[i], q[i], carries[i+1]);
        end
    endgenerate

    assign cout = carries[DATA_WIDTH];
endmodule