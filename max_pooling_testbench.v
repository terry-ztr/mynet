`timescale 1ns/1ps

module max_pooling_testbench();

reg clk;
reg rst_n;
reg fire;
reg [5:0] data_in;
reg row;
wire [5:0] data_out;
wire done;

max_pooling max_pooling(
                        .clk(clk),
                        .rst_n(rst_n),
                        .fire(fire),
                        .data_in(data_in),
                        .row(row),
                        .data_out(data_out),
                        .done(done)
                       );

always #20 clk = ~clk;

initial
begin
clk = 1'b0;
rst_n = 1'b1;
data_in = 6'd0;
row = 1'b0;
fire = 1'b0;

#5 rst_n = 1'b0;
#5 rst_n = 1'b1;
#40 fire = 1'b1;

data_in = 6'd1;
#40
data_in = 6'd2;
#40 
data_in = 6'd3;
#40
data_in = 6'd4;
#40
data_in = 6'd5;
#40
data_in = 6'd6;
#40 
data_in = 6'd7;
#40
data_in = 6'd8;
#40
data_in = 6'd9;
#40
data_in = 6'd10;
#40 
data_in = 6'd11;
#40
data_in = 6'd12;
#40
data_in = 6'd13;
#40
data_in = 6'd14;
#40 
data_in = 6'd15;
#40
data_in = 6'd16;
#40

row = 1'b1;

data_in = 6'd21;
#40
data_in = 6'd22;
#40 
data_in = 6'd23;
#40
data_in = 6'd24;
#40
data_in = 6'd25;
#40
data_in = 6'd26;
#40 
data_in = 6'd27;
#40
data_in = 6'd28;
#40
data_in = 6'd9;
#40
data_in = 6'd10;
#40 
data_in = 6'd1;
#40
data_in = 6'd2;
#40
data_in = 6'd3;
#40
data_in = 6'd4;
#40 
data_in = 6'd5;
#40
data_in = 6'd6;
#40


#400 $stop;

end

endmodule


































