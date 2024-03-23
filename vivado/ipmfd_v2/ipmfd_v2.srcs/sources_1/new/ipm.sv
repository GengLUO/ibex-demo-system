module ipm #(
    parameter n = 4,
    parameter k = 1,
    localparam N = n - k + 1,
    localparam WIDTH = N * 8,
    localparam N_END = N - 1
) (
    input logic clk_i,
    input logic reset_ni,
    input logic [WIDTH - 1:0] a_i,
    input logic [WIDTH - 1:0] b_i,
    
    input logic ipm_en_i,  // dynamic enable signal, for FSM control
    input logic ipm_sel_i, // static decoder output, for data muxes
    input ibex_pkg::ipm_op_e ipm_operator_i,
    
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
    state_e ipm_state_q, ipm_state_d;
    logic next_q, next_d;

    // Registers for loop indices and intermediate values
    logic [$clog2(N)-1:0] i_q, j_q, i_d, j_d;
    
    logic [7:0] multiplier_inputs_a[0:2];
    logic [7:0] multiplier_inputs_b[0:2];
    logic [7:0] multiplier_results[0:2];

    
//  logic        ipm_hold;
  logic        ipm_en;
  assign ipm_en = ipm_en_i;
  
    logic [7:0] result [0:N-1];
    logic [7:0] a [0:N-1];
    logic [7:0] b [0:N-1];
    
    always_comb begin
        int i;
        for (i = 0; i < N; i++) begin
            a[i] = a_i[WIDTH-1 - i*8 -: 8];
            b[i] = b_i[WIDTH-1 - i*8 -: 8];
        end
    end
  
    
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
    
    ibex_pkg::ipm_op_e operator;
    assign operator = ipm_operator_i;
    
    logic [31:0] sq_res;
    logic [7:0] sq_res_block [0:3];
    always_comb begin
        int i;
        for (i = 0; i < N; i++) begin
            sq_res_block[i] = sq_res[31 - i*8 -: 8];
        end
    end
    sq sq_inst(
        .sq_i(a_i),
        .sq_o(sq_res)
    );
    
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
genvar idx;
generate
    for (idx = 0; idx < 3; idx = idx + 1) begin : gfmul_gen
        gfmul gfmul_inst (
            .rs1(multiplier_inputs_a[idx]),
            .rs2(multiplier_inputs_b[idx]),
            .rd(multiplier_results[idx])
        );
    end
endgenerate

        // State transition and output logic
    always_ff @(posedge clk_i or negedge reset_ni) begin
        if (!reset_ni) begin
            ipm_state_q <= IDLE;
            i_q <= 0;
            j_q <= 0;
            for (int i = 0; i < N; i++) begin
                result[i] <= 0;
            end
        end else if (ipm_en) begin
            ipm_state_q <= ipm_state_d;
            i_q <= i_d;
            j_q <= j_d;
            
            case (ipm_state_q)
                IDLE: begin
                    if (ipm_sel_i) begin
                        for (int i = 0; i < N; i++) begin
                            result[i] <= 0;
                        end
                    end
                end
                COMPUTE: begin
                    unique case (operator)
                        ibex_pkg::IPM_OP_MUL: begin
                            result[index_i] <= result[index_i] ^ T ^ U;
                        end
                        ibex_pkg::IPM_OP_MASK: begin
                            result[index_j] <= a[index_j] ^ multiplier_results[0] ^ multiplier_results[1] ^ multiplier_results[2];
                            for (int i = k ; i < n ; i++) begin
                                result[i] <= random[0][i];
                            end
                        end
                        ibex_pkg::IPM_OP_HOMOG: begin
                            result[0] <= b[0] ^ multiplier_results[0] ^ multiplier_results[1] ^ multiplier_results[2];
                            for (int i = 1; i < N ; i++) begin
                                result[i] <= a[i];
                            end
                        end
                        ibex_pkg::IPM_OP_SQUARE: begin
                            result[0] <= sq_res_block[0];
                            for (int i = 1; i < N ; i++) begin
                                result[i] <= multiplier_results[i-1];
                            end
                        end
                        default;
                    endcase
                end
                default:;
            endcase
      
        end
    end
    
    
        // Next state logic
    always_comb begin
        ipm_state_d = ipm_state_q;
        if (ipm_sel_i) begin
            unique case (ipm_state_q)
                IDLE: ipm_state_d = COMPUTE;
                COMPUTE: begin
                    unique case (operator)
                        ibex_pkg::IPM_OP_MUL: begin
                            ipm_state_d = (i_q == N_END && j_q == N_END) ? DONE : COMPUTE; //require n^2 cycles to complete
                        end
                        ibex_pkg::IPM_OP_MASK: begin
                            ipm_state_d = (j_q == k-1) ? DONE : COMPUTE; //require k cycles
                        end
                        ibex_pkg::IPM_OP_HOMOG,
                        ibex_pkg::IPM_OP_SQUARE: begin
                            ipm_state_d = (j_q == 1) ? DONE : COMPUTE; //require 1 cycles
                        end
                        default:;
                    endcase
                end
                DONE: ipm_state_d = IDLE;
                default:ipm_state_d = IDLE;
        endcase
        end
    end

    always_ff @(posedge clk_i or negedge reset_ni) begin
        if (!reset_ni) begin
            next_q <= 0; //only used for ipmmul
            U_prime_q <= 0; //only used for ipmmul
        end else begin
            next_q <= next_d; //only used for ipmmul
            U_prime_q <= U_prime_d; //only used for ipmmul
        end
    end
    
    always_comb begin
        next_d = 1;
        U_prime_d = 0;
        for(int i=0 ; i < 3; i++) begin
            multiplier_inputs_a[i] = 0;
            multiplier_inputs_b[i] = 0;
        end
        T = 0;
        U = 0;
        
        unique case (operator)
            ibex_pkg::IPM_OP_MUL: begin
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
            end
            default:;
        endcase
        
        unique case (operator)
            ibex_pkg::IPM_OP_MUL: begin
                multiplier_inputs_a[0] = a[index_i];
                multiplier_inputs_b[0] = b[index_j];
                multiplier_inputs_a[1] = multiplier_results[0];
                multiplier_inputs_b[1] = L_prime[index_j];
                T = multiplier_results[1];
        
                multiplier_inputs_a[2] = U_prime_q;
                multiplier_inputs_b[2] = L_prime_inv[index_i];
                U = multiplier_results[2];
            end
            ibex_pkg::IPM_OP_MASK: begin
                multiplier_inputs_a[0] = random[0][k];
                multiplier_inputs_b[0] = L_prime[k]; //[index_j][k]
                multiplier_inputs_a[1] = random[0][k+1];
                multiplier_inputs_b[1] = L_prime[k+1]; //[index_j][k+1]
                multiplier_inputs_a[2] = random[0][k+2];
                multiplier_inputs_b[2] = L_prime[k+2]; //[index_j][k+2]
            end
            ibex_pkg::IPM_OP_HOMOG: begin
                for (int i = 0; i < 3; i++) begin
                    multiplier_inputs_a[i] = L_prime[i+1];
                    multiplier_inputs_b[i] = a[i+1] ^ b[i+1];
                end
            end
            ibex_pkg::IPM_OP_SQUARE: begin
                for (int i = 0; i < 3; i++) begin
                    multiplier_inputs_a[i] = sq_res_block[i+1];
                    multiplier_inputs_b[i] = L_prime[i+1];
                end
            end
            default: ;
        endcase
    end
    
    // Index update logic
always_comb begin
    i_d = i_q;
    j_d = j_q;

    if (ipm_state_q == COMPUTE) begin
        if (next_q) begin
            if (j_q < N_END) begin
                j_d = j_q + 1;
            end 
            else begin
                i_d = (i_q < N_END) ? i_q + 1 : 0;
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

assign valid_o = ipm_state_q == DONE;

always_comb begin
    int i;
    for (i = 0; i < N; i++) begin
        result_o[WIDTH-1 - i*8 -: 8] = result[i];
    end
end


endmodule
