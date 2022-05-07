`timescale 1ns/1ps

module tenary_adder(clk, rst_n, fire, done, w11, w12, w13, w21, w22, w23, w31, w32, w33, x11, x12, x13, x21, x22, x23, x31, x32, x33, partial_result,
                    r, b, data_out
                   );

parameter [8:0] INPUT_SIZE = 16;
parameter [4:0] TI = 3;
parameter [3:0] ADDR_BITS = 4;
parameter [10:0] INPUT_CHANNEL = 3;

localparam ITERATION_TIMES = INPUT_CHANNEL/TI;

input clk;
input rst_n;
input fire;

input w11;
input w12;
input w13;
input w21;
input w22;
input w23;
input w31;
input w32;
input w33;

input signed [5:0] x11; 
input signed [5:0] x12; 
input signed [5:0] x13; 
input signed [5:0] x21; 
input signed [5:0] x22; 
input signed [5:0] x23; 
input signed [5:0] x31; 
input signed [5:0] x32; 
input signed [5:0] x33; 
output partial_result;
output reg done;
//input signed [12:0] data_in;
input signed [15:0] r;
input signed [15:0] b;
output signed [5:0] data_out;

wire signed [5:0] sign_inv_1;
wire signed [5:0] sign_inv_2;
wire signed [5:0] sign_inv_3;
wire signed [5:0] sign_inv_4;
wire signed [5:0] sign_inv_5;
wire signed [5:0] sign_inv_6;
wire signed [5:0] sign_inv_7;
wire signed [5:0] sign_inv_8;
wire signed [5:0] sign_inv_9;

wire signed [7:0] partial_sum_1;
wire signed [7:0] partial_sum_2;
wire signed [7:0] partial_sum_3;
wire signed [9:0] partial_sum;

reg signed [7:0] partial_result_1;
reg signed [7:0] partial_result_2;
reg signed [7:0] partial_result_3;
reg signed [9:0] partial_result;
reg signed [12:0] line_buffer [INPUT_SIZE-1:0];
reg signed [5:0] data_out;
reg [ADDR_BITS-1:0] pointer;
reg [4:0] counter;
reg [7:0] over_counter;

assign sign_inv_1 = w11 ? x11 : (~x11 + 1'b1);
assign sign_inv_2 = w12 ? x12 : (~x12 + 1'b1);
assign sign_inv_3 = w13 ? x13 : (~x13 + 1'b1);
assign sign_inv_4 = w21 ? x21 : (~x21 + 1'b1);
assign sign_inv_5 = w22 ? x22 : (~x22 + 1'b1);
assign sign_inv_6 = w23 ? x23 : (~x23 + 1'b1);
assign sign_inv_7 = w31 ? x31 : (~x31 + 1'b1);
assign sign_inv_8 = w32 ? x32 : (~x32 + 1'b1);
assign sign_inv_9 = w33 ? x33 : (~x33 + 1'b1);

assign partial_sum_1 = sign_inv_1 + sign_inv_2 + sign_inv_3;
assign partial_sum_2 = sign_inv_4 + sign_inv_5 + sign_inv_6;
assign partial_sum_3 = sign_inv_7 + sign_inv_8 + sign_inv_9;

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     begin
       partial_result_1 <= 8'd0; 
       partial_result_2 <= 8'd0; 
       partial_result_3 <= 8'd0; 
     end
   else if (fire)
     begin
       partial_result_1 <= partial_sum_1;
       partial_result_2 <= partial_sum_2;
       partial_result_3 <= partial_sum_3;
     end
end

assign partial_sum = partial_result_1 + partial_result_2 + partial_result_3;

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
      partial_result <= 10'd0;
   else if(fire)
      partial_result <= partial_sum;
end

reg start;
reg over;

always@(posedge clk or negedge rst_n)
begin
    if(!rst_n)
    begin
     line_buffer[0] <= 13'd0;
     line_buffer[1] <= 13'd0;
     line_buffer[2] <= 13'd0;
     line_buffer[3] <= 13'd0;
     line_buffer[4] <= 13'd0;
     line_buffer[5] <= 13'd0;
     line_buffer[6] <= 13'd0;
     line_buffer[7] <= 13'd0;
     line_buffer[8] <= 13'd0;
     line_buffer[9] <= 13'd0;
     line_buffer[10] <= 13'd0;
     line_buffer[11] <= 13'd0;
     line_buffer[12] <= 13'd0;
     line_buffer[13] <= 13'd0;
     line_buffer[14] <= 13'd0;
     line_buffer[15] <= 13'd0;
    end
    else if(start)
     line_buffer[pointer] <= line_buffer[pointer] + partial_result;
end 


always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     pointer <= 0;
   else if((pointer == INPUT_SIZE-1) & (counter == TI+1) & fire)
     pointer <= 0;
   else if((counter == TI+1) & fire)
     pointer <= pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     counter <= 5'd0;
   else if((counter == TI+2) & fire)
     counter <= 5'd3;
   else if(fire)
     counter <= counter + 1'b1;
end

//to handle the pipeline delay
always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     start <= 1'b0;
   else if((counter == 3'd1) & fire)
     start <= 1'b1;
end

//indicate the partial result in the linebuffer has complete the computation 
always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     over <= 1'b0;
   else if((over_counter == 2*ITERATION_TIMES-1) & fire)
     over <= 1'b1;
   else if((over_counter == 2*ITERATION_TIMES) & fire)
     over <= 1'b0;
end

//counter++ both when pointer achieve 0 and the last address
always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
      over_counter <= 8'd0;
   else if(((pointer=='b0) | (pointer == INPUT_SIZE-1)) & (counter==TI) & fire)
      over_counter <= over_counter + 1'b1;
   else if((over_counter == 2*ITERATION_TIMES) & fire)
      over_counter <= 8'd0;
end







reg signed[12:0] data_in;
wire signed [28:0] mult_result;
wire signed [29:0] add_result;

assign mult_result = data_in * r;
assign add_result = mult_result[28] ? (mult_result >>> 3 + b) : mult_result + b;

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
      data_in <= 13'd0;
   else if(over & (counter==TI+2) & fire)
      data_in <= line_buffer[pointer-1];
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
      data_out <= 6'd0;
   else if((!add_result[29]) & (|add_result[28:6]) & fire & over)
      data_out <= 6'd31;
   else if((add_result[29]) & (!(&add_result[28:6])) & fire & over)
      data_out <= 6'b100000;
   else if (fire & over)
      data_out <= add_result[5:0];
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     done <= 1'b0;
   else if(fire & over)
     done <= 1'b1;
   else 
     done <= 1'b0;
end

endmodule







