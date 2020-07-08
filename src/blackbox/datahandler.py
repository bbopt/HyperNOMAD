# ------------------------------------------------------------------------------
#  HyperNOMAD - Hyper-parameter optimization of deep neural networks with
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
import torch.utils.data
import random
import os
import sys

sys.path.append(os.environ.get('HYPERNOMAD_HOME')+"/src/blackbox/blackbox")


class DataHandler(object):
    def __init__(self, dataset, batch_size):
        self.__dataset = dataset
        available_datasets = ['MNIST', 'Fashion-MNIST', 'KMNIST', 'EMNIST', 'CIFAR10', 'CIFAR100', 'STL10', 'SVHN',
                              'CUSTOM', 'MINIMNIST']
        self.__batch_size = batch_size
        self.__transform_train = None
        self.__transform_test = None
        assert dataset in available_datasets, 'Choose a valid dataset \n'

    @property
    def dataset(self):
        return self.__dataset

    @property
    def batch_size(self):
        return self.__batch_size

    @property
    def transform_train(self):
        return self.__transform_train

    @transform_train.setter
    def transform_train(self, t_train):
        self.__transform_train = t_train

    @property
    def transform_test(self):
        return self.__transform_test

    @transform_test.setter
    def transform_test(self, t_test):
        self.__transform_test = t_test

    @property
    def get_mean_and_std(self):
        """Compute the mean and std value of dataset."""

        dataloader = self.get_loaders()
        image_size, num_classes = self.get_info_data
        input_channels = image_size[0]
        mean = torch.zeros(input_channels)
        std = torch.zeros(input_channels)
        print('==> Computing mean and std..')

        for inputs, targets in dataloader:
            for i in range(input_channels):
                mean[i] += inputs[:, i, :, :].mean()
                std[i] += inputs[:, i, :, :].std()
        mean.div_(len(dataset))
        std.div_(len(dataset))
        return mean, std

    @property
    def get_info_data(self):
        """Get the size of the images and number of classes for each dataset"""
        image_size = None
        total_number_classes = 0
        if self.dataset in ['MINIMNIST', 'MNIST', 'Fashion-MNIST', 'KMNIST', 'EMNIST']:
            image_size = (1, 28, 28)
            total_number_classes = 10
        if self.dataset in ['CIFAR10', 'SVNH']:
            image_size = (3, 32, 32)
            total_number_classes = 10
        if self.dataset == 'CIFAR100':
            image_size = (3, 32, 32)
            total_number_classes = 100
        if self.dataset == 'STL10':
            image_size = (3, 96, 96)
            total_number_classes = 10
        return image_size, total_number_classes

    def get_loaders(self):
        trainloader = None
        validloader = None
        testloader = None
        root = os.path.join('./data', self.dataset)

        if self.dataset == 'MINIMNIST':
            print(">>> Preparing the simplifed MNIST dataset...")
            transform_train = transforms.Compose([transforms.ToTensor(),
                                                  transforms.Normalize((0.1307,), (0.3081,)), ])
            trainset = torchvision.datasets.MNIST(root=root, train=True, download=True, transform=transform_train)
            testset = torchvision.datasets.MNIST(root=root, train=False, download=True, transform=transform_train)

            n_train = 300
            n_valid = 100
            indices = list(range(len(trainset)))
            random.shuffle(indices)

            indices_test = list(range(len(testset)))
            random.shuffle(indices_test)

            train_sampler = torch.utils.data.sampler.SubsetRandomSampler(indices[:n_train])
            valid_sampler = torch.utils.data.sampler.SubsetRandomSampler(indices[60000-n_valid:])
            test_sampler = torch.utils.data.sampler.SubsetRandomSampler(indices_test[:n_valid])

            trainloader = torch.utils.data.DataLoader(trainset, batch_size=self.batch_size, num_workers=12,
                                                      sampler=train_sampler)
            validloader = torch.utils.data.DataLoader(trainset, batch_size=100, sampler=valid_sampler, num_workers=12)
            testloader = torch.utils.data.DataLoader(testset, batch_size=100, sampler=test_sampler, num_workers=12)

        if self.dataset in ['MNIST', 'Fashion-MNIST', 'KMNIST', 'EMNIST']:
            if self.dataset == 'MNIST':
                print(">>> Preparing MNIST dataset...")
                transform_train = transforms.Compose([transforms.ToTensor(),
                                                      transforms.Normalize((0.1307,), (0.3081,)), ])
                trainset = torchvision.datasets.MNIST(root=root, train=True, download=True, transform=transform_train)
                testset = torchvision.datasets.MNIST(root=root, train=False, download=True, transform=transform_train)
            if self.dataset == 'Fashion-MNIST':
                print(">>> Preparing Fashion-MNIST dataset...")
                transform_train = transforms.Compose([transforms.ToTensor(),
                                                      transforms.Normalize((0.1307,), (0.3081,)), ])
                trainset = torchvision.datasets.FashionMNIST(root, train=True, transform=transform_train,
                                                             target_transform=None, download=True)
                testset = torchvision.datasets.FashionMNIST(root, train=False, transform=transform_train,
                                                            target_transform=None, download=True)
            if self.dataset == 'KMNIST':
                print(">>> Preparing KMNIST dataset...")
                transform_train = transforms.Compose([transforms.ToTensor(),
                                                      transforms.Normalize((0.1307,), (0.3081,)), ])
                trainset = torchvision.datasets.KMNIST(root, train=True, transform=transform_train,
                                                       target_transform=None, download=True)
                testset = torchvision.datasets.KMNIST(root, train=False, transform=transform_train,
                                                      target_transform=None, download=True)
            if self.dataset == 'EMNIST':
                print(">>> Preparing EMNIST dataset...")
                transform_train = transforms.Compose([transforms.ToTensor(),
                                                      transforms.Normalize((0.1307,), (0.3081,)), ])
                trainset = torchvision.datasets.EMNIST(root, train=True, transform=transform_train,
                                                       target_transform=None, download=True)
                testset = torchvision.datasets.EMNIST(root, train=False, transform=transform_train,
                                                      target_transform=None, download=True)

            n_valid = 40000
            indices = list(range(len(trainset)))
            random.shuffle(indices)
            train_sampler = torch.utils.data.sampler.SubsetRandomSampler(indices[:n_valid])
            valid_sampler = torch.utils.data.sampler.SubsetRandomSampler(indices[n_valid:])

            trainloader = torch.utils.data.DataLoader(trainset, batch_size=self.batch_size, num_workers=12,
                                                      sampler=train_sampler)
            validloader = torch.utils.data.DataLoader(trainset, batch_size=100, sampler=valid_sampler, num_workers=12)
            testloader = torch.utils.data.DataLoader(testset, batch_size=100, shuffle=True, num_workers=12)

        if self.dataset in ['CIFAR10', 'CIFAR100']:

            transform_train = transforms.Compose([transforms.RandomCrop(32, padding=4),
                                                  # transforms.RandomHorizontalFlip(),
                                                  transforms.ToTensor(),
                                                  transforms.Normalize([0.4914, 0.4822, 0.4465],
                                                                       [0.2023, 0.1994, 0.2010]),
                                                  ])
            transform_test = transforms.Compose([transforms.ToTensor(),
                                                 transforms.Normalize([0.4914, 0.4822, 0.4465],
                                                                      [0.2023, 0.1994, 0.2010]),
                                                 ])
            if self.dataset == 'CIFAR10':
                print(">>> Preparing CIFAR-10 dataset...")
                trainset = torchvision.datasets.CIFAR10(root=root, train=True, download=True,
                                                        transform=transform_train)
                testset = torchvision.datasets.CIFAR10(root=root, train=False, download=True,
                                                       transform=transform_test)
            else:
                print(">>> Preparing CIFAR-100 dataset...")
                trainset = torchvision.datasets.CIFAR100(root=root, train=True, download=True,
                                                         transform=transform_train)
                testset = torchvision.datasets.CIFAR100(root=root, train=False, download=True,
                                                        transform=transform_test)
            n_valid = 40000
            indices = list(range(len(trainset)))
            random.shuffle(indices)

            trainloader = torch.utils.data.DataLoader(trainset, batch_size=self.batch_size, shuffle=False,
                                                      sampler=torch.utils.data.sampler.SubsetRandomSampler(
                                                          indices[:n_valid]), num_workers=12)
            validloader = torch.utils.data.DataLoader(trainset, batch_size=100,
                                                      sampler=torch.utils.data.sampler.SubsetRandomSampler(
                                                          indices[n_valid:]), num_workers=12)
            testloader = torch.utils.data.DataLoader(testset, batch_size=100, shuffle=False, num_workers=12)

        if self.dataset == 'STL10':
            print(">>> Preparing STL10 dataset...")
            trainset = torchvision.datasets.STL10(root, train=True, transform=None, target_transform=None,
                                                  download=True)
            testset = torchvision.datasets.STL10(root, train=False, transform=None, target_transform=None,
                                                 download=True)
            
            n_valid = 40000
            indices = list(range(len(trainset)))
            random.shuffle(indices)

            trainloader = torch.utils.data.DataLoader(trainset, batch_size=self.batch_size, shuffle=False,
                                                      sampler=torch.utils.data.sampler.SubsetRandomSampler(
                                                          indices[:n_valid]), num_workers=12)
            validloader = torch.utils.data.DataLoader(trainset, batch_size=100,
                                                      sampler=torch.utils.data.sampler.SubsetRandomSampler(
                                                          indices[n_valid:]), num_workers=12)
            testloader = torch.utils.data.DataLoader(testset, batch_size=100, shuffle=False, num_workers=12)

        return trainloader, validloader, testloader


if __name__ == '__main__':
    dl = DataHandler('Fashion-MNIST', 128)
    image_size, num_classes = dl.get_info_data
    mean, std = dl.get_mean_and_std
    print(mean)
    print(std)
