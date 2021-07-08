module d_latch (
    input d, e,
    output q
);
    wire d_n, and1, and2, q_n;

    not d_inv(d_n, d);
    and and_upper(and1, d_n, e);
    and and_lower(and2, d, e);
    nor nor_upper(q, and1, q_n);
    nor nor_lower(q_n, and2, q);
endmodule