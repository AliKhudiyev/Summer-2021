module bus_selector 
    #(parameter width = 32)
(
    input sel,
    input [width-1:0] data,
    output [width-1:0] result
);
    assign result = sel ? data : 32'b0;
endmodule