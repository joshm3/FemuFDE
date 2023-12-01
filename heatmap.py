import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
sns.set()

df = pd.read_csv("heatLog.csv")
df = df[df.columns[1:]]
df = df.transpose()

#plt.imshow(df,cmap='hot',interpolation='nearest')
sns.heatmap(df)
# plt.ylim(256,0)
plt.xlim(0,60)

plt.title("Joshua-Martin-Pagemap-Lat-CDF")
plt.xlabel("Seconds")
plt.ylabel("VA")
plt.savefig('heatmap.jpg')