`include "d_flip_flop.v"

module register #(
    parameter DATA_WIDTH = 16
) (
    input [DATA_WIDTH-1:0] d,
    input e,
    output [DATA_WIDTH-1:0] q
);
    generate
        genvar i;

        for (i = 0; i < DATA_WIDTH; i = i + 1) begin
            d_flip_flop dff(d[i], e, q[i]);
        end
    endgenerate
endmodule