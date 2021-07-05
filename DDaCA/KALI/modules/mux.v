module mux #(
    parameter DATA_WIDTH = 16
) (
    input [DATA_WIDTH-1:0] d0, d1,
    input s,
    output [DATA_WIDTH-1:0] q
);
    wire s_n;
    wire [DATA_WIDTH-1:0] q0, q1;

    not s_inv(s_n, s);

    generate
        genvar i;

        for (i = 0; i < DATA_WIDTH; i = i + 1) begin
            and and1(q1[i], s, d1[i]);
            and and2(q0[i], s, d0[i]);
            or or1(q[i], q0[i], q1[i]);
        end
    endgenerate

endmodule