module memory #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_SPACE = 14
) (
    input [ADDR_SPACE:0] addr,
    input [DATA_WIDTH-1:0] data,
    input clk,
    input we,
    output reg [DATA_WIDTH/2-1:0] out
);
    reg [DATA_WIDTH-1:0] mem [2**ADDR_SPACE-1:0];

    always @(posedge clk) begin
        if (we) begin
            mem[addr[ADDR_SPACE:1]] <= data;
        end

        if (addr[0]) begin
            out <= { mem[addr[ADDR_SPACE-1:0]][DATA_WIDTH-1:DATA_WIDTH-ADDR_SPACE], 
                    mem[addr[ADDR_SPACE-1:0]][DATA_WIDTH-2*ADDR_SPACE-1], 
                    mem[addr[ADDR_SPACE-1:0]][DATA_WIDTH-2*ADDR_SPACE-2-1] };
        end
        else begin
            out <= { mem[addr[ADDR_SPACE-1:0]][DATA_WIDTH-ADDR_SPACE-1:DATA_WIDTH-2*ADDR_SPACE], 
                    mem[addr[ADDR_SPACE-1:0]][DATA_WIDTH-2*ADDR_SPACE-2], 
                    mem[addr[ADDR_SPACE-1:0]][DATA_WIDTH-2*ADDR_SPACE-2-2] };
        end
    end
endmodule