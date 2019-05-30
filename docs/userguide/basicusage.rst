***************************
Basic usage
***************************

HYPERNOMAD is a library that aims at optimizing the hyperparameters of a deep neural  network for a given application.
Note that, at this stage, HYPERNOMAD is tailored for convolutional networks only.

In order to start an optimization, a few informations must be provided in order to specify the dataset used and optionnally some informations on the search space.
This is done in a parameter file which is passed as an argument to hypernomad.exe, as is shown in the following example:

.. code-block:: sh

    (path_to_bin)/hypernomad.exe parameter_file.txt
    
    
Choosing a dataset
=========================
    
The parameter file must contain some mandatory informations such as the name of the data set and the number of times the blackbox is called, 
which corresponds to the number of different configurations HYPERNOMAD is allowed to try. This package comes with the data sets
that are listed in the table below. 
   
+--------------+--------------+-----------------+-----------+------------------+
| Dataset      | Training data| validation data | test data | Number of classes|
+--------------+--------------+-----------------+-----------+------------------+
| MNIST        |  40000       | 10000           | 10000     |    10            |
+--------------+--------------+-----------------+-----------+------------------+
| Fashion MNIST|  40000       | 10000           | 10000     |    10            |
+--------------+--------------+-----------------+-----------+------------------+
| EMNIST       |  40000       | 10000           | 10000     |    10            |
+--------------+--------------+-----------------+-----------+------------------+
| KMNIST       |  40000       | 10000           | 10000     |    10            |
+--------------+--------------+-----------------+-----------+------------------+
| CIFAR10      |  40000       | 10000           | 10000     |    10            |
+--------------+--------------+-----------------+-----------+------------------+
| CIFAR100     |  40000       | 10000           | 10000     |    100           |
+--------------+--------------+-----------------+-----------+------------------+
| STL10        |  4000        | 1000            | 8000      |    10            |
+--------------+--------------+-----------------+-----------+------------------+

HYPERNOMAD also offers the possibily of using a personnal data set in which case the user is 
responsible for providing the necessary informations to the blackbox. The necessary instructions to do so are provided in the Advanced usage section.

Specifying the search space
==============================

The user can choose to provide additionnal informations on the search space considered. HYPERNOMAD allows for a good flexibility of tuning a convolutional network
by considering multiple aspects of a network at once such as the architecture, the dropout rate, the choice of the optimizer and the hyperparameters related to the optimization aspect
(learning rate, weight decay, momentum, ...), the batch size, etc. The user can choose to optimize all these aspects or select a few and fixe the others to certain values. The user can also change
the default range of each hyperparameter. 

This information is passed through the parameter file by using a specific synthax:

.. code-block:: sh

  KEYWORD INITIAL_VALUE LOWER_BOUND UPPER_BOUND FIXED/VAR


This table lists all the possible keywords, their interpretation and the default values and ranges for each one

+-------------------------+---------------------------------------------+-----------+----------------------------------+
| Name                    | Description                                 | Default   | Range                            |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| DATASET                 | name of the dataset used                    | no default|From previous table, or CUSTOM    |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| NUMBER_OF_CLASSES       | number of classes of the problem            | no default| Only use if dataset is CUSTOM    |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| MAX_BB_EVALS            | number of configurations to try             | no default| Integer > 1                      |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| NUM_CON_LAYERS          | number of convolutional layers              | 2         | [0, 100]                         |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| OUTPUT_CHANNELS         | number of output channels of the layer      | 6         | [1, 100]                         |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| KERNELS                 | size of the kernels applied                 | 5         | [1, 20]                          |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| STRIDES                 | step of the kernels                         | 1         | [1, 3]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| PADDINGS                | size of the padding                         | 0         | [0, 2]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| DO_POOL                 | weather apply a pooling or not              | 0         | 0, 1                             |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| NUM_FC_LAYERS           | number of fully connected layers            | 2         | [0, 500]                         |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| SIZE_FC_LAYER           | size of the fully connected layer           | 128       | [1, 1000]                        |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|BATCH_SIZE               | the batch size                              | 128       | [1, 400]                         |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|OPTIMIZER_CHOICE         | from SGD, Adam, Adagrad, RMSProp            | 3         | [1, 4]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|OPT_PARAM_1              | learning rate                               | 0.1       | [0, 1]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|OPT_PARAM_2              | second parameter of the optimizer           | 0.9       | [0, 1]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|OPT_PARAM_3              | third parameter of the optimizer            | 0.005     | [0, 1]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|OPT_PARAM_4              | fourth parameter of the optimizer           | 0         | [0, 1]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
| DROPOUT_RATE            | probability of dropping a node              | 0.5       | [0, 0.95]                        |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|ACTIVATION_FUNCTION      | choice from ReLU, Sigmoid or Tanh           | 1         | [1, 3]                           |
+-------------------------+---------------------------------------------+-----------+----------------------------------+
|REMAINING_HYPERPARAMETERS| use to fixe or vary those not listed in     | VAR       | FIXED, VAR                       |
|                         | the parameter file                          |           |                                  |
+-------------------------+---------------------------------------------+-----------+----------------------------------+


Example of a parameter file
==============================
Here is an example of an acceptable parameter file. First, the dataset MNIST is choosen and we specify that HYPERNOMAD is allowed to try a maximum of 100 configurations. Then, the number of convolutional layers is fixed throught the optimization to 5, the two '-' appearing after the '5' mean that the default lower and upper bounds are not changed. The kernels, number of fully connected layers and activation function are respectively initialized at 3, 6, and 2 (Sigmoid) and the dropout rate is initialized at 0.6 with a new lower bound of 0.3 and upper bound of 0.8


.. code-block:: sh

    # Mandatory information
    DATASET                 MNIST
    MAX_BB_EVAL             100

    # Optional information
    NUM_CON_LAYERS          5  -  -  FIXED
    KERNELS                 3
    NUM_FC_LAYERS           6
    ACTIVATION_FUNCTION     2
    DROPOUT_RATE            0.6  0.3 0.8


This parameter file is provided in the directory 'examples' from where we can execute the following command in order to run 
HYPERNOMAD on this search space

.. code-block:: sh

    ./hypernomad.exe parameter_file_mnist.txt
