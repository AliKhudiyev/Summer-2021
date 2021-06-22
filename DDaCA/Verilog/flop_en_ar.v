module flop_en_ar (
    input clk,
    input reset,
    input en,
    input [3:0] d,
    output reg [3:0] q
);
    always @(posedge clk, negedge reset) begin
        if (reset == 0) q <= d;
        else if (en) q <= d;
    end
endmodule