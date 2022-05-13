`timescale 1ns/1ps

module tenary_adder_testbench();

reg clk;
reg rst_n;
reg fire;

reg w11;
reg w12;
reg w13;
reg w21;
reg w22;
reg w23;
reg w31;
reg w32;
reg w33;

reg [5:0] x11;
reg [5:0] x12;
reg [5:0] x13;
reg [5:0] x21;
reg [5:0] x22;
reg [5:0] x23;
reg [5:0] x31;
reg [5:0] x32;
reg [5:0] x33;

wire [9:0] partial_result;
wire done;

//reg [12:0] data_in;
reg [15:0] r;
reg [15:0] b;
wire [5:0] data_out;


tenary_adder tenary_adder(
                          .clk(clk),
                          .rst_n(rst_n),
                          .fire(fire),
                          .done(done),
                          .w11(w11),
                          .w12(w12),
                          .w13(w13),
                          .w21(w21),
                          .w22(w22),
                          .w23(w23),
                          .w31(w31),
                          .w32(w32),
                          .w33(w33),
                          .x11(x11),
                          .x12(x12),
                          .x13(x13),
                          .x21(x21),
                          .x22(x22),
                          .x23(x23),
                          .x31(x31),
                          .x32(x32),
                          .x33(x33),
                          .partial_result(partial_result),
//                          .data_in(data_in),
                          .data_out(data_out),
                          .r(r),
                          .b(b)
                         );

always #20 clk = ~clk;

initial
begin
clk = 1'b0;
rst_n = 1'b1;
fire = 1'b0;

x11 = 6'd0;
x12 = 6'd0;
x13 = 6'd0;
x21 = 6'd0;
x22 = 6'd0;
x23 = 6'd0;
x31 = 6'd0;
x32 = 6'd0;
x33 = 6'd0;

w11 = 1'b0;
w12 = 1'b0;
w13 = 1'b0;
w21 = 1'b0;
w22 = 1'b0;
w23 = 1'b0;
w31 = 1'b0;
w32 = 1'b0;
w33 = 1'b0;

//data_in = 13'd0;
r = 16'd0;
b = 16'd0;


#5 rst_n = 1'b0;
#5 rst_n = 1'b1;

#40 
fire = 1'b1;

x11 = 6'd1;
x12 = 6'd1;
x13 = 6'd1;
x21 = 6'd1;
x22 = 6'd1;
x23 = 6'd1;
x31 = 6'd1;
x32 = 6'd1;
x33 = 6'd1;

w11 = 1'b0;
w12 = 1'b0;
w13 = 1'b0;
w21 = 1'b0;
w22 = 1'b0;
w23 = 1'b0;
w31 = 1'b0;
w32 = 1'b0;
w33 = 1'b0;

//data_in = 13'd1;
r = 16'hfff8;
b = 16'hffff;

#40
x11 = 6'd2;
x12 = 6'd2;
x13 = 6'd2;
x21 = 6'd2;
x22 = 6'd2;
x23 = 6'd2;
x31 = 6'd2;
x32 = 6'd2;
x33 = 6'd2;

#40
x11 = 6'd3;
x12 = 6'd3;
x13 = 6'd3;
x21 = 6'd3;
x22 = 6'd3;
x23 = 6'd3;
x31 = 6'd3;
x32 = 6'd3;
x33 = 6'd3;


//cycle2

#40
x11 = 6'd4;
x12 = 6'd4;
x13 = 6'd4;
x21 = 6'd4;
x22 = 6'd4;
x23 = 6'd4;
x31 = 6'd4;
x32 = 6'd4;
x33 = 6'd4;

#40
x11 = 6'd5;
x12 = 6'd5;
x13 = 6'd5;
x21 = 6'd5;
x22 = 6'd5;
x23 = 6'd5;
x31 = 6'd5;
x32 = 6'd5;
x33 = 6'd5;

#40
x11 = 6'd6;
x12 = 6'd6;
x13 = 6'd6;
x21 = 6'd6;
x22 = 6'd6;
x23 = 6'd6;
x31 = 6'd6;
x32 = 6'd6;
x33 = 6'd6;

//cycle3

#40
x11 = 6'd7;
x12 = 6'd7;
x13 = 6'd7;
x21 = 6'd7;
x22 = 6'd7;
x23 = 6'd7;
x31 = 6'd7;
x32 = 6'd7;
x33 = 6'd7;

#40
x11 = 6'd8;
x12 = 6'd8;
x13 = 6'd8;
x21 = 6'd8;
x22 = 6'd8;
x23 = 6'd8;
x31 = 6'd8;
x32 = 6'd8;
x33 = 6'd8;

#40
x11 = 6'd9;
x12 = 6'd9;
x13 = 6'd9;
x21 = 6'd9;
x22 = 6'd9;
x23 = 6'd9;
x31 = 6'd9;
x32 = 6'd9;
x33 = 6'd9;


#4000 $stop;

end
endmodule
 


























