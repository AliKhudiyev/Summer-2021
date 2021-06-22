module divby3fsm (
    input clk,
    input res,
    output q
);
    reg [1:0] state, next_state;
    parameter S0 = 2'b00;
    parameter S1 = 2'b01;
    parameter S2 = 2'b01;

    always @(posedge clk, posedge res) begin
        if(res) state <= S0;
        else state <= next_state;
    end

    always @(*) begin
        case (state)
            S0: next_state = S0;
            S1: next_state = S1;
            S2: next_state = S2;
            default: next_state = S0;
        endcase
    end

    assign q = (state == S0);
endmodule