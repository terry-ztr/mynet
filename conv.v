`timescale 1ns/1ps

module conv(clk, rst_n, data_in, weight_in, buffer_data_fire, buffer_weight_fire, compute_fire, r, b, compute_done, data_out 
           );

parameter [8:0] INPUT_SIZE = 16;
parameter [4:0] TI = 3;
parameter [3:0] ADDR_BITS = 4;
parameter [10:0] INPUT_CHANNEL = 3;

localparam ITERATION_TIMES = INPUT_CHANNEL/TI;
localparam INPUT_DEPTH = 4*ITERATION_TIMES*INPUT_SIZE;


input clk;
input rst_n;
input [6*TI-1:0] data_in;
input [8:0] weight_in;
input buffer_data_fire;
input buffer_weight_fire;
input compute_fire;
input signed [15:0] r;
input signed [15:0] b;

//output partial_result;
output reg compute_done;
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
reg [4:0] compute_counter;
reg [7:0] over_compute_counter;

wire w11;
wire w12;
wire w13;
wire w21;
wire w22;
wire w23;
wire w31;
wire w32;
wire w33;

reg signed [5:0] x11;
reg signed [5:0] x12;
reg signed [5:0] x13;
reg signed [5:0] x21;
reg signed [5:0] x22;
reg signed [5:0] x23;
reg signed [5:0] x31;
reg signed [5:0] x32;
reg signed [5:0] x33;

reg [8:0] weight_buffer [TI-1:0];
reg [6*TI-1:0] data_buffer [INPUT_DEPTH-1:0];
reg [ADDR_BITS-1:0] data_out_pointer;
reg [ADDR_BITS+7:0] data_in_pointer;
reg [4:0] weight_in_pointer;
reg [4:0] buffer_out_counter;
reg [17:0] x11_reg;
reg [17:0] x12_reg;
reg [17:0] x13_reg;
reg [17:0] x21_reg;
reg [17:0] x22_reg;
reg [17:0] x23_reg;
reg [17:0] x31_reg;
reg [17:0] x32_reg;
reg [17:0] x33_reg;
reg [8:0] kernel_reg;


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


//communication
//data in buffer
always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     buffer_out_counter <= 5'd0;
   else if(buffer_out_counter == TI)
     buffer_out_counter <= 5'd1;
   else if(compute_fire)
     buffer_out_counter <= buffer_out_counter + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     data_out_pointer <= 0;
   else if (buffer_out_counter == TI)
     data_out_pointer <= data_out_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     data_in_pointer <= 0;
   else if(buffer_data_fire)
     data_in_pointer <= data_in_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
   begin
      data_buffer[0] <= 18'd0;
      data_buffer[1] <= 18'd0;
      data_buffer[2] <= 18'd0;
      data_buffer[3] <= 18'd0;
      data_buffer[4] <= 18'd0;
      data_buffer[5] <= 18'd0;
      data_buffer[6] <= 18'd0;
      data_buffer[7] <= 18'd0;
      data_buffer[8] <= 18'd0;
      data_buffer[9] <= 18'd0;
      data_buffer[10] <= 18'd0;
      data_buffer[11] <= 18'd0;
      data_buffer[12] <= 18'd0;
      data_buffer[13] <= 18'd0;
      data_buffer[14] <= 18'd0;
      data_buffer[15] <= 18'd0;
   end
   else if (buffer_data_fire)
      data_buffer[data_in_pointer] <= data_in;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
    begin
     x11_reg <= 0;
     x12_reg <= 0;
     x13_reg <= 0;
     x21_reg <= 0;
     x22_reg <= 0;
     x23_reg <= 0;
     x31_reg <= 0;
     x32_reg <= 0;
     x33_reg <= 0;
    end
   else if(compute_fire)
    begin
     x11_reg <= data_buffer[data_out_pointer];
     x12_reg <= data_buffer[data_out_pointer+1];
     x13_reg <= data_buffer[data_out_pointer+2];
     x21_reg <= data_buffer[data_out_pointer+INPUT_SIZE];
     x22_reg <= data_buffer[data_out_pointer+INPUT_SIZE+1];
     x23_reg <= data_buffer[data_out_pointer+INPUT_SIZE+2];
     x31_reg <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE];
     x32_reg <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+1];
     x33_reg <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+2];
    end
end

//weight in buffer
always@(compute_fire or data_out_pointer or buffer_out_counter)
begin
   if(compute_fire)
     case(buffer_out_counter)
        5'd1 :
              begin
                x11 = x11_reg[5:0];
                x12 = x12_reg[5:0];
                x13 = x13_reg[5:0];
                x21 = x21_reg[5:0];
                x22 = x22_reg[5:0];
                x23 = x23_reg[5:0];
                x31 = x31_reg[5:0];
                x32 = x32_reg[5:0];
                x33 = x33_reg[5:0];
              end
        5'd2 :
              begin
                x11 = x11_reg[11:6];
                x12 = x12_reg[11:6];
                x13 = x13_reg[11:6];
                x21 = x21_reg[11:6];
                x22 = x22_reg[11:6];
                x23 = x23_reg[11:6];
                x31 = x31_reg[11:6];
                x32 = x32_reg[11:6];
                x33 = x33_reg[11:6];
              end
        5'd3 :
              begin
                x11 = x11_reg[17:12];
                x12 = x12_reg[17:12];
                x13 = x13_reg[17:12];
                x21 = x21_reg[17:12];
                x22 = x22_reg[17:12];
                x23 = x23_reg[17:12];
                x31 = x31_reg[17:12];
                x32 = x32_reg[17:12];
                x33 = x33_reg[17:12];
              end
        default
              begin
                x11 = 6'd0;
                x12 = 6'd0;
                x13 = 6'd0;
                x21 = 6'd0;
                x22 = 6'd0;
                x23 = 6'd0;
                x31 = 6'd0;
                x32 = 6'd0;
                x33 = 6'd0;
              end
     endcase
   else
     begin
        x11 = 6'd0;
        x12 = 6'd0;
        x13 = 6'd0;
        x21 = 6'd0;
        x22 = 6'd0;
        x23 = 6'd0;
        x31 = 6'd0;
        x32 = 6'd0;
        x33 = 6'd0;
     end
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     weight_in_pointer <= 0;
   else if(buffer_weight_fire & (weight_in_pointer != TI))
     weight_in_pointer <= weight_in_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
   begin
      weight_buffer[0] <= 9'd0;
      weight_buffer[1] <= 9'd0;
      weight_buffer[2] <= 9'd0;
   end
   else if (buffer_weight_fire & (weight_in_pointer != TI))
      weight_buffer[weight_in_pointer] <= weight_in;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     kernel_reg <= 9'd0;
   else if(buffer_out_counter!=TI)
     kernel_reg <= weight_buffer[buffer_out_counter];
   else if(buffer_out_counter==TI)
     kernel_reg <= weight_buffer[0];
end

assign w11 = compute_fire ? kernel_reg[0] : 1'b0;
assign w12 = compute_fire ? kernel_reg[1] : 1'b0;
assign w13 = compute_fire ? kernel_reg[2] : 1'b0;
assign w21 = compute_fire ? kernel_reg[3] : 1'b0;
assign w22 = compute_fire ? kernel_reg[4] : 1'b0;
assign w23 = compute_fire ? kernel_reg[5] : 1'b0;
assign w31 = compute_fire ? kernel_reg[6] : 1'b0;
assign w32 = compute_fire ? kernel_reg[7] : 1'b0;
assign w33 = compute_fire ? kernel_reg[8] : 1'b0;









//tenary adder computation
always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     begin
       partial_result_1 <= 8'd0;
       partial_result_2 <= 8'd0;
       partial_result_3 <= 8'd0;
     end
   else if (compute_fire)
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
   else if(compute_fire)
      partial_result <= partial_sum;
end

reg start;
reg over;

always@(posedge clk or negedge rst_n)
begin
    if(start)
     line_buffer[pointer] <= line_buffer[pointer] + partial_result;
end


always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     pointer <= 0;
   else if((pointer == INPUT_SIZE-1) & (compute_counter == TI+1) & compute_fire)
     pointer <= 0;
   else if((compute_counter == TI+2) & compute_fire)
     pointer <= pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     compute_counter <= 5'd0;
   else if((compute_counter == TI+2) & compute_fire)
     compute_counter <= 5'd3;
   else if(compute_fire)
     compute_counter <= compute_counter + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     start <= 1'b0;
   else if((compute_counter == 3'd1) & compute_fire)
     start <= 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     over <= 1'b0;
   else if((over_compute_counter == 2*ITERATION_TIMES-1) & compute_fire)
     over <= 1'b1;
   else if((over_compute_counter == 2*ITERATION_TIMES) & compute_fire)
     over <= 1'b0;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
      over_compute_counter <= 8'd0;
   else if(((pointer=='b0) | (pointer == INPUT_SIZE-1)) & (compute_counter==TI) & compute_fire)
      over_compute_counter <= over_compute_counter + 1'b1;
   else if((over_compute_counter == 2*ITERATION_TIMES) & compute_fire)
      over_compute_counter <= 8'd0;
end


reg signed[12:0] batch_data;
wire signed [28:0] mult_result;
wire signed [29:0] add_result;

assign mult_result = batch_data * r;
assign add_result = mult_result[28] ? (mult_result >>> 3 + b) : mult_result + b;

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
      batch_data <= 13'd0;
   else if(over & (compute_counter==TI) & compute_fire)
      batch_data <= line_buffer[pointer-1];
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
      data_out <= 6'd0;
   else if((!add_result[29]) & (|add_result[28:6]) & compute_fire & over)
      data_out <= 6'd31;
   else if((add_result[29]) & (!(&add_result[28:6])) & compute_fire & over)
      data_out <= 6'b100000;
   else if (compute_fire & over)
      data_out <= add_result[5:0];
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     compute_done <= 1'b0;
   else if(compute_fire & over)
     compute_done <= 1'b1;
   else
     compute_done <= 1'b0;
end

endmodule









































