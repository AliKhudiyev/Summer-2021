module example (
    input a,
    input b,
    input c,
    output y
);
    assign y = a & (b | c);
    always @*
        $display("%b",y);
endmodule