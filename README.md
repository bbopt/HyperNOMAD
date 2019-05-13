# A tutorial on hyperparameter optimization of deep neural networks with HYPERNOMAD

HYPERNOMAD is a Python package dedicated to the hyperparameter optimization of deep neural networks. The package contains a blackbox specifically designed for this problematic and provides a link with the NOMAD software used for the optimization. The blackbox takes as inputs a list of hyperparameters, builds a corresponding deep neural network in order to train, validate and test it on a specific data set before returning the test error as a mesure of performance. NOMAD is then used to minimize this error. The following appendix provides an overview of how to use the HYPERNOMAD package.

The following tutorial shows the different steps to take in order to run HYPERNOMAD on a first example. The complete functionalities of HYPERNOMAD are described in the [documentation](https://hypernomad.readthedocs.io/en/latest/).

## Prerequisites

In order to run HYPERNOMAD correctly, please make sure to have:

* A compiled version of [NOMAD](https://www.gerad.ca/nomad/).
* Python > 3.6
* [PyTorch](https://pytorch.org/)
* GCC > 3.8


## Getting Started

First build the executable by running the following command.

```
make
```

The next phase of to create a parameter file that contains the necessary informations to specify the classification problem, the search space and the initial starting point. Here is an example of a parameter file designed for the MNIST problem. The number of convolutional layers is fixed to 5, which means that this value will not change during the optimization, and it bounds are not changed from the default values. On the other hand, the dropout rate is allowed to vary between [0.3, 0.8] instead of the default range of [0, 1] and is initialized at 0.6


```
# Mandatory information
DATASET  		    MNIST
MAX_BB_EVAL 		100

# Optional information
NUM_CON_LAYERS 		5  -  -  FIXED
KERNELS 		      3
NUM_FULL_LAYERS		6
ACTIVATION 		    2
DROUPOUT_RATE 		0.6  0.3 0.8
```

## Running an optimization

The optimization starts by executing the command:

```
./hypernomad.exe parameter_file.txt
```

