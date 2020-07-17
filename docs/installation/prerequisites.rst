***************************
Prerequisites
***************************


In order to run HyperNOMAD correctly, please make sure to have:

* Python > 3.6
* PyTorch_.
* GCC > 3.8
* A compiled version of NOMAD_.


Additionally, \hypernomad has the following Python requirements:

    * Numpy
    * Matplotlib


Check that the requirements are fullfiled
============================================

Here are simple tests to check that everything is set correctly before installing HyperNOMAD.


Pytorch
--------

In order to test that Pytorch is correctly installed try the following command

.. code-block:: sh

    python
    >>> import torch
    

NOMAD
-------

Try running the following command

.. code-block:: sh

   nomad -info

For more help on the installation of NOMAD, please refer to the user_guide_.

.. _Pytorch: https://pytorch.org
.. _NOMAD: https://www.gerad.ca/nomad/
.. _user_guide: https://www.gerad.ca/nomad/Downloads/user_guide.pdf
