# ------------------------------------------------------------------------------
#  HYPERNOMAD - Hyper-parameter optimization of deep neural networks with
#		        NOMAD.
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
import statistics
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import time
import os
import sys
from neural_net import *
from datahandler import *

sys.path.append(os.environ.get('HYPERNOMAD')+"/src/blackbox/blackbox")


class Evaluator(object):
    def __init__(self, device, cnn, trainloader, validloader, testloader, optimizer, batch_size, dataset):
        self.__device = device
        self.__cnn = cnn
        self.__trainloader = trainloader
        self.__validloader = validloader
        self.__testloader = testloader
        self.__batch_size = batch_size
        self.__optimizer = optimizer
        self.__dataset = dataset
        self.__train_acc = None
        self.__val_acc = None
        self.__test_acc = None
        self.__best_epoch = None

    @property
    def device(self):
        return self.__device

    @property
    def cnn(self):
        return self.__cnn

    @cnn.setter
    def cnn(self, new_cnn):
        self.__cnn = new_cnn

    @property
    def trainloader(self):
        return self.__trainloader

    @property
    def validloader(self):
        return self.__validloader

    @property
    def testloader(self):
        return self.__testloader

    @property
    def batch_size(self):
        return self.__batch_size

    @property
    def optimizer(self):
        return self.__optimizer

    @property
    def dataset(self):
        return self.__dataset

    def train(self):
        criterion = nn.CrossEntropyLoss()

        if torch.cuda.is_available():
            self.cnn = torch.nn.DataParallel(self.cnn)
            cudnn.benchmark = True

        epoch = 0
        stop = False
        l_val_acc = []
        l_train_acc = []
        best_val_acc = 0
        epochs = []
        max_epochs = 500
        if self.dataset =='MINIMNIST':
            max_epochs = 50

        # plots
        fig = plt.figure()
        ax1 = fig.add_subplot(111)
        ax1.set_title('Training and validation accuracies')
        ax1.set_xlabel('Number of epochs')
        ax1.set_ylabel('Accuracies')
        plt.ion()

        while (not stop) and (epoch < max_epochs):
            self.cnn.train()
            train_loss = 0
            correct = 0
            total = 0
            for batch_idx, (inputs, targets) in enumerate(self.trainloader):
                inputs, targets = inputs.to(self.device), targets.to(self.device)
                self.optimizer.zero_grad()
                outputs = self.cnn(inputs)
                loss = criterion(outputs, targets)
                loss.backward()
                self.optimizer.step()

                train_loss += loss.item()
                _, predicted = outputs.max(1)
                total += targets.size(0)
                correct += predicted.eq(targets).sum().item()
                self.__train_acc = 100. * correct / total

            l_train_acc.append(self.__train_acc)

            self.cnn.eval()
            val_loss = 0
            val_correct = 0
            val_total = 0
            with torch.no_grad():
                for batch_idx, (inputs, targets) in enumerate(self.validloader):
                    inputs, targets = inputs.to(self.device), targets.to(self.device)
                    outputs = self.cnn(inputs)
                    loss = criterion(outputs, targets)
                    val_loss += loss.item()
                    _, predicted = outputs.max(1)
                    val_total += targets.size(0)
                    val_correct += predicted.eq(targets).sum().item()
                    self.__val_acc = 100. * val_correct / val_total
            if self.__val_acc > best_val_acc:
                best_val_acc = self.__val_acc
                torch.save(self.cnn.state_dict(), 'best_model.pth')
            l_val_acc.append(self.__val_acc)

            epochs.append(epoch)

            ax1.plot(epochs, l_train_acc, color='r', label='Training accuracy')
            ax1.plot(epochs, l_val_acc, color='b', label='Validation accuracy')
            handles, labels = ax1.get_legend_handles_labels()
            handle_list, label_list = [], []
            for handle, label in zip(handles, labels):
                if label not in label_list:
                    handle_list.append(handle)
                    label_list.append(label)
            ax1.legend(handle_list, label_list, loc='best')
            # ax1.legend(loc='best')

            plt.pause(1e-6)

            # Stop early
            if (epoch == 25) and (best_val_acc < 20):
                stop = True

            # Early stopping criteria
            if (epoch > 49) and (epoch % 50 == 0):
                l_train = l_train_acc[epoch - 50:epoch]
                l_val = l_val_acc[epoch - 50:epoch]
                std_train = statistics.stdev(l_train)
                std_val = statistics.stdev(l_val)
                if std_train < 0.001:
                    stop = True
                if (std_train > 0.001) and (std_val < 0.001):
                    stop = True

            if epoch == 100:
                for param_group in self.optimizer.param_groups:
                    param_group['lr'] /= 10

            if (epoch > 101) and (epoch % 100 == 0):
                for param_group in self.optimizer.param_groups:
                    param_group['lr'] /= 5

            print("Epoch {},  Train accuracy: {:.3f}, Val accuracy: {:.3f}".format(epoch + 1, self.__train_acc,
                                                                                   self.__val_acc))
            epoch += 1

        print('> Finished Training')

        # get the best validation accuracy and the corresponding epoch
        best_epoch = np.argmax(l_val_acc)
        best_val_acc = l_val_acc[best_epoch]

        # use the saved net to assess the test accuracy
        print('Best validation accuracy and corresponding epoch number : {:.3f}/{}'.format(best_val_acc, best_epoch + 1))
        return best_val_acc, best_epoch

    def test(self):
        criterion = nn.CrossEntropyLoss()
        self.cnn.load_state_dict(torch.load('best_model.pth'))

        total_test = 0
        correct_test = 0
        self.cnn.eval()
        test_loss = 0
        for batch_idx, (inputs, targets) in enumerate(self.testloader):
            inputs, targets = inputs.to(self.device), targets.to(self.device)
            outputs = self.cnn(inputs)
            loss = criterion(outputs, targets)
            test_loss += loss.item()
            _, predicted = outputs.max(1)
            total_test += targets.size(0)
            correct_test += predicted.eq(targets).sum().item()
            test_acc = 100. * correct_test / total_test

        # exit(0)
        return test_acc


if __name__ == '__main__':
    ev = Evaluator()
