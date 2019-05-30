***************************
Step by step installation of HYPERNOMAD
***************************

The installation of HYPERNOMAD can start once the package is downloaded and the prerequisites installed. The package contains a Makefile responsible for builinding the binaries. To start the installation, you need to execute the following command:

.. code-block:: sh

    make

Check that the installation is successful
============================================

The executable hypernomad.exe is located in the bin directory. You can check that the installation is successful by trying to run the commad

.. code-block:: sh

    (path_to_bin)/hypernomad.exe -i
    
This should return the following informations:


.. code-block:: sh

    --------------------------------------------------
      HyperNomad - version 1.0
    --------------------------------------------------
      Using Nomad version 3.9.0 - www.gerad.ca/nomad
    --------------------------------------------------

    Run           : hypernomad.exe hyperparameters_file
    Info          : hypernomad.exe -i
    Help          : hypernomad.exe -h
    Version       : hypernomad.exe -v
    Usage         : hypernomad.exe -u
    Neighboors    : hypernomad.exe -n hyperparameters_file
