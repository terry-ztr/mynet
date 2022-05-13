`timescale 1ns/1ps

module conv_testbench();

reg clk;
reg rst_n;
reg[17:0] data_in;
reg [8:0] weight_in;
reg buffer_data_fire;
reg buffer_weight_fire;
reg compute_fire;
reg [15:0]r;
reg [15:0]b;

wire compute_done;
wire [5:0] data_out;


conv conv(
          .clk(clk),
          .rst_n(rst_n),
          .data_in(data_in),
          .r(r),
          .b(b),
          .weight_in(weight_in),
          .buffer_weight_fire(buffer_weight_fire),
          .buffer_data_fire(buffer_data_fire),
          .compute_fire(compute_fire),
          .compute_done(compute_done),
          .data_out(data_out)
                               );

always #20 clk = ~clk;

initial
begin
clk = 1'b0;
rst_n = 1'b1;
data_in = 18'd0;
weight_in = 9'd0;
buffer_data_fire = 1'b0;
buffer_weight_fire = 1'b0;
compute_fire = 1'b0;
r = 16'd1;
b = 16'd0;

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
data_in = 18'h03081;
#640
data_in = 18'h06102;
#640
data_in = 18'h09183;
#640
compute_fire = 1'b1;
data_in = 18'h06144;
weight_in = 9'h1ff;
#640

#2000 $stop;


end

endmodule













