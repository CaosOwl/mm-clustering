#main 
import numpy as np
import pandas as pd
import time
#keras
import keras
import tensorflow as tf
from keras.models import Sequential
from keras import Input
from keras import Model
from keras.layers import Dense# Neural network
import keras.backend as K
#sklearn
from sklearn.preprocessing import OneHotEncoder
from sklearn.preprocessing import StandardScaler, MinMaxScaler
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
#mathplotlib
import matplotlib
matplotlib.use('pdf')
import matplotlib.pyplot as plt
from scipy.stats import norm
import matplotlib.mlab as mlab

#reading with h5
import h5py
#parser
import argparse

'''
old reading of data from simple .txt

dataset = pd.read_csv("generated_data_1mio.txt", delim_whitespace=True) #You need to change #directory accordingly
dataset = dataset.head(100000) #Return 10 rows of data
#Changing pandas dataframe to numpy array
multiplexed_data = dataset[dataset.columns[:61]].values
solution_data = dataset[dataset.columns[61:]].values
'''


##########################################

def extract_data(chosen_MM, header, data): ### Fct for extracting the right data and storing it in a list
    chosen_data = []
    chosen_index = header.index(chosen_MM)
    j=0    
    while(j < len(data)):
        chosen_data.append(data[j][chosen_index])
        j+=1

    return np.asarray(chosen_data)

def extract_solution(chosen_MM, header, data): ### Fct for extracting the right data and storing it in a list
    chosen_data = []
    chosen_index = header.index(chosen_MM)
    j=0    
    while(j < len(data)):
        chosen_data.append(data[j][chosen_index][0:2])
        j+=1

    return np.asarray(chosen_data)

######

def select_hits(chosen_data, chosen_solution): ### Fct for only selecting data with actual hits, others get rejected
    
    return_data = []
    return_solution = []

    i=0
    while (i<len(chosen_solution)):
        
        if (abs(chosen_solution[i][0] - chosen_solution[i][1]) > 10): #hit!, -1000 is no hit, rejected
            return_data.append(chosen_data[i])
            return_solution.append(chosen_solution[i])

        i+=1

    return return_data, return_solution	

######

def merging_hits(chosen_data, chosen_solution): #fct for merging two single hit events to double hit events
                                                #so far taking pairs and merging, means dataset gets half of original amount of entries!
    return_data = []
    return_solution = []


    i=0
    while (i<len(chosen_solution)):
        temp_list = []
        try:
            return_data.append(np.add(chosen_data[i],chosen_data[i+1]))  
            temp_list.append(chosen_solution[i])
            temp_list.append(chosen_solution[i+1])
            return_solution.append(np.asarray(temp_list)) 
            i = i + 2

        except IndexError:
            i = i + 2
            continue

    return return_data, return_solution

def kill_empty_solutions(solution, amount):
    
    return_solution = []
    for entry in solution:
        temp = []
        i=0
        while(i<amount):
            temp.append(entry[i])
            i += 1
        return_solution.append(temp)

    return return_solution


#########################################

parser = argparse.ArgumentParser(
    description='Plot decay time, length and boosed length of DM.',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter
)

parser.add_argument("-i", "--inputfile", type=str, default='test.h5', help='input file to run the NN over')
parser.add_argument("-o", "--outname", type=str, default='multiple_ch64_900k_200epoch_NA64style_mindist20_V2.h5', help='name of the output file')
parser.add_argument("-nd", "--ndata", type=int, default=100000, help='amount of data to read in the model')
parser.add_argument("-ne", "--epochs", type=int, default=200, help='Number of epochs')
parser.add_argument("-op", "--optimizer", type=str, default="adam", help='optimizer of the model')

args = parser.parse_args()

with h5py.File(args.inputfile, 'r') as f:

    keyword = list(f.keys())[0] #select first keyword - in our case ClusterTree
    
    #extracting headers, names of the histos in ClusterTree
    i=0
    header = [] #list with histo names
    while True:
        search_string = "FIELD_"+str(i)+"_NAME"
        name = f[keyword].attrs.get(search_string)
        if(name == None):
            break
        header.append(name.decode("utf-8")) #list
        i += 1
    print(header)

    #extracting data
    data = np.asarray(f[keyword][:args.ndata]).tolist() #extracting as numpy.void, converting in numpy.ndarray and then to list...

    #leaving the with, closing file

#from choosing which data, e.g. MM1X, we get right data from list
chosen_MM = "ChanOutput"
chosen_MM_solution = "TruePosition"
chosen_data = extract_data(chosen_MM, header, data)
chosen_solution = extract_solution(chosen_MM_solution, header, data)

#merge data two dataset with double hits
merged_hit_data, merged_hit_solution = chosen_data, chosen_solution#merging_hits(hit_data, hit_solution)

#we need numpy arrays as input...
merged_hit_data = np.asarray(merged_hit_data)
merged_hit_solution = np.asarray(merged_hit_solution)

#normalizing ADC counts for input
scaler1 = StandardScaler()
merged_hit_data = scaler1.fit_transform(merged_hit_data)

scaler2 = StandardScaler()
merged_std = np.std(merged_hit_solution)
merged_mean = np.mean(merged_hit_solution)
merged_hit_solution = scaler2.fit_transform(merged_hit_solution)

multiplexed_train,multiplexed_test,solution_train,solution_test = train_test_split(merged_hit_data,merged_hit_solution,test_size = 0.1)

first_solution_train = []
second_solution_train = []

first_solution_test = []
second_solution_test = []

for arrays in solution_train:
    arrays.sort()
    first_solution_train.append(arrays[0])
    second_solution_train.append(arrays[1])

for arrays in solution_test:
    arrays.sort()
    first_solution_test.append(arrays[0])
    second_solution_test.append(arrays[1])

first_solution_train = np.asarray(first_solution_train)
second_solution_train = np.asarray(second_solution_train)
first_solution_test = np.asarray(first_solution_test)
second_solution_test = np.asarray(second_solution_test)


#creating training and validating sets

def fancy_loss(y_true, y_pred):

    return K.std(y_true-y_pred)

def residual_sigma(y_true, y_pred): #metric for the residuals sigma, given in unit of strips

    #retransformation from scaling, 
    #original scaling: score = (true-mean)/std 
    #back scaling --> true = score*std + mean
    y_true_scaled = y_true*merged_std + merged_mean
    y_pred_scaled = y_pred*merged_std + merged_mean
    return K.std(y_pred_scaled-y_true_scaled)

Input_1 = Input((64,))

x = Dense(600, activation='relu')(Input_1)
x = Dense(500, activation='relu')(x)
x = Dense(400, activation='relu')(x)
x = Dense(300, activation='relu')(x)

out1 = Dense(200,  activation='relu')(x)
out1 = Dense(100,  activation='relu')(out1)
out1 = Dense(50,  activation='relu')(out1)
out1 = Dense(40,  activation='relu')(out1)
out1 = Dense(30,  activation='relu')(out1)
out1 = Dense(20,  activation='relu')(out1)
out1 = Dense(10,  activation='relu')(out1)
out1 = Dense(1,  activation='linear', name='first_solution')(out1)

out2 = Dense(200,  activation='relu')(x)
out2 = Dense(100,  activation='relu')(out2)	
out2 = Dense(50,  activation='relu')(out2)
out2 = Dense(40,  activation='relu')(out2)
out2 = Dense(30,  activation='relu')(out2)
out2 = Dense(20,  activation='relu')(out2)
out2 = Dense(10,  activation='relu')(out2)
out2 = Dense(1,  activation='linear', name='second_solution')(out2)

model = Model(inputs=Input_1, outputs=[out1,out2])
model.compile(optimizer = args.optimizer, loss = 'mean_absolute_error', metrics=[residual_sigma])

history = model.fit(multiplexed_train, [first_solution_train, second_solution_train], validation_data = (multiplexed_test,[first_solution_test, second_solution_test]), epochs=args.epochs)

first_solution_pred, second_solution_pred = model.predict(multiplexed_test)


model.save(args.outname)
fig=plt.figure()
plt.plot(history.history['first_solution_residual_sigma'], 'o')
plt.plot(history.history['second_solution_residual_sigma'], 'o')

plt.title('Model Goodness')
plt.ylabel('Residual sigma')
plt.xlabel('Epoch')
plt.legend(['First Train', 'First Validation'])
plt.savefig("goodness_model_sol1.pdf")

plt.clf()
plt.plot(history.history['val_first_solution_residual_sigma'], 'o')
plt.plot(history.history['val_second_solution_residual_sigma'], 'o')

plt.title('Model Goodness')
plt.ylabel('Residual sigma')
plt.xlabel('Epoch')
plt.legend(['Second Train', 'Second Validation'])
plt.savefig("goodness_model_sol2.pdf")

#fig=plt.figure()
plt.clf()

test_solution_totransform = []
train_solution_totransform = []

i=0
while i<len(first_solution_pred):
    temp = []
    temp.append(first_solution_pred[i][0])
    temp.append(second_solution_pred[i][0])
    train_solution_totransform.append(temp)
    i += 1

i=0
while i<len(first_solution_test):
    temp2 = []
    temp2.append(first_solution_test[i])
    temp2.append(second_solution_test[i])
    test_solution_totransform.append(temp2)
    i += 1


train_solution_totransform = np.asarray(train_solution_totransform)
solution_pred = scaler2.inverse_transform(train_solution_totransform)
solution_test = scaler2.inverse_transform(test_solution_totransform)

i=0
diff = []
distance = []

print("Starting to calculate residuals")
while i < len(solution_pred):
    diff.append(solution_test[i][0] - solution_pred[i][0])
    diff.append(solution_test[i][1] - solution_pred[i][1])
    distance.append(abs(solution_test[i][0] - solution_test[i][1]))
    distance.append(abs(solution_test[i][0] - solution_test[i][1]))
    i+=1


#creating distance plot
step = 0.25
x_range = np.arange(0,20, step)
y_results = []
y_results_range = []

print("Starting to create Distance - Residual plot")
for x in x_range:
    i=0
    avg=[]
    counter=[]
    while i < len(diff):
        if(distance[i] > x - step and distance[i] < x + step):
            avg.append(abs(diff[i]))  
            if(abs(diff[i]) < 0.5):
                counter.append(abs(diff[i]))
        i+=1
    
    output = "Strip range of "+str(x)+" has been calculated"
    print(output)
    if(len(avg) == 0):
        y_results.append(0)
        y_results_range.append(0)
    
    else:
        y_results.append(np.mean(avg))
        y_results_range.append(len(counter)/len(avg))

diff = np.asarray(diff)
diff = diff[diff < 10]
diff = diff[diff > -10]
diff = diff.tolist()

(mu, sigma) = norm.fit(diff)
binwidth = 0.01
n, bins, patches = plt.hist(diff, normed=1, bins=np.arange(min(diff), max(diff) + binwidth, binwidth))
y = mlab.normpdf(bins, mu, sigma)
l = plt.plot(bins, y, 'r--', linewidth=2)
plt.title(r'$\mathrm{Histogram\ of\ Residuals:}\ \mu=%.3f,\ \sigma=%.3f$' %(mu, sigma))

print(mu)
print(sigma)

plt.xlabel('Difference in Strips')
plt.xlim([-10,10])
#plt.ylim([0,2])
plt.ylabel('Counts')
plt.savefig("residulas_toy.pdf")
plt.clf()
#plt.show()

plt.plot(x_range, y_results_range, 'o')
plt.ylabel('Residuals within [-0.5, 0.5] strips [%]')
plt.xlabel('Distance between clusters [strips]')
plt.savefig("distance_residual_range_toy.pdf")
plt.clf()

plt.plot(x_range, y_results, 'o')
plt.ylabel('Average Absolute Residual |PosPred - PosTrue| [strips]')
plt.xlabel('Distance between clusters [strips]')
plt.savefig("distance_residual_toy.pdf")
#plt.show()






