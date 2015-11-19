#include <pal.h>

/** Convolution on input image 'x' with a square kernel 'm' of size 'msize'.
 *
 * @param output_image Pointer to output, 2D array '(rows/p)*(cols/p)*num_kernels'
 *                     where 'p'= xy_pooling
 *
 * @param image        Pointer to input image, a 2D array of size 'rows' x 'cols'
 *
 * @param rows         Number of rows from image to run kernel
 *
 * @param cols         Number of columns from image to apply kernel
 *
 * @param image_stride Number of floats per line in the input image
 *
 * @param kernel       Pointer to array of 2d convlution kernels
 *
 * @param krows        number of rows in convolution kernel
 *
 * @param kcols        number of cols in convolution kernel
 *
 * @param num_kernels  number of convolution kernels
 *                     such that the total filter size is mrows*mcols*num_kernels
 *                     and output size is rows*cols*num_kernels
 *
 * @param xy_pooling   max pooling size; max of each p x p square of results 
                       is stored in the output
 * @return      None
 *
 */

/*
Performs 2d convolutions of a single image with an array of filters
The output of filter is offset by a bias and clamped above zero;
the output image is reduced by 'max-pooling'

Implements the layer in convolutional neural network algorithm for image recognition.

an epiphany implementation may be able to cache the entire kernel once
to reduce data-movement.
 */

float p_conv2d_apply_kernel(const float* image, int image_stride, const float* kernel, int kernel_rows, int kernel_cols) {
    int i,j;
    float acc=0.f;

    for (j=0; j<kernel_rows; j++,image+=image_stride-mcols) {
        for (i=0; i<kernel_cols; i++, kernel++) {
            acc+=*image * *kernel;
		}
    }
    return acc;
}

void p_conv2d_multi_relu_maxpool_f32(
                                  float* output_image,
                                  const float *src_image,
                                  int rows, int cols, int src_image_stride,
                                  const float* kernel,
                                  int krows, int kcols, int num_kernels,
                                  const float *bias_per_depth, // added to each slice prior to ReLU
                                  float min_output, // =0.f for use in conv-nn
                                  int xy_pooling);  //=1 for no pooling

{
    int rowsub, colsub, row,col,mi, mj, d;
    float P;
    const float *src, *pm,*p_kernel,src_sub;
    float *dst;

    px = x;
    dst = r;

    for (d=0,p_kernel=m; d<num_kernels; d++,p_kernel+=mrows*mcols) {
        float bias = bias_per_depth[d];

        for (row = 0,src=src_image; row < rows - mrows+1  ; 
             row+=xy_pooling,src+=src_image_stride-cols) {
            for (col = 0; col < cols - mcols+1 ;
                 col+=xy_pooling,src+=xy_pooling) {
                float value=min_output;
                // Apply the kernel to a square of input pixels & pool the result
                float src_sub=src;
                for (rowsub=0; rowsub<xy_maxpooling;
                     rowsub++,src_sub+=image_stride-xy_pooling) {
                    for (colsub=0; colsub<xy_maxpooling; colsub++,src_sub++) {
                        float psub=p_conv2d_apply_kernel(src_sub,pk)+bias;
                        if (psub>value) {value=psub;}
                    }
				}
                *dst++ = value;
			}
        }
	}
}


