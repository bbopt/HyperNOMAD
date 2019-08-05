*****
# Hyperparameter optimization of deep neural networks with HyperNOMAD
*****

HyperNOMAD is a C++ and Python package dedicated to the hyperparameter optimization of deep neural networks. The package contains a blackbox specifically designed for this problematic and provides a link with the NOMAD software used for the optimization. The blackbox takes as inputs a list of hyperparameters, builds a corresponding deep neural network in order to train, validate and test it on a specific data set before returning the test error as a mesure of performance. NOMAD is then used to minimize this error. The following appendix provides an overview of how to use the HyperNOMAD package.

The following tutorial shows the different steps to take in order to run HyperNOMAD on a first example. The complete functionalities of HyperNOMAD are described in the [documentation](https://hypernomad.readthedocs.io/en/latest/).

## Prerequisites

In order to run HyperNOMAD correctly, please make sure to have:

* A compiled version of [NOMAD](https://www.gerad.ca/nomad/).
* Python > 3.6
* [PyTorch](https://pytorch.org/)
* GCC > 3.8


## installation of HyperNOMAD

First build the executable by running the following command.

```
make

    building HyperNOMAD ...

    To be able to run the example
    the HYPERNOMAD_HOME environment variable
    must be set to the HyperNOMAD home directory
    
```

When the compilation is successful, a message appears asking to set an environment variable 'HYPERNOMAD_HOME'. This can be done by adding a line in the file .profile or .bashrc :


```
    export HYPERNOMAD_HOME=hypernomad_directory
```    

The executable hypernomad.exe is located in the bin directory. You can check that the installation is successful by trying to run the commad

```
    $HYPERNOMAD_HOME/bin/./hypernomad.exe -i
```    

which should return the following informations:

```
    --------------------------------------------------
      HyperNomad - version 1.0
    --------------------------------------------------
      Using Nomad version 3.9.0 - www.gerad.ca/nomad
    --------------------------------------------------

    Run           : hypernomad.exe parameters_file
    Info          : hypernomad.exe -i
    Help          : hypernomad.exe -h
    Version       : hypernomad.exe -v
    Usage         : hypernomad.exe -u
    Neighboors    : hypernomad.exe -n parameters_file
```

## Getting started

The next phase is to create a parameter file that contains the necessary informations to specify the classification problem, the search space and the initial starting point. HyperNOMAD allows for a good flexibility of tuning a convolutional network by considering multiple aspects of a network at once such as the architecture, the dropout rate, the choice of the optimizer and the hyperparameters related to the optimization aspect (learning rate, weight decay, momentum, ...), the batch size, etc. The user can choose to optimize all these aspects or select a few and fixe the others to certain values. The user can also change the default range of each hyperparameter. 

This information is passed through the parameter file by using a specific synthax:

```
  KEYWORD   INITIAL_VALUE   LOWER_BOUND   UPPER_BOUND   FIXED/VAR
```

Here is an example of an acceptable parameter file. First, the dataset MNIST is choosen and we specify that HyperNOMAD is allowed to try a maximum of 100 configurations. Then, the number of convolutional layers is fixed throught the optimization to 5, the two '-' appearing after the '5' mean that the default lower and upper bounds are not changed. The kernels, number of fully connected layers and activation function are respectively initialized at 3, 6, and 2 (Sigmoid) and the dropout rate is initialized at 0.6 with a new lower bound of 0.3 and upper bound of 0.8

Finally, all the remaining hyperparameters that are not explicitly mentioned in this file are fixed to their default values during the optimization.


```
DATASET                 MNIST
MAX_BB_EVAL             100

# Optional information
NUM_CON_LAYERS          5  -  -  FIXED # The initial value is fixed
                                       # lower and upper bounds have no
                                       # influence when parameter 
                                       # is fixed.

KERNELS                 3              # Only the initial value is set (not fixed)
                                       # the lower bound and upper bound
                                       # have default values.

NUM_FC_LAYERS           6
ACTIVATION_FUNCTION     2
DROPOUT_RATE            0.6  0.3 0.8  # The lower and upper bounds 
                                      # are set to values that are not 
                                      # the default ones
REMAINING_HPS           FIXED
```

More details are provided in the user guide section of the [documentation](https://hypernomad.readthedocs.io/en/latest/).


## Running an optimization

The optimization starts by executing the command:

```
$HYPERNOMAD_HOME/bin/./hypernomad.exe parameter_file.txt
```

Multiple examples of parameter files are provided in the folder examples. One uses CIFAR-10 starting from the default starting point and the other use MNIST with different configurations.

To use these files, the cammand is:

```
$HYPERNOMAD_HOME/bin/./hypernomad.exe $HYPERNOMAD_HOME/examples/cifar10_default.txt
```
or 


```
$HYPERNOMAD_HOME/bin/./hypernomad.exe $HYPERNOMAD_HOME/examples/mnist_fc_optim.txt
```

### Citing HyperNOMAD

If you use HyperNOMAD, please cite the following [paper](https://arxiv.org/abs/1907.01698).


```
@article{DBLP:journals/corr/abs-1907-01698,
  author    = {Dounia Lakhmiri and
               S{\'{e}}bastien Le Digabel and
               Christophe Tribes},
  title     = {HyperNOMAD: Hyperparameter optimization of deep neural networks using
               mesh adaptive direct search},
  journal   = {CoRR},
  volume    = {abs/1907.01698},
  year      = {2019},
  url       = {http://arxiv.org/abs/1907.01698},
  archivePrefix = {arXiv},
  eprint    = {1907.01698},
  timestamp = {Mon, 08 Jul 2019 14:12:33 +0200},
  biburl    = {https://dblp.org/rec/bib/journals/corr/abs-1907-01698},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}

```
