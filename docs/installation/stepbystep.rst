***************************
Step by step installation of HyperNOMAD
***************************

The installation of HyperNOMAD can start once the package is downloaded and the prerequisites installed. The package contains a Makefile responsible for builinding the binaries. To start the installation, you need to execute the following command:

.. code-block:: sh

    make
        building HyperNOMAD ...

        To be able to run the example
        the HYPERNOMAD_HOME environment variable
        must be set to path_to_home_directory_of_hypernomad
    
When the compilation is successful, a message appears asking to set an environment variable 'HYPERNOMAD_HOME'. This can be done by adding a line in the file .profile or .bashrc :

.. code-block:: sh

    export HYPERNOMAD_HOME=path_to_home_directory_of_hypernomad
    

Check that the installation is successful
============================================


The executable hypernomad.exe is located in the bin directory. You can check that the installation is successful by trying to run the commad

.. code-block:: sh

    $HYPERNOMAD_HOME/bin/./hypernomad.exe -i
    
which should return the following informations:


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
