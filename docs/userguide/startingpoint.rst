***************************
Using X0 as a starting point
***************************

The main advantage of choosing this method of initialization rather than the previous one, which relies on using keywords, is that defining
an X0 allows for more flexiblity since one can choose a value for each parameter of each layer. For example, using a keyword such as 'KERNELS' means that all the kernels applied on every convolutional layer will have the same initial value.
Whereas an X0 allows to initialize each kernel individually.

The order and meaning of the variables in X0 is hardcoded in HYPERNOMAD. Let's use the following parameter file as an example :


.. code-block:: sh

  
  DATASET MNIST
  MAX_BB_EVAL 100

  HYPER_DISPLAY 3
  
  #              [ CONVOLUTION BLOCK               ]   [ FULLY CONNECTED BLOCK ]  [BATCH] [   OPTIMIZER BLOCK      ] [DROPOUT][ACTIVATION]
  X0           (   2     6  5 1 0 1    16  5 1 0 1        2  128   84               128     3   0.1  0.9  0.0005 0     0.2        1        )
  #LOWER_BOUND (   1     1  1 1 0 0     1  1 1 0 0        0    1    1                1       1   0    0     0    0       0        1        )  
  #UPPER_BOUND ( 100  1000 20 3 2 1  1000 20 3 2 1      500 1000 1000              400       4   1    1    1      1   1    3 )


  DROPOUT_RATE 0.5 - - FIXED
  KERNELS 10 - - FIXED
  REMAINING_HYPERPARAMETERS VAR


Analysis of the example
=========================

First, 'HYPER_DISPLAY' allows set the level of details on the steps of HYPERNOMAD. The default value is 1, and the maximum is 3.
Then, X0 is presented as a list of parameters that are respectively categorised into the convolutional block, the fully connected block, 
the batch size, the optimizer block, the dropout rate and the activate function.

The blocks for the batch size, dropout rate and activate function contain each one single value which that of the corresponding hyperparameter.

The first variable of the convolutional block indicates the number of convolutional layers : 2 in this example. Each convolutional layer has
5 associated variables : (number of output channeles, kernel, stride, padding, do pooling). Therefor, the first convolutional layer has 6 output channels, a (5,5) kernel,
a stride of 1, no padding and performs a pooling afterwards. The same goes for the second layer.

The first variable of the fully connected block corresponds to the number of fully connected layers. The following variables indicate the size
of each fully connected layer.

The first variable of the optimizer block indicates which optimizer is used, here is it Adagrad. The optimizer block always has 4 associated variables
whose meaning change according to the optimizer chosen. For example in the case of SGD, the first variable is the learning rate followed by the momentum,
the dampening and the weight decay.


Advantage of using X0
==========================

In addition to being able to initialize each hyperparameter on it's own, we can also define specific lower and upper bounds for each 
single hyperparameter as is shown in the previous example.

.. note:: Note that X0 takes precedence over the other keywords, therefore the tags KERNELS and DROPOUT_RATE will not affect this initial starting point.
  
    
    
    
