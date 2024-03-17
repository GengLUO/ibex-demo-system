//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/03/13 14:50:59
// Design Name: 
// Module Name: ipmmul_ex
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module ipmmul #(
    parameter int N = 4,
    localparam int WIDTH = N * 8
) (
    input logic clk_i,
    input logic reset_ni,
    input logic [WIDTH - 1:0] a_i,
    input logic [WIDTH - 1:0] b_i,
    
    input logic ipm_en_i,  // dynamic enable signal, for FSM control
    input logic ipm_sel_i, // static decoder output, for data muxes
    input ibex_pkg::ipm_op_e ipm_operator_i, //TODO:include other operations
    
    output logic [WIDTH - 1:0] result_o,
    output logic valid_o
);

    // State definitions
    typedef enum logic [1:0] {
        IDLE,
        COMPUTE,
        DONE
    } state_e;
    
    // Registers for FSM
    state_e ipmmul_state_q, ipmmul_state_d;
    logic next_q, next_d;

    // Registers for loop indices and intermediate values
    logic [$clog2(N)-1:0] i_q, j_q, i_d, j_d;
    
    logic [7:0] mult1_a, mult1_b, mult2_a, mult2_b, mult3_a, mult3_b;
    logic [7:0] mult1_result, mult2_result, mult3_result;
    
  logic        ipm_hold;
  logic        ipm_en;
  assign ipm_en = ipm_en_i;
  
    
    // Memory for computations
    logic [7:0] T;
    logic [7:0] U;
    logic [7:0] U_prime_q, U_prime_d;
    logic [7:0] L_prime[4];
    logic [7:0] L_prime_inv[4];
    logic [7:0] random[4][4];

    logic [1:0] index_i, index_j; //the true index to be performed on the matrix
    assign index_i = next_q ? i_q : j_q;
    assign index_j = next_q ? j_q : i_q;
    
    initial begin
    L_prime[0] = 8'd1;
    L_prime[1] = 8'd27;
    L_prime[2] = 8'd250;
    L_prime[3] = 8'd188;

    L_prime_inv[0] = 8'd1;
    L_prime_inv[1] = 8'd204;
    L_prime_inv[2] = 8'd125;
    L_prime_inv[3] = 8'd189;
    
    random[0][0] = 8'd43;
    random[0][1] = 8'd65;
    random[0][2] = 8'd63;
    random[0][3] = 8'd97;
    
    random[1][0] = 8'd123;
    random[1][1] = 8'd1;
    random[1][2] = 8'd239;
    random[1][3] = 8'd54;
    
    random[2][0] = 8'd78;
    random[2][1] = 8'd76;
    random[2][2] = 8'd127;
    random[2][3] = 8'd179;
    
    random[3][0] = 8'd222;
    random[3][1] = 8'd48;
    random[3][2] = 8'd74;
    random[3][3] = 8'd59;
    
end

// Instantiation of GF(256) multipliers
gfmul gfmul1_inst (
    .rs1(mult1_a),
    .rs2(mult1_b),
    .rd(mult1_result)
);

gfmul gfmul2_inst (
    .rs1(mult2_a),
    .rs2(mult2_b),
    .rd(mult2_result)
);

gfmul gfmul3_inst (
    .rs1(mult3_a),
    .rs2(mult3_b),
    .rd(mult3_result)
);

        // State transition and output logic
    always_ff @(posedge clk_i or negedge reset_ni) begin
        if (!reset_ni) begin
            ipmmul_state_q <= IDLE;
            i_q <= 0;
            j_q <= 0;
//            valid_o <= 0;
        end else if (ipm_en) begin
            ipmmul_state_q <= ipmmul_state_d;
            i_q <= i_d;
            j_q <= j_d;
            
            case (ipmmul_state_q)
                IDLE: if (ipm_sel_i) result_o <= 0;
                COMPUTE: result_o[WIDTH-1 - (index_i * 8) -: 8] <= result_o[WIDTH-1 - (index_i * 8) -: 8] ^ T ^ U;
//                DONE: valid_o <= 1;
                default:;
            endcase
        end
    end
    
    
        // Next state logic
    always_comb begin
        ipmmul_state_d = ipmmul_state_q;
        if (ipm_sel_i) begin
            unique case (ipmmul_state_q)
                IDLE: ipmmul_state_d = COMPUTE;
//                COMPUTE: ipmmul_state_d = (index_i < N-1-1 || index_j < N-1) ? COMPUTE : DONE;
                COMPUTE: ipmmul_state_d = (i_q == N-1 && j_q == N-1) ? DONE : COMPUTE;
                DONE: ipmmul_state_d = IDLE;
                default:ipmmul_state_d = IDLE;
        endcase
        end
    end

    always_ff @(posedge clk_i or negedge reset_ni) begin
        if (!reset_ni) begin
            next_q <= 0;
            U_prime_q <= 0;
        end else begin
            next_q <= next_d;
            U_prime_q <= U_prime_d;
        end
    end

    always_comb begin
        if (i_d == j_d) begin
            next_d = 1;
            U_prime_d = 0;
        end
        else begin
            if (next_q) begin
                next_d = 0;
                U_prime_d = random[i_d][j_d];
            end
            else begin
                next_d = 1;
                U_prime_d = U_prime_q;
            end
        end
        
        mult1_a = a_i[WIDTH-1 - (index_i * 8) -: 8];
        mult1_b = b_i[WIDTH-1 - (index_j * 8) -: 8];
        mult2_a = mult1_result;
        mult2_b = L_prime[index_j];
        T = mult2_result;
        
        mult3_a = U_prime_q; 
        mult3_b = L_prime_inv[index_i]; 
        U = mult3_result;
    end
    
    // Index update logic
always_comb begin
    i_d = i_q;
    j_d = j_q;

    if (ipmmul_state_q == COMPUTE) begin
        if (next_q) begin
            if (j_q < N-1) begin
                j_d = j_q + 1;
            end 
            else begin
                i_d = (i_q < N-1) ? i_q + 1 : 0;
                j_d = i_d;
            end
        end
        else begin
            j_d = j_q;
        end
    end else begin
        i_d = 0;
        j_d = 0;
    end
end

assign valid_o = ipmmul_state_q == DONE;

endmodule