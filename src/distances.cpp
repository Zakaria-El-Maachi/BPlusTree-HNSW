#include <vector>
#include <algorithm>
#include <array>
#include <emmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h>

using namespace std;

float euclideanDistance(const array<float, 100>& a, const array<float, 100>& b) {
    float sum = 0.0f;
    for (size_t i = 0; i < 100; ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}


// float avx2_l2_distance(const array<float, 100>& x, const array<float, 100>& y)
// {
//     const float *a = x.data();
//     const float *b = y.data();
//     __m256 sum = _mm256_setzero_ps(); // Initialize sum to 0
//     unsigned i;
//     for (i = 0; i + 7 < 100; i += 8)
//     {                                              // Process 8 floats at a time
//         __m256 a_vec = _mm256_load_ps(&a[i]);      // Load 8 floats from a
//         __m256 b_vec = _mm256_load_ps(&b[i]);      // Load 8 floats from b
//         __m256 diff = _mm256_sub_ps(a_vec, b_vec); // Calculate difference
//         sum = _mm256_fmadd_ps(diff, diff, sum);    // Calculate sum of squares
//     }
//     float result = 0;
//     for (unsigned j = 0; j < 8; ++j)
//     { // Reduce sum to a single float
//         result += ((float *)&sum)[j];
//     }
//     return result; // Return L2 norm squared (not square root)
// }


// float avx512_l2_distance_opt(const array<float, 100>& x, const array<float, 100>& y)
// {
//     const float *a = x.data();
//     const float *b = y.data();
//     const uint32_t kFloatsPerVec = 16;
//     __m512 sum1 = _mm512_setzero_ps();
//     // n = 112;
//     for (uint32_t i = 0; i + kFloatsPerVec <= 112; i += kFloatsPerVec)
//     {
//         // Load two sets of 32 floats from a and b with aligned memory access
//         __m512 a_vec1 = _mm512_load_ps(&a[i]);
//         __m512 b_vec1 = _mm512_load_ps(&b[i]);
//         __m512 diff1 = _mm512_sub_ps(a_vec1, b_vec1);
//         sum1 = _mm512_fmadd_ps(diff1, diff1, sum1);
//     }

//     // Combine the two sum vectors to a single sum vector

//     float result = 0;
//     // Sum the remaining floats in the sum vector using non-vectorized operations
//     for (int j = 0; j < kFloatsPerVec; j++)
//     {
//         // result += ((float*)&sum12)[j];
//         result += ((float *)&sum1)[j];
//     }
//     return result;
// }
