module bit_adder (
    input a, b, cin,
    output q, cout
);
    wire a_and_b, a_and_c, b_and_c;
    wire ab_or_ac;
    wire a_xor_b;

    and ab(a_and_b, a, b);
    and ac(a_and_c, a, c);
    and bc(b_and_c, b, c);
    or or1(ab_or_ac, a_and_b, a_and_c);
    or carry(cout, ab_or_ac, b_and_c);

    xor xor1(a_xor_b, a, b);
    xor ans(q, a_xor_b, cin);
endmodule