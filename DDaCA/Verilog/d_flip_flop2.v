`include "d_latch.v"

module d_flip_flop2 (
    input clk,
    input d,
    output q
);
    wire nclk, tmp;

    not clk_inv(nclk, clk);
    d_latch master(nclk, d, tmp);
    d_latch slave(clk, tmp, q);
endmodule