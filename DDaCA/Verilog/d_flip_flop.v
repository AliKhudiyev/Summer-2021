module dff (
    input clk,
    input [3:0] d,
    output reg [3:0] q
);
    always @(posedge clk) begin
        q <= d;
    end
endmodule