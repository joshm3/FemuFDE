import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib.ticker as ticker
from decimal import Decimal

df = pd.read_csv("heatLog.csv")

df.drop(columns = df.columns[0], axis = 1, inplace= True)
df = df.transpose()
# df.to_csv("test.csv")

#plt.imshow(df,cmap='hot',interpolation='nearest')
sns.heatmap(data=df, yticklabels=40, xticklabels=100, vmin = 0, vmax = 5)
# plt.ylim(256,0)
# plt.xlim(0,60)

def to_hex(x, pos):
    return '%.2E' % (int(x)*4096*1024)
fmt = ticker.FuncFormatter(to_hex)
axes = plt.gca()
axes.get_yaxis().set_major_formatter(fmt)
axes.invert_yaxis()

def to_hex(x, pos):
    return (x-.5)/10
fmt = ticker.FuncFormatter(to_hex)
axes = plt.gca()
axes.get_xaxis().set_major_formatter(fmt)

# fmt = ticker.FormatStrFormatter('%f') 
# axes = plt.gca()
# axes.get_yaxis().set_major_formatter(fmt)

plt.title("Joshua-Martin-Pagemap-Lat-CDF")
plt.xlabel("Seconds")
plt.ylabel("VA")
plt.savefig('heatmap.jpg')