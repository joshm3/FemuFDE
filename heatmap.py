import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib.ticker as ticker
sns.set()

df = pd.read_csv("heatLog.csv")

df.drop(columns = df.columns[0], axis = 1, inplace= True)
df = df.transpose()
df["address"] = pd.Series([], dtype=int)
# df.to_csv("test.csv")

#plt.imshow(df,cmap='hot',interpolation='nearest')
sns.heatmap(df)
# plt.ylim(256,0)
# plt.xlim(0,60)

def to_hex(x, pos):
    print(x)
    print('%x' % int(x))
    return x
fmt = ticker.FuncFormatter(to_hex)
axes = plt.gca()
axes.get_yaxis().set_major_formatter(fmt)

# fmt = ticker.FormatStrFormatter('%f') 
# axes = plt.gca()
# axes.get_yaxis().set_major_formatter(fmt)

plt.title("Joshua-Martin-Pagemap-Lat-CDF")
plt.xlabel("Seconds")
plt.ylabel("VA")
plt.savefig('heatmap.jpg')