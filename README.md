# HYPERNOMAD
An interface for hyper-parameter optimization with NOMAD.

# A tutorial on Hyper-parameter Optimization of deep neural networks with the NOMAD software

The following tutorial presents a step by step guide in order to build a project for optimizing the hyper-parameters of a deep neurla network using NOMAD. This is a derivative free optimization software that is designed to optimize constrained, single or bi-objective derivative black-boxes. NOMAD has the capacity to handle mized variable problems: real, integer and categorical.


## Prerequisites

First, start with installing the latest version of:

* [NOMAD](https://www.gerad.ca/nomad/)

In order to execute the examples, you will also need to have:

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
