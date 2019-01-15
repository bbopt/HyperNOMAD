
# A tutorial on hyper-parameter optimization of deep neural networks with HYPERNOMAD

The following tutorial presents a step by step guide in order to build a project for optimizing the hyper-parameters of a deep neurla network using NOMAD. It is a derivative free optimization software that is designed to optimize constrained, single or bi-objective derivative blackboxes. NOMAD has the capacity to handle mixed variable problems: real, integer and categorical.


## Prerequisites

First, start with installing the latest version of:

* [NOMAD](https://www.gerad.ca/nomad/)

Reading the [user guide](https://www.gerad.ca/nomad/Downloads/user_guide.pdf) is strongly recommended to familiarize with this software along with the examples given with the NOMAD package.

In order to execute the examples, you will also need:

* [PyTorch](https://pytorch.org/)

## Getting Started

### Creating the black-box

The black-box is the file that will take the hyper-parameters as inputs, and will return the value of the objective and/or constraints as outputs.

In order to make the black-box compatible with NOMAD, you have to make sure that you can execute the following command:

```
.\bb.py param_1 param_2 ... param_n
```

Where 

* bb.py is the black-box. Note that you can also code it in C++.
* param_1 .. param_n are the hyper-parameters you want to optimize.

### Link with NOMAD

With the black-box done, it is now time to link it to NOMAD.

First, we start with the file 'param_test1.txt' where you need to indicate the black-box you want to optimize. The following line sais that the black-box is written in Python and the file is 'pytorch_bb.py'.

```
BB_EXE "$python ./pytorch_bb.py"
```

The entry 'BB_INPUT_TYPE' defines the type of each variable.

* C : categorical variables.
* I : integar variables.
* R : real variables.

The rest of the entries specify the following informations:

* 'DIMENSION' : dimension of the optimization problem.
* 'x0' : the starting point.
* 'lower_bound' and 'upper_bound'.
* 'BB_OUTPUT_TYPE' : the output of the black-box. Here, the black-box returns one value corresponding to the objective function.
* 'MAX_BB_EVAL' : maximum number of black-box evaluations.


### Dealing with categorical variables

One of the advantages of NOMAD is its ability to deal with categorical variables. NOMAD also allows to defines a neighborhood structure for each categorical variable. These neighbohrs can be located in a different search space than the current point, and it is up to the user to define the neighbors and their search spaces for each categorical variable. This is done in the file 'pytorch_cat.cpp' with the function: 

```
void My_Extended_Poll::construct_extended_points ( const Eval_Point & x)
```

In our example, each convolutional layer is defined with 5 variables:

* The number of output channels.
* The kernel size.
* The stride.
* The padding.
* Whether to do a pooling operation or not.

Therefore, adding a convolutional layer means adding 5 other variables thus changing the dimension of the problem and the search space. 

The same idea applies to the number of fully connected layers. These two values correspond to the only 2 categorical variables in our example.

## Running an optimization

The optimization starts by executing the command 

```
./pytorch_cat.exe
```

The stats are displayed according to the 'DISPLAY_DEGREE' chosen within the parameter file. It can be increased to 3 for more details about the steps of NOMAD. The details of the execution can also be saved in the file specified for the tag 'HISTORY_FILE'.


