# ------------------------------------------------------------------------------
#  HYPERNOMAD - Hyper-parameter optimization of deep neural networks with
#		NOMAD.                                                  
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
import torchvision
import torchvision.transforms as transforms
import torch.optim as optim
import torch.utils.data
import torch.backends.cudnn as cudnn
import matplotlib.pyplot as plt
import numpy as np
import statistics
import copy
import random
import os
import sys
from neural_net import *
from utils import *

# Define device
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

print('Device:', device)

# Construct the network

# Example: python cifar10.py 13  64 3 1 1 0  64 3 1 1 1  128 3 1 1 0  128 3 1 1 1 256 3 1 1 0 256 3 1 1 0 256 3 1 1 1
# 512 3 1 1 0 512 3 1 1 0 512 3 1 1 1 512 3 1 1 0 512 3 1 1 0 512 3 1 1 1 3 4096 4096 10 128 0.1 0.9 5e-4 0 1

# Model
print('==> Building model..')
num_conv_layers = int(sys.argv[1])
shift = 0
list_param_conv_layers = []
for i in range(num_conv_layers):
    conv_layer_param = (int(sys.argv[2 + shift]), int(sys.argv[3 + shift]), int(sys.argv[4 + shift]),
                        int(sys.argv[5 + shift]), int(sys.argv[6 + shift]))
    list_param_conv_layers += [conv_layer_param]
    shift += 5

num_full_layers = int(sys.argv[2 + shift])
list_param_full_layers = []
for i in range(num_full_layers):
    list_param_full_layers += [int(sys.argv[3 + shift + i])]

print(list_param_conv_layers)

batch_size = int(sys.argv[3 + shift + i + 1])
optimizer_choice = int(sys.argv[3 + shift + i + 2])
arg1 = float(sys.argv[3 + shift + i + 3])       # lr
arg2 = float(sys.argv[3 + shift + i + 4])       # momentum
arg3 = float(sys.argv[3 + shift + i + 5])       # weight decay
arg4 = float(sys.argv[3 + shift + i + 6])
dropout_rate = float(sys.argv[3 + shift + i + 7])
activation = int(sys.argv[3 + shift + i + 8])

cnn = NeuralNet(num_conv_layers, num_full_layers, list_param_conv_layers, list_param_full_layers, dropout_rate,
                activation)
cnn.to(device)
print(cnn)

criterion = nn.CrossEntropyLoss()
try:
    if optimizer_choice == 1:
        optimizer = optim.SGD(cnn.parameters(), lr=arg1, momentum=arg2, weight_decay=arg3, dampening=arg4)
    if optimizer_choice == 2:
        optimizer = optim.Adam(cnn.parameters(), lr=arg1, betas=(arg2, arg3), weight_decay=arg4)
    if optimizer_choice == 3:
        optimizer = optim.Adagrad(cnn.parameters(), lr=arg1, lr_decay=arg2, weight_decay=arg4,
                                  initial_accumulator_value=arg3)
    if optimizer_choice == 4:
        optimizer = optim.RMSProp(cnn.parameters(), lr=arg1, momentum=arg2, alpha=arg3, weight_decay=arg4)
except ValueError:
    print('optimizer got an empty list')
    exit(0)

# Data
print('==> Preparing data..')
transform_train = transforms.Compose([
    transforms.RandomCrop(32, padding=4),
    transforms.RandomHorizontalFlip(),
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465), (0.2023, 0.1994, 0.2010)),
])

transform_test = transforms.Compose([
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465), (0.2023, 0.1994, 0.2010)),
])

trainset = torchvision.datasets.CIFAR10(root='./data', train=True, download=True, transform=transform_train)

n_valid = 40000
indices = list(range(len(trainset)))
random.shuffle(indices)

trainloader = torch.utils.data.DataLoader(trainset, batch_size=batch_size, shuffle=False,
                                          sampler=torch.utils.data.sampler.SubsetRandomSampler(indices[:n_valid]),
                                          num_workers=2)
validloader = torch.utils.data.DataLoader(trainset, batch_size=100,
                                          sampler=torch.utils.data.sampler.SubsetRandomSampler(indices[n_valid:]),
                                          num_workers=1)

testset = torchvision.datasets.CIFAR10(root='./data', train=False, download=True, transform=transform_test)
testloader = torch.utils.data.DataLoader(testset, batch_size=100, shuffle=False, num_workers=2)
classes = ('plane', 'car', 'bird', 'cat', 'deer', 'dog', 'frog', 'horse', 'ship', 'truck')

if torch.cuda.is_available():
    cnn = torch.nn.DataParallel(cnn)
    cudnn.benchmark = True


# Start training

epoch = 0
precedent_val_acc = -1
stop = False
l_val_acc = []
l_train_acc = []
best_val_acc = 0

while (not stop) and (epoch < 500):
    print('\nEpoch: %d' % epoch)
    cnn.train()
    train_loss = 0
    correct = 0
    total = 0
    for batch_idx, (inputs, targets) in enumerate(trainloader):
        inputs, targets = inputs.to(device), targets.to(device)
        optimizer.zero_grad()
        outputs = cnn(inputs)
        loss = criterion(outputs, targets)
        loss.backward()
        optimizer.step()

        train_loss += loss.item()
        _, predicted = outputs.max(1)
        total += targets.size(0)
        correct += predicted.eq(targets).sum().item()
        train_acc = 100.*correct/total

    l_train_acc.append(train_acc)

    cnn.eval()
    val_loss = 0
    val_correct = 0
    val_total = 0
    with torch.no_grad():
        for batch_idx, (inputs, targets) in enumerate(validloader):
            inputs, targets = inputs.to(device), targets.to(device)
            outputs = cnn(inputs)
            loss = criterion(outputs, targets)
            val_loss += loss.item()
            _, predicted = outputs.max(1)
            val_total += targets.size(0)
            val_correct += predicted.eq(targets).sum().item()
            val_acc = 100. * val_correct / val_total
    if val_acc > best_val_acc:
        best_val_acc = val_acc
        torch.save(cnn.state_dict(), 'best_model.pth')
    l_val_acc.append(val_acc)

    # Early stopping criteria
    if (epoch > 49) and (epoch % 50 == 0):
        l_train = l_train_acc[epoch-50:epoch]
        l_val = l_val_acc[epoch - 50:epoch]
        std_train = statistics.stdev(l_train)
        std_val = statistics.stdev(l_val)
        if std_train < 0.001:
            stop = True
        if (std_train > 0.001) and (std_val < 0.001):
            stop = True

    if epoch % 100 == 0:
        for param_group in optimizer.param_groups:
            param_group['lr'] = arg1 * 0.8

    print("Epoch {}/{},  Train accuracy: {:.3f}, Val accuracy: {:.3f}".format(epoch + 1, 500, train_acc, val_acc))
    epoch += 1

print('> Finished Training')

# get the best validation accuracy and the corresponding epoch
best_epoch = np.argmax(l_val_acc)
best_val_acc = l_val_acc[best_epoch]

# use the saved net to assess the test accuracy
print('Best validation accuracy and corresponding epoch number : {:.3f}/{}'.format(best_val_acc, best_epoch+1))
best_model = NeuralNet(num_conv_layers, num_full_layers, list_param_conv_layers, list_param_full_layers, dropout_rate,
                       activation)

if torch.cuda.is_available():
    best_model = torch.nn.DataParallel(best_model)
    cudnn.benchmark = True

best_model.load_state_dict(torch.load('best_model.pth'))

total_test = 0
correct_test = 0
best_model.eval()
test_loss = 0
for batch_idx, (inputs, targets) in enumerate(testloader):
    inputs, targets = inputs.to(device), targets.to(device)
    outputs = best_model(inputs)
    loss = criterion(outputs, targets)
    test_loss += loss.item()
    _, predicted = outputs.max(1)
    total_test += targets.size(0)
    correct_test += predicted.eq(targets).sum().item()
    test_acc = 100. * correct_test / total_test

print('> Final accuracy %.3f' % test_acc)

# plot training and validation accuracy
# x_axis = range(epoch)
# plt.plot(x_axis, l_train_acc, 'r', label='Training acc')
# plt.plot(x_axis, l_val_acc, 'b', label='Validation acc')
# plt.show()

exit(0)
