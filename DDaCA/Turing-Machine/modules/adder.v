module adder (
    input a, b, cin,
    output q, cout
);
    wire a_xor_b;
    wire a_and_b, a_and_c, b_and_c;
    wire or1;

    xor xor1(a_xor_b, a, b);
    xor xor2(q, a_xor_b, cin);

    and and_1(a_and_b, a, b);
    and and_2(a_and_c, a, c);
    and and_3(b_and_c, b, c);
    or or_1(or1, a_and_b, a_and_c);
    or or_2(cout, or1, b_and_c);
endmodule