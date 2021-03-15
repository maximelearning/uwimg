/**
 * Estevan Seyfried, estevans
 * Maxime Sutters, msutters
 */

#include <math.h>
#include <stdlib.h>
#include "image.h"
#include "matrix.h"

// Run an activation function on each element in a matrix,
// modifies the matrix in place
// matrix m: Input to activation function
// ACTIVATION a: function to run
void activate_matrix(matrix m, ACTIVATION a)
{
    int i, j;
    for(i = 0; i < m.rows; ++i){
        double sum = 0;
        for(j = 0; j < m.cols; ++j){
            double x = m.data[i][j];
            if(a == LOGISTIC){
                m.data[i][j] = 1.0 / (1.0 + exp(-x));
            } else if (a == RELU){
                if (x <= 0) m.data[i][j] = 0.0;
            } else if (a == LRELU){
                if (x <= 0) m.data[i][j] *= 0.1;
            } else if (a == SOFTMAX){
                m.data[i][j] = exp(x);
            }
            sum += m.data[i][j];
        }
        if (a == SOFTMAX) {
            for(j = 0; j < m.cols; ++j){
                m.data[i][j] = m.data[i][j] / sum;
            }
        }
    }
}

// Calculates the gradient of an activation function and multiplies it into
// the delta for a layer
// matrix m: an activated layer output
// ACTIVATION a: activation function for a layer
// matrix d: delta before activation gradient
void gradient_matrix(matrix m, ACTIVATION a, matrix d)
{
    int i, j;
    for (i = 0; i < m.rows; ++i){
        for (j = 0; j < m.cols; ++j){
            double x = m.data[i][j];
            if (a == LOGISTIC){
                d.data[i][j] *= x - x * x;
            } else if (a == RELU){
                if (x <= 0) d.data[i][j] = 0.0;
            } else if (a == LRELU){
                if (x <= 0) d.data[i][j] *= 0.1;
            }
            // if its linear or softmax gradient is 1, so we dont change d
        }
    }
}

// Forward propagate information through a layer
// layer *l: pointer to the layer
// matrix in: input to layer
// returns: matrix that is output of the layer
matrix forward_layer(layer *l, matrix in)
{
    // Save the input for backpropagation
    l->in = in;  

    // Multiply input by weights and apply activation function.
    matrix out = matrix_mult_matrix(in, l->w);
    activate_matrix(out, l->activation);

    // free the old output and save the current output for gradient calculation
    free_matrix(l->out);
    l->out = out;
    return out;
}

// Backward propagate derivatives through a layer
// layer *l: pointer to the layer
// matrix delta: partial derivative of loss w.r.t. output of layer
// returns: matrix, partial derivative of loss w.r.t. input to layer
matrix backward_layer(layer *l, matrix delta)
{
    // 1.4.1
    // delta is dL/dy
    // Modify it in place to be dL/d(xw)
    gradient_matrix(l->out, l->activation, delta);

    // 1.4.2
    // Calculate dL/dw and save it in l->dw
    // xt * dL/d(xw) where xt is the transpose of the input matrix x.
    free_matrix(l->dw);
    matrix xt = transpose_matrix(l->in);
    l->dw = matrix_mult_matrix(xt, delta);
    free_matrix(xt);

    // 1.4.3
    // Calculate dL/dx and return it.
    // dL/d(xw) * wt where wt is the transpose of our weights, w
    matrix wt = transpose_matrix(l->w);
    matrix dx = matrix_mult_matrix(delta, wt);
    free_matrix(wt);

    return dx;
}

// Update the weights at layer l
// layer *l: pointer to the layer
// double rate: learning rate
// double momentum: amount of momentum to use
// double decay: value for weight decay
void update_layer(layer *l, double rate, double momentum, double decay)
{
    // Calculate Δw_t = dL/dw_t - λw_t + mΔw_{t-1}
    // l->dw = dL/dw_t, l->v = Δw_t
    matrix rTimesWPlusDW = axpy_matrix(-decay, l->w, l->dw);
    matrix dw_t = axpy_matrix(momentum, l->v, rTimesWPlusDW);
    matrix wPlus1 = axpy_matrix(rate, dw_t, l->w);
    
    // Remember to free any intermediate results to avoid memory leaks
    free_matrix(rTimesWPlusDW);
    
    // save it to l->v
    free_matrix(l->v);
    l->v = dw_t;

    // Update l->w
    free_matrix(l->w);
    l->w = wPlus1;
}

// Make a new layer for our model
// int input: number of inputs to the layer
// int output: number of outputs from the layer
// ACTIVATION activation: the activation function to use
layer make_layer(int input, int output, ACTIVATION activation)
{
    layer l;
    l.in  = make_matrix(1,1);
    l.out = make_matrix(1,1);
    l.w   = random_matrix(input, output, sqrt(2./input));
    l.v   = make_matrix(input, output);
    l.dw  = make_matrix(input, output);
    l.activation = activation;
    return l;
}

// Run a model on input X
// model m: model to run
// matrix X: input to model
// returns: result matrix
matrix forward_model(model m, matrix X)
{
    int i;
    for(i = 0; i < m.n; ++i){
        X = forward_layer(m.layers + i, X);
    }
    return X;
}

// Run a model backward given gradient dL
// model m: model to run
// matrix dL: partial derivative of loss w.r.t. model output dL/dy
void backward_model(model m, matrix dL)
{
    matrix d = copy_matrix(dL);
    int i;
    for(i = m.n-1; i >= 0; --i){
        matrix prev = backward_layer(m.layers + i, d);
        free_matrix(d);
        d = prev;
    }
    free_matrix(d);
}

// Update the model weights
// model m: model to update
// double rate: learning rate
// double momentum: amount of momentum to use
// double decay: value for weight decay
void update_model(model m, double rate, double momentum, double decay)
{
    int i;
    for(i = 0; i < m.n; ++i){
        update_layer(m.layers + i, rate, momentum, decay);
    }
}

// Find the index of the maximum element in an array
// double *a: array
// int n: size of a, |a|
// returns: index of maximum element
int max_index(double *a, int n)
{
    if(n <= 0) return -1;
    int i;
    int max_i = 0;
    double max = a[0];
    for (i = 1; i < n; ++i) {
        if (a[i] > max){
            max = a[i];
            max_i = i;
        }
    }
    return max_i;
}

// Calculate the accuracy of a model on some data d
// model m: model to run
// data d: data to run on
// returns: accuracy, number correct / total
double accuracy_model(model m, data d)
{
    matrix p = forward_model(m, d.X);
    int i;
    int correct = 0;
    for(i = 0; i < d.y.rows; ++i){
        if(max_index(d.y.data[i], d.y.cols) == max_index(p.data[i], p.cols)) ++correct;
    }
    return (double)correct / d.y.rows;
}

// Calculate the cross-entropy loss for a set of predictions
// matrix y: the correct values
// matrix p: the predictions
// returns: average cross-entropy loss over data points, 1/n Σ(-ylog(p))
double cross_entropy_loss(matrix y, matrix p)
{
    int i, j;
    double sum = 0;
    for(i = 0; i < y.rows; ++i){
        for(j = 0; j < y.cols; ++j){
            sum += -y.data[i][j]*log(p.data[i][j]);
        }
    }
    return sum/y.rows;
}


// Train a model on a dataset using SGD
// model m: model to train
// data d: dataset to train on
// int batch: batch size for SGD
// int iters: number of iterations of SGD to run (i.e. how many batches)
// double rate: learning rate
// double momentum: momentum
// double decay: weight decay
void train_model(model m, data d, int batch, int iters, double rate, double momentum, double decay)
{
    int e;
    for(e = 0; e < iters; ++e){
        data b = random_batch(d, batch);
        matrix p = forward_model(m, b.X);
        fprintf(stderr, "%06d: Loss: %f\n", e, cross_entropy_loss(b.y, p));
        matrix dL = axpy_matrix(-1, p, b.y); // partial derivative of loss dL/dy
        backward_model(m, dL);
        update_model(m, rate/batch, momentum, decay);
        free_matrix(dL);
        free_data(b);
    }
}


// Questions 
//
// 5.2.2.1 Why might we be interested in both training accuracy and testing accuracy? What do these two numbers tell us about our current model?
// We care about both accuracies because we want to know how effective the training 
// process was and how effective the model is on real data. 
// Training accuracy pertains to how effective the model is at classifying data it was trained on.
// Testing accuracy describes how accurate the model is at classifying data it has not seen before.
//
// 5.2.2.2 Try varying the model parameter for learning rate to different powers of 10 (i.e. 10^1, 10^0, 10^-1, 10^-2, 10^-3) and training the model. What patterns do you see and how does the choice of learning rate affect both the loss during training and the final model accuracy?
// Accuracy for both training and testing were effected very similarly as the learning rate changed.
// Accuracy for both peaked around 0.1, a learning rate of 10 resulted in almost no accuracy and 
// rates lower than 0.01 corresponded to a gradual decrease in accuracy. 
//
// 5.2.2.3 Try varying the parameter for weight decay to different powers of 10: (10^0, 10^-1, 10^-2, 10^-3, 10^-4, 10^-5). How does weight decay affect the final model training and test accuracy?
// Any weight decay higher than 1.0 resulted in a lower testing and training accuracy.
// As weight decay becomes smaller (approaches 0) accuracy increases but eventually 
// seems to plateau, so at least with SoftMax, a decay of 0 seems to be the best choice. 
//
// 5.2.3.1 Currently the model uses a logistic activation for the first layer. Try using a the different activation functions we programmed. How well do they perform? What's best?
// In order from best to worst Testing Accuracy with default settings: 
// Relu: 0.9281, LRelu: 0.9263, Linear: 0.9162, Logistic: 0.8949, SoftMax: 0.6043
// Relu barely outpreformed LRelu.
//
// 5.2.3.2 Using the same activation, find the best (power of 10) learning rate for your model. What is the training accuracy and testing accuracy?
// Relu peaks at a learning rate of 0.01, training accuracy: 0.92605, testing accuracy: 0.9281
// Both accuracies decrease the further you diverge from the peak of 0.01
//
// 5.2.3.3 Right now the regularization parameter `decay` is set to 0. Try adding some decay to your model. What happens, does it help? Why or why not may this be?
// A small amount of weight decay (0.001) resulted in a slightly higher accuracy, but accuracy dropped dramatically for any rate higher 
// than 0.1. At higher decay rates the benefits that we gained from penalizing larger weights begins to negativly affect the entire model.
//
// 5.2.3.4 Modify your model so it has 3 layers instead of two. The layers should be `inputs -> 64`, `64 -> 32`, and `32 -> outputs`. Also modify your model to train for 3000 iterations instead of 1000. Look at the training and testing error for different values of decay (powers of 10, 10^-4 -> 10^0). Which is best? Why?
// A decay rate of 0.001 performed slightly better than 0 and much better than 10.
// As we suspected previously, a small amount of decay helps normalize the model, but too much
// negativly affects accuracy. 
//
// 5.3.2.1 How well does your network perform on the CIFAR dataset?
// We tried different configurations of layers and settings, but the 
// best results for MNIST also performed the best for CIFAR. However the 
// accuracy was much lower. Using a 3 layer RELU, RELU, SOFTMAX with rate: 0.01, decay: 0.001
// MNIST: training accuracy: 0.9721166666666666, test accuracy: 0.965
// CIFAR: training accuracy: 0.46252, test accuracy: 0.4447
