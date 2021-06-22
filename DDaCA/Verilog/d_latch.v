module d_latch (
    input en,
    input d,
    output q
);
    wire nd;
    wire and1, and2;
    wire nor1, nor2;
    
    not d_inv(nd, d);
    and and_1(and1, nd, en);
    and and_2(and2, d, en);
    nor nor_1(nor1, and1, nor2);
    nor nor_2(nor2, and2, nor1);

    assign q = nor1;
endmodule