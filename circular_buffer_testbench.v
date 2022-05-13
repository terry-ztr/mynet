`timescale 1ns/1ps

module circular_buffer_testbench();

reg clk;
reg rst_n;
reg[23:0] data_in;
reg [8:0] weight_in;
reg buffer_data_fire;
reg buffer_weight_fire;
reg buffer_done;

wire [5:0] x11;
wire [5:0] x12;
wire [5:0] x13;
wire [5:0] x21;
wire [5:0] x22;
wire [5:0] x23;
wire [5:0] x31;
wire [5:0] x32;
wire [5:0] x33;
wire w11;
wire w12;
wire w13;
wire w21;
wire w22;
wire w23;
wire w31;
wire w32;
wire w33;


circular_buffer circular_buffer(
                                .clk(clk),
                                .rst_n(rst_n),
                                .data_in(data_in),
                                .weight_in(weight_in),
                                .buffer_weight_fire(buffer_weight_fire),
                                .buffer_data_fire(buffer_data_fire),
                                .buffer_done(buffer_done),
                                .x11(x11),
                                .x12(x12),
                                .x13(x13),
                                .x21(x21),
                                .x22(x22),
                                .x23(x23),
                                .x31(x31),
                                .x32(x32),
                                .x33(x33),
                                .w11(w11),
                                .w12(w12),
                                .w13(w13),
                                .w21(w21),
                                .w22(w22),
                                .w23(w23),
                                .w31(w31),
                                .w32(w32),
                                .w33(w33)
                               );

always #20 clk = ~clk;

initial
begin
clk = 1'b0;
rst_n = 1'b1;
data_in = 24'd0;
weight_in = 9'd0;
buffer_data_fire = 1'b0;
buffer_weight_fire = 1'b0;
buffer_done = 1'b0;

#5 rst_n = 1'b0;
#5 rst_n = 1'b1;

//transfer weight
#40
buffer_weight_fire = 1'b1;
weight_in = 9'h155;
#40
weight_in = 9'h0aa;
#40
weight_in = 9'h1ff;
#40
weight_in = 9'd0;


//transfer data
#40
buffer_data_fire = 1'b1;
data_in = 24'h103081;
#1280
data_in = 24'h206102;
#1280
data_in = 24'h309183;
#1280
buffer_done = 1'b1;
data_in = 24'h406144;
weight_in = 9'h1ff;
#1280

#2000 $stop;


end

endmodule













