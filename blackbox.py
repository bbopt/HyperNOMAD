# ------------------------------------------------------------------------------
#  HYPERNOMAD - Hyper-parameter optimization of deep neural networks with
#               NOMAD.
#
#
#
#  This program is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
#  for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#  You can find information on the NOMAD software at www.gerad.ca/nomad
# ------------------------------------------------------------------------------

import torch
import torch.optim as optim
import torch.utils.data
import torch.backends.cudnn as cudnn
import os
import sys
from datahandler import DataHandler
from evaluator import *
from neural_net import NeuralNet


# Read the inputs sent from HYPERNOMAD
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

print('> Reading the inputs..')
# get the dataset
dataset = str(sys.argv[1])

# Architecture
num_conv_layers = int(sys.argv[2])

shift = 0
list_param_conv_layers = []
for i in range(num_conv_layers):
    conv_layer_param = (int(sys.argv[3 + shift]), int(sys.argv[4 + shift]), int(sys.argv[5 + shift]),
                        int(sys.argv[6 + shift]), int(sys.argv[7 + shift]))
    list_param_conv_layers += [conv_layer_param]
    shift += 5

last_index = shift + 2
num_full_layers = int(sys.argv[last_index + 1])
list_param_full_layers = []
for i in range(num_full_layers):
    list_param_full_layers += [int(sys.argv[last_index + 2 + i])]

# First 2 : blackbox.py, dataset
batch_size_index = 2 + (2 + num_conv_layers*5 + num_full_layers)
batch_size = int(sys.argv[batch_size_index])

# HPs
optimizer_choice = int(sys.argv[batch_size_index + 1])
arg1 = float(sys.argv[batch_size_index + 2])               # lr
arg2 = float(sys.argv[batch_size_index + 3])               # momentum
arg3 = float(sys.argv[batch_size_index + 4])               # weight decay
arg4 = float(sys.argv[batch_size_index + 5])               # dampening
dropout_rate = float(sys.argv[batch_size_index + 6])
activation = int(sys.argv[batch_size_index + 7])

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

# Test if the correct information is passed - especially in the case of CUSTOM dataset
assert isinstance(trainloader, torch.utils.data.dataloader.DataLoader), 'Trainloader given is not of class DataLoader'
assert isinstance(validloader, torch.utils.data.dataloader.DataLoader), 'Validloader given is not of class DataLoader'
assert isinstance(testloader, torch.utils.data.dataloader.DataLoader), 'Testloader given is not of class DataLoader'
assert image_size is not None, 'Image size can not be None'
assert number_classes is not None, 'Total number of classes can not be None'

num_input_channels = image_size[0]

print('> Constructing the network')
# construct the network
cnn = NeuralNet(num_conv_layers, num_full_layers, list_param_conv_layers, list_param_full_layers,
                dropout_rate, activation, image_size[1], number_classes, num_input_channels)

cnn.to(device)

try:
    if optimizer_choice == 1:
        optimizer = optim.SGD(cnn.parameters(), lr=arg1, momentum=arg2, weight_decay=arg3,
                              dampening=arg4)
    if optimizer_choice == 2:
        optimizer = optim.Adam(cnn.parameters(), lr=arg1, betas=(arg2, arg3), weight_decay=arg4)
    if optimizer_choice == 3:
        optimizer = optim.Adagrad(cnn.parameters(), lr=arg1, lr_decay=arg2, weight_decay=arg4,
                                  initial_accumulator_value=arg3)
    if optimizer_choice == 4:
        optimizer = optim.RMSprop(cnn.parameters(), lr=arg1, momentum=arg2, alpha=arg3, weight_decay=arg4)
except ValueError:
    print('optimizer got an empty list')
    exit(0)

print(cnn)

# The evaluator trains and tests the network
evaluator = Evaluator(device, cnn, trainloader, validloader, testloader, optimizer, batch_size)
print('> Training')
best_val_acc, best_epoch = evaluator.train()
print('> Testing')
test_acc = evaluator.test()

# Output of the blackbox
print('> Final accuracy %.3f' % test_acc)
