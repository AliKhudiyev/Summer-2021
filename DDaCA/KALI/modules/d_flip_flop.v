`include "d_latch.v"

module d_flip_flop (
    input d, clk,
    output q
);
    wire clk_n, q_tmp;

    not clk_inv(clk_n, clk);
    d_latch master(d, clk_n, q_tmp);
    d_latch slave(q_tmp, clk, q);
endmodule