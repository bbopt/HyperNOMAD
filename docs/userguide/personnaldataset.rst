****************************
Using a personnal dataset
****************************

In addition to the datasets embedded in hyperNOMAD, the user can choose to use a personnal dataset by specifying the following informations
in the parameter file:

.. code-block:: sh

    DATASET CUSTOM
    NUMBER_OF_CLASSES 20
    
    
When using a CUSTOM dataset, it is mandatory to provide hyperNOMAD with the number of classes.
The user is also responsible of plugging the 3 datasets (training, validation and testing) into the blackbox. In the file blackbox.py, 
the lines 80 to 84 must be completed with the adequate information.


.. code-block:: sh

    # Load the data
    print('> Preparing the data..')

    if dataset is not 'CUSTOM':
        dataloader = DataHandler(dataset, batch_size)
        image_size, number_classes = dataloader.get_info_data
        trainloader, validloader, testloader = dataloader.get_loaders()
    else:
        # Add here the adequate information
        image_size = None
        number_classes = None
        trainloader = None
        validloader = None
        testloader = None
        
 
The image size is a tuple of the form : (number_input_channels, length_image, width_image). In the case of MNIST, the image size is (1, 28, 28).

The trainload, validloader and testloader must be instances of 'torch.utils.data.dataloader.DataLoader'.
