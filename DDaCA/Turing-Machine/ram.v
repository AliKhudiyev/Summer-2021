module ram #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_SPACE = 14
) (
    input [ADDR_SPACE-1:0] addr,
    input [DATA_WIDTH-1:0] data,
    input clk,
    input we,
    output reg [DATA_WIDTH-1:0] out
);
    reg [DATA_WIDTH-1:0] mem [2**ADDR_SPACE-1:0];

    always @(posedge clk) begin
        if (we) begin
            mem[addr] <= data;
        end
        out <= mem[addr];
    end
endmodule