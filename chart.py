import matplotlib.pyplot as plt 
import pandas as pd 
import numpy as np
import matplotlib.ticker as ticker
  
# #Code for plotting the latencies
dataCsv = pd.read_csv("dmesgLog.csv")
        
data = dataCsv['latency'].values
data = [x/1000 for x in data] #convert nanoseconds to microseconds
data = np.sort(data)#[:-1000] #uncomment to remove the longest x latencies
  
# # Option 1: plotting PDF and CDF without dots
count, bins_count = np.histogram(data, bins=10) 
pdf = count / sum(count) 
cdf = np.cumsum(pdf) 
plt.plot(bins_count[1:], pdf, color="red", label="PDF") 
plt.plot(bins_count[1:], cdf, label="CDF") 
plt.legend() 

# # # Option 2: Plot each point with a dot then connect with a line
# # y = np.arange(len(x)) / float(len(x)) 
# # plt.title('CDF using sorting the data') 
# # plt.plot(x, y, marker='o') 

plt.title("Joshua-Martin-Pagemap-Lat-CDF")
plt.xlabel("Latency in Microseconds")
plt.ylabel("Percentile")
plt.savefig('Joshua-Martin-Pagemap-Lat-CDF.jpg')


# Code for plotting the va and pa mappings
# data = pd.read_csv("log1.csv")
        
# X = data['va'].tolist()
# X = [int(x, 16) for x in X]

# plt.locator_params(axis='y', nbins=1)
# plt.locator_params(axis='x', nbins=4)

# def to_hex(x, pos):
#     return '%x' % int(x)
# fmt = ticker.FuncFormatter(to_hex)
# axes = plt.gca()
# axes.get_xaxis().set_major_formatter(fmt)

# #Set address space as x max/min
# #axes.set_xlim([int("555555559000", 16), int("555959575000", 16)]) #VA log1
# axes.set_xlim([0, 164908164 / (4*1024)])

# plt.plot(X, len(X) * [0], "x")

# # Show the plot 
# plt.savefig('log2pa.jpg')












