#version 450

#define TILE_SIZE 16

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE , local_size_z = 1) in;

layout(std430, binding = 0) readonly buffer ABuffer {
    float A[];
};

layout(std430, binding = 1) readonly buffer BBuffer {
    float B[];
};

layout(std430, binding = 2) writeonly buffer CBuffer {
    float C[];
};

layout(push_constant) uniform MatSize {
    uint M;
    uint N;
    uint K;
} uSize;

// 每个工作组共享的一块 tile
shared float Asub[TILE_SIZE][TILE_SIZE];
shared float Bsub[TILE_SIZE][TILE_SIZE];

void main() {
    uint row = gl_GlobalInvocationID.y;
    uint col = gl_GlobalInvocationID.x;

    float acc = 0.0;

    // 遍历所有 tile（t 是 tile 的横向索引）
    for (uint t = 0; t < uSize.K / TILE_SIZE; ++t) {
        // 每个线程加载一格 tile 到 shared memory
        Asub[gl_LocalInvocationID.y][gl_LocalInvocationID.x] 
            = A[row * uSize.K + (t * TILE_SIZE + gl_LocalInvocationID.x)];
        Bsub[gl_LocalInvocationID.y][gl_LocalInvocationID.x] 
            = B[(t * TILE_SIZE + gl_LocalInvocationID.y) * uSize.N + col];
        
        barrier(); // 所有线程都加载完 tile 后才可用
        
        // 本地 tile 内部计算
        for (uint k = 0; k < TILE_SIZE; ++k) {
            acc += Asub[gl_LocalInvocationID.y][k] * Bsub[k][gl_LocalInvocationID.x];
        }//end for k

        barrier(); // 同步清除 shared memory，准备下一 tile
    }

    // 写回结果矩阵
    C[row * uSize.N + col] = acc;
}